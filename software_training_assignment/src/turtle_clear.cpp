// Component 1: clear (kill) every existing turtle
#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"
#include "turtlesim/srv/kill.hpp"

using namespace std::chrono_literals;

namespace software_training_assignment
{

class TurtleClear : public rclcpp::Node
{
public:
  explicit TurtleClear(const rclcpp::NodeOptions & options)
  : Node("turtle_clear", options)
  {
    client_ = this->create_client<turtlesim::srv::Kill>("/kill");
    // Give ROS discovery a moment so we can see every turtle's /pose topic
    timer_ = this->create_wall_timer(1s, std::bind(&TurtleClear::clear_turtles, this));
  }

private:
  void clear_turtles()
  {
    timer_->cancel();  // run once

    if (!client_->wait_for_service(5s)) {
      RCLCPP_ERROR(this->get_logger(), "/kill service not available - is turtlesim running?");
      return;
    }

    // Every turtle publishes /<name>/pose of type turtlesim/msg/Pose
    int count = 0;
    for (const auto & [topic, types] : this->get_topic_names_and_types()) {
      const std::string suffix = "/pose";
      bool is_pose_type = false;
      for (const auto & t : types) {
        if (t == "turtlesim/msg/Pose") {is_pose_type = true;}
      }
      if (!is_pose_type || topic.size() <= suffix.size() + 1 ||
        topic.compare(topic.size() - suffix.size(), suffix.size(), suffix) != 0)
      {
        continue;
      }
      const std::string name = topic.substr(1, topic.size() - suffix.size() - 1);  // "/turtle1/pose" -> "turtle1"
      kill(name);
      ++count;
    }

    if (count == 0) {
      RCLCPP_INFO(this->get_logger(), "No turtles to clear");
    }
  }

  void kill(const std::string & name)
  {
    auto request = std::make_shared<turtlesim::srv::Kill::Request>();
    request->name = name;
    client_->async_send_request(
      request,
      [this, name](rclcpp::Client<turtlesim::srv::Kill>::SharedFuture) {
        RCLCPP_INFO(this->get_logger(), "Cleared %s", name.c_str());
      });
  }

  rclcpp::Client<turtlesim::srv::Kill>::SharedPtr client_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace software_training_assignment

RCLCPP_COMPONENTS_REGISTER_NODE(software_training_assignment::TurtleClear)
