// Component 3: spawn stationary_turtle and moving_turtle
// (also re-spawns turtle1 for the circle mover, since component 1 clears it)
#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"
#include "turtlesim/srv/spawn.hpp"

using namespace std::chrono_literals;

namespace software_training_assignment
{

class TurtleSpawner : public rclcpp::Node
{
public:
  explicit TurtleSpawner(const rclcpp::NodeOptions & options)
  : Node("turtle_spawner", options)
  {
    spawn_turtle1_ = this->declare_parameter<bool>("spawn_turtle1", true);

    client_ = this->create_client<turtlesim::srv::Spawn>("/spawn");
    // Don't call services in the constructor: a component isn't spinning yet
    timer_ = this->create_wall_timer(500ms, std::bind(&TurtleSpawner::spawn_turtles, this));
  }

private:
  void spawn_turtles()
  {
    timer_->cancel();  // run once

    if (!client_->wait_for_service(5s)) {
      RCLCPP_ERROR(this->get_logger(), "/spawn service not available - is turtlesim running?");
      return;
    }
    if (spawn_turtle1_) {
      spawn("turtle1", 5.544, 5.544);
    }
    spawn("stationary_turtle", 5.0, 5.0);
    spawn("moving_turtle", 25.0, 10.0);  // turtlesim clamps x to the window edge (~11.09)
  }

  void spawn(const std::string & name, double x, double y)
  {
    auto request = std::make_shared<turtlesim::srv::Spawn::Request>();
    request->name = name;
    request->x = x;
    request->y = y;
    request->theta = 0.0;

    client_->async_send_request(
      request,
      [this, name](rclcpp::Client<turtlesim::srv::Spawn>::SharedFuture future) {
        if (future.get()->name.empty()) {
          RCLCPP_WARN(this->get_logger(), "Could not spawn %s (already exists?)", name.c_str());
        } else {
          RCLCPP_INFO(this->get_logger(), "Spawned %s", name.c_str());
        }
      });
  }

  rclcpp::Client<turtlesim::srv::Spawn>::SharedPtr client_;
  rclcpp::TimerBase::SharedPtr timer_;
  bool spawn_turtle1_;
};

}  // namespace software_training_assignment

RCLCPP_COMPONENTS_REGISTER_NODE(software_training_assignment::TurtleSpawner)
