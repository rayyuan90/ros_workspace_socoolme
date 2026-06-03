from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode


def generate_launch_description():
    ld = LaunchDescription()

    talker_comp = ComposableNode(
        package='cpp_pubsub',
        plugin='cpp_pubsub::MinimalPublisher',
        name='talker_node'
    )

    listener_comp = ComposableNode(
        package='cpp_pubsub',
        plugin='cpp_pubsub::MinimalSubscriber',
        name='listener_node'
    )

    container = ComposableNodeContainer(
        name='ComponentManager',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[talker_comp, listener_comp],
        output='screen'
    )

    ld.add_action(container)
    return ld
