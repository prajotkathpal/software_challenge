cmake_minimum_required(VERSION 3.8)
project(software_training_assignment)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(turtlesim REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# ---------- Custom interfaces ----------
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Distance.msg"
  "srv/Reset.srv"
  "action/Waypoint.action"
)

# ---------- Components ----------
# add_component(<target> <source> <plugin class> <executable> [uses custom interfaces])
# (a macro, not a function: rclcpp_components_register_node must run in this file's scope
#  or the components are never added to the package's component index)
macro(add_component target source plugin executable)
  add_library(${target} SHARED ${source})
  ament_target_dependencies(${target}
    rclcpp rclcpp_action rclcpp_components turtlesim geometry_msgs)
  set(_uses_interfaces "${ARGN}")
  if(_uses_interfaces)
    rosidl_target_interfaces(${target} ${PROJECT_NAME} "rosidl_typesupport_cpp")
  endif()
  rclcpp_components_register_node(${target}
    PLUGIN "${PROJECT_NAME}::${plugin}"
    EXECUTABLE ${executable})
  install(TARGETS ${target}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
endmacro()

add_component(turtle_clear           src/turtle_clear.cpp           TurtleClear          turtle_clear_node)
add_component(turtle_circle          src/turtle_circle.cpp          TurtleCircle         turtle_circle_node)
add_component(turtle_spawner         src/turtle_spawner.cpp         TurtleSpawner        turtle_spawner_node)
add_component(reset_moving_turtle    src/reset_moving_turtle.cpp    ResetMovingTurtle    reset_moving_turtle_node    INTERFACES)
add_component(distance_publisher     src/distance_publisher.cpp     DistancePublisher    distance_publisher_node     INTERFACES)
add_component(waypoint_action_server src/waypoint_action_server.cpp WaypointActionServer waypoint_action_server_node INTERFACES)

install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
