"""Start turtlesim, clear it, then load all software training components."""
from launch import LaunchDescription
from launch.actions import TimerAction
from launch_ros.actions import ComposableNodeContainer, Node
from launch_ros.descriptions import ComposableNode

PKG = 'software_training_assignment'


def component(plugin, name, params=None):
    return ComposableNode(
        package=PKG,
        plugin=f'{PKG}::{plugin}',
        name=name,
        parameters=[params or {}],
    )


def generate_launch_description():
    # 1. turtlesim first - everything else depends on it
    turtlesim = Node(
        package='turtlesim',
        executable='turtlesim_node',
        name='turtlesim',
        parameters=[{'background_r': 30, 'background_g': 30, 'background_b': 60}],
        output='screen',
    )

    # 2. Clear any existing turtles
    clear_container = ComposableNodeContainer(
        name='clear_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[component('TurtleClear', 'turtle_clear')],
        output='screen',
    )

    # 3. Everything else, in one multi-threaded container
    training_container = ComposableNodeContainer(
        name='training_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container_mt',
        composable_node_descriptions=[
            component('TurtleSpawner', 'turtle_spawner'),
            component('TurtleCircle', 'turtle_circle',
                      {'linear_speed': 2.0, 'angular_speed': 1.0}),
            component('DistancePublisher', 'distance_publisher'),
            component('ResetMovingTurtle', 'reset_moving_turtle',
                      {'start_x': 25.0, 'start_y': 10.0}),
            component('WaypointActionServer', 'waypoint_action_server'),
        ],
        output='screen',
    )

    return LaunchDescription([
        turtlesim,
        TimerAction(period=2.0, actions=[clear_container]),
        TimerAction(period=4.0, actions=[training_container]),
    ])
