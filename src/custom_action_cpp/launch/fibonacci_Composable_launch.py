from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode


def generate_launch_description():
    ld = LaunchDescription()

    # Define the composable node (the component that's registered in the
    # `action_server` library via rclcpp_components_register_node)
    fibonacci_component = ComposableNode(
        package='custom_action_cpp',
        plugin='custom_action_cpp::FibonacciActionServer',
        name='fibonacci_action_server'
    )

    # Use the generic component container to host the component
    container = ComposableNodeContainer(
        name='fibonacci_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[fibonacci_component],
        output='screen'
    )

    ld.add_action(container)
    return ld