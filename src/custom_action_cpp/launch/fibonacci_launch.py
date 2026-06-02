# my_robot_launch.py
import os
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    """
    这是 ROS 2 Launch 系统固定的入口函数，必须返回一个 LaunchDescription 对象。
    """
    
    # 定义你要启动的第一个节点 (例如你的 C++ 节点)
    my_cpp_node = Node(
        package='custom_action_cpp',         # 1. 你的功能包名字 (对应 CMakeLists.txt 里的 project)
        executable='fibonacci_action_server',          # 2. 可执行文件名字，改为实际注册的 executable
        name='fibonacci_server_node',        # 3. 运行时的节点别名 (会覆盖 C++ 代码里 Node("xxx") 的名字)
        output='screen',                     # 4. 日志输出方式：screen 表示直接打印到当前终端屏幕
        emulate_tty=True,                    # 5. 确保 C++ 的 RCLCPP_INFO 颜色和排版能正确在终端显示
        parameters=[                         # 6. [可选] 载入参数
            {'my_parameter': 'hello_world'},
            # os.path.join(get_package_share_directory('pkg'), 'config', 'params.yaml') # 或者载入 yaml 文件
        ],
        remappings=[                         # 7. [可选] 话题重映射 (改名)
            ('/original_topic', '/new_topic')
        ]
    )

    # 定义你要启动的第二个节点 (例如官方的某个节点，可以写在一起)
    # another_node = Node( ... )

    # 创建 LaunchDescription 并把所有要启动的节点打包丢进去
    return LaunchDescription([
        my_cpp_node,
        # another_node
    ])