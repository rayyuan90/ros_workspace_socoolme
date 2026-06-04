from launch import LaunchDescription
from launch.actions import GroupAction, IncludeLaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare
from launch.actions import GroupAction
from launch_ros.actions import PushROSNamespace


def generate_launch_description():
    launch_dir = PathJoinSubstitution([FindPackageShare('launch_tutorial'), 'launch'])
    return LaunchDescription(
        [
            IncludeLaunchDescription(
                PathJoinSubstitution([launch_dir, "turtlesim_world_1_launch.py"])
            ),
            IncludeLaunchDescription(
                PathJoinSubstitution([launch_dir, "turtlesim_world_2_launch.py"])
            ),
            IncludeLaunchDescription(
                PathJoinSubstitution([launch_dir, "broadcaster_listener_launch.py"]),
                launch_arguments={"target_frame": "carrot1"}.items(),
            ),
            IncludeLaunchDescription(
                PathJoinSubstitution([launch_dir, "mimic_launch.py"])
            ),
            IncludeLaunchDescription(
                PathJoinSubstitution([launch_dir, "fixed_broadcaster_launch.py"])
            ),
            IncludeLaunchDescription(
                PathJoinSubstitution([launch_dir, "turtlesim_rviz_launch.py"])
            ),
            GroupAction(
                actions=[
                    PushROSNamespace("turtlesim2"),
                    IncludeLaunchDescription(
                        
                        PathJoinSubstitution([launch_dir, 'broadcaster_listener_launch.py']),
                            launch_arguments={'target_frame': 'carrot1'}.items()
                        ),
                ]
            ),
        ]
    )
