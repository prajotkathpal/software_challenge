// Component 6: action server that drives moving_turtle to a waypoint in a straight line
#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"
#include "software_training_assignment/action/waypoint.hpp"
#include "turtlesim/msg/pose.hpp"

namespace software_training_assignment
{

class WaypointActionServer : public rclcpp::Node
{
public:
  using Waypoint = software_training_assignment::action::Waypoint;
  using GoalHandle = rclcpp_action::ServerGoalHandle<Waypoint>;
  using Pose = turtlesim::msg::Pose;
  using Twist = geometry_msgs::msg::Twist;

  explicit WaypointActionServer(const rclcpp::NodeOptions & options)
  : Node("waypoint_action_server", options)
  {
    tolerance_ = this->declare_parameter<double>("tolerance", 0.1);        // how close counts as "arrived"
    max_linear_ = this->declare_parameter<double>("max_linear_speed", 2.0);
    max_angular_ = this->declare_parameter<double>("max_angular_speed", 2.0);

    cmd_pub_ = this->create_publisher<Twist>("/moving_turtle/cmd_vel", 10);
    pose_sub_ = this->create_subscription<Pose>(
      "/moving_turtle/pose", 10,
      [this](const Pose::SharedPtr msg) {
        std::lock_guard<std::mutex> lock(pose_mutex_);
        pose_ = *msg;
        have_pose_ = true;
      });

    using namespace std::placeholders;
    action_server_ = rclcpp_action::create_server<Waypoint>(
      this, "/moving_turtle_waypoint",
      std::bind(&WaypointActionServer::handle_goal, this, _1, _2),
      std::bind(&WaypointActionServer::handle_cancel, this, _1),
      std::bind(&WaypointActionServer::handle_accepted, this, _1));

    RCLCPP_INFO(this->get_logger(), "Ready: send goals to /moving_turtle_waypoint");
  }

private:
  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID & uuid,
    std::shared_ptr<const Waypoint::Goal> goal)
  {
    (void)uuid;
    // Turtlesim's window is about 0..11.08 in both axes; outside it the turtle can't arrive
    if (goal->x < 0.0 || goal->x > 11.08 || goal->y < 0.0 || goal->y > 11.08) {
      RCLCPP_WARN(this->get_logger(), "Rejecting goal (%.2f, %.2f): outside the window", goal->x, goal->y);
      return rclcpp_action::GoalResponse::REJECT;
    }
    RCLCPP_INFO(this->get_logger(), "Accepted goal (%.2f, %.2f)", goal->x, goal->y);
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandle> goal_handle)
  {
    (void)goal_handle;
    RCLCPP_INFO(this->get_logger(), "Cancel requested");
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void handle_accepted(const std::shared_ptr<GoalHandle> goal_handle)
  {
    // Run on its own thread so the executor keeps delivering pose messages
    std::thread{std::bind(&WaypointActionServer::execute, this, std::placeholders::_1), goal_handle}
    .detach();
  }

  static double normalize_angle(double a)
  {
    while (a > M_PI) {a -= 2.0 * M_PI;}
    while (a < -M_PI) {a += 2.0 * M_PI;}
    return a;
  }

  void stop()
  {
    cmd_pub_->publish(Twist());
  }

  void execute(const std::shared_ptr<GoalHandle> goal_handle)
  {
    const auto start = std::chrono::steady_clock::now();
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<Waypoint::Feedback>();
    auto result = std::make_shared<Waypoint::Result>();
    rclcpp::Rate rate(20);  // 20 Hz control loop

    auto elapsed_ns = [&start]() {
        return static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - start).count());
      };

    while (rclcpp::ok()) {
      if (goal_handle->is_canceling()) {
        stop();
        result->duration = elapsed_ns();
        goal_handle->canceled(result);
        RCLCPP_INFO(this->get_logger(), "Goal canceled");
        return;
      }

      Pose pose;
      {
        std::lock_guard<std::mutex> lock(pose_mutex_);
        if (!have_pose_) {
          rate.sleep();
          continue;  // wait for the first pose
        }
        pose = pose_;
      }

      const double dx = goal->x - pose.x;
      const double dy = goal->y - pose.y;
      const double distance = std::hypot(dx, dy);

      feedback->distance = distance;
      goal_handle->publish_feedback(feedback);

      if (distance < tolerance_) {
        break;  // arrived
      }

      // Face the waypoint first, then drive straight at it
      const double heading_error = normalize_angle(std::atan2(dy, dx) - pose.theta);
      Twist cmd;
      cmd.angular.z = std::clamp(4.0 * heading_error, -max_angular_, max_angular_);
      if (std::fabs(heading_error) < 0.05) {
        cmd.linear.x = std::min(1.5 * distance, max_linear_);
      }
      cmd_pub_->publish(cmd);

      rate.sleep();
    }

    stop();
    if (rclcpp::ok()) {
      result->duration = elapsed_ns();
      goal_handle->succeed(result);
      RCLCPP_INFO(
        this->get_logger(), "Reached waypoint in %.2f s",
        static_cast<double>(result->duration) / 1e9);
    }
  }

  rclcpp_action::Server<Waypoint>::SharedPtr action_server_;
  rclcpp::Publisher<Twist>::SharedPtr cmd_pub_;
  rclcpp::Subscription<Pose>::SharedPtr pose_sub_;
  std::mutex pose_mutex_;
  Pose pose_;
  bool have_pose_ = false;
  double tolerance_;
  double max_linear_;
  double max_angular_;
};

}  // namespace software_training_assignment

RCLCPP_COMPONENTS_REGISTER_NODE(software_training_assignment::WaypointActionServer)
