// Component 2: drive turtle1 in a circle
#include <chrono>
#include <memory>

#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"

using namespace std::chrono_literals;

namespace software_training_assignment
{

class TurtleCircle : public rclcpp::Node
{
public:
  explicit TurtleCircle(const rclcpp::NodeOptions & options)
  : Node("turtle_circle", options)
  {
    // Speeds can be changed at launch time as parameters
    linear_speed_ = this->declare_parameter<double>("linear_speed", 2.0);    // m/s forward
    angular_speed_ = this->declare_parameter<double>("angular_speed", 1.0);  // rad/s turning

    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);

    // Publish a velocity command 10 times a second
    timer_ = this->create_wall_timer(100ms, std::bind(&TurtleCircle::publish_velocity, this));

    RCLCPP_INFO(
      this->get_logger(), "Moving turtle1 in a circle (linear=%.2f, angular=%.2f)",
      linear_speed_, angular_speed_);
  }

private:
  void publish_velocity()
  {
    // Constant forward speed + constant turn rate = circle (radius = linear / angular)
    geometry_msgs::msg::Twist msg;
    msg.linear.x = linear_speed_;
    msg.angular.z = angular_speed_;
    publisher_->publish(msg);
  }

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  double linear_speed_;
  double angular_speed_;
};

}  // namespace software_training_assignment

RCLCPP_COMPONENTS_REGISTER_NODE(software_training_assignment::TurtleCircle)
