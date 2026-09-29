// Component 5: publish the distance between stationary_turtle and moving_turtle
#include <chrono>
#include <cmath>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"
#include "software_training_assignment/msg/distance.hpp"
#include "turtlesim/msg/pose.hpp"

using namespace std::chrono_literals;

namespace software_training_assignment
{

class DistancePublisher : public rclcpp::Node
{
public:
  using Pose = turtlesim::msg::Pose;
  using Distance = software_training_assignment::msg::Distance;

  explicit DistancePublisher(const rclcpp::NodeOptions & options)
  : Node("distance_publisher", options)
  {
    stationary_sub_ = this->create_subscription<Pose>(
      "/stationary_turtle/pose", 10,
      [this](const Pose::SharedPtr msg) {stationary_ = *msg; have_stationary_ = true;});

    moving_sub_ = this->create_subscription<Pose>(
      "/moving_turtle/pose", 10,
      [this](const Pose::SharedPtr msg) {moving_ = *msg; have_moving_ = true;});

    publisher_ = this->create_publisher<Distance>("/turtle_distance", 10);
    timer_ = this->create_wall_timer(100ms, std::bind(&DistancePublisher::publish_distance, this));
  }

private:
  void publish_distance()
  {
    if (!have_stationary_ || !have_moving_) {
      return;  // wait until both turtles exist
    }
    Distance msg;
    msg.x_distance = moving_.x - stationary_.x;
    msg.y_distance = moving_.y - stationary_.y;
    msg.distance = std::hypot(msg.x_distance, msg.y_distance);
    publisher_->publish(msg);
  }

  rclcpp::Subscription<Pose>::SharedPtr stationary_sub_;
  rclcpp::Subscription<Pose>::SharedPtr moving_sub_;
  rclcpp::Publisher<Distance>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  Pose stationary_;
  Pose moving_;
  bool have_stationary_ = false;
  bool have_moving_ = false;
};

}  // namespace software_training_assignment

RCLCPP_COMPONENTS_REGISTER_NODE(software_training_assignment::DistancePublisher)
