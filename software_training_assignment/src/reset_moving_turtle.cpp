// Component 4: service that resets moving_turtle to its starting position
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"
#include "software_training_assignment/srv/reset.hpp"
#include "turtlesim/srv/teleport_absolute.hpp"

namespace software_training_assignment
{

class ResetMovingTurtle : public rclcpp::Node
{
public:
  using Reset = software_training_assignment::srv::Reset;
  using Teleport = turtlesim::srv::TeleportAbsolute;

  explicit ResetMovingTurtle(const rclcpp::NodeOptions & options)
  : Node("reset_moving_turtle", options)
  {
    start_x_ = this->declare_parameter<double>("start_x", 25.0);
    start_y_ = this->declare_parameter<double>("start_y", 10.0);

    teleport_client_ = this->create_client<Teleport>("/moving_turtle/teleport_absolute");

    service_ = this->create_service<Reset>(
      "/reset_moving_turtle",
      std::bind(
        &ResetMovingTurtle::handle_reset, this,
        std::placeholders::_1, std::placeholders::_2));

    RCLCPP_INFO(this->get_logger(), "Ready: call /reset_moving_turtle to reset moving_turtle");
  }

private:
  void handle_reset(
    const std::shared_ptr<Reset::Request> request,
    std::shared_ptr<Reset::Response> response)
  {
    (void)request;  // request is empty

    // Waiting for the teleport reply here would block this node's only thread,
    // so we report success once the request has been sent to a ready service.
    if (!teleport_client_->service_is_ready()) {
      RCLCPP_WARN(this->get_logger(), "moving_turtle does not exist - reset failed");
      response->success = false;
      return;
    }

    auto teleport = std::make_shared<Teleport::Request>();
    teleport->x = start_x_;
    teleport->y = start_y_;
    teleport->theta = 0.0;
    teleport_client_->async_send_request(teleport);

    RCLCPP_INFO(this->get_logger(), "Reset moving_turtle to (%.1f, %.1f)", start_x_, start_y_);
    response->success = true;
  }

  rclcpp::Service<Reset>::SharedPtr service_;
  rclcpp::Client<Teleport>::SharedPtr teleport_client_;
  double start_x_;
  double start_y_;
};

}  // namespace software_training_assignment

RCLCPP_COMPONENTS_REGISTER_NODE(software_training_assignment::ResetMovingTurtle)
