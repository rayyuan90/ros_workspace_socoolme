import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import SetEnvironmentVariable
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # 💡 最佳工程实践：将你的 XML 配置文件放在功能包的 config 目录下，并使其随功能包一起安装编译
    # 假设你的功能包叫 sync_async_node_example_cpp
    pkg_share_dir = get_package_share_directory('sync_async_node_example_cpp')
    xml_config_path = os.path.join(pkg_share_dir, 'config', 'SyncAsync.xml')
    
    # 如果你还未写 install 拷贝，也可以临时强行指定绝对路径进行真车调试：
    # xml_config_path = '/home/user/ros2_ws/src/sync_async_node_example_cpp/config/SyncAsync.xml'

    return LaunchDescription([
        # 1. 🛡️ 强制锁死底层中间件为 Fast DDS
        SetEnvironmentVariable(name='RMW_IMPLEMENTATION', value='rmw_fastrtps_cpp'),
        
        # 2. ⚡ 核心开关：强制允许底层 DDS 剥夺 ROS2 C++ 代码权限，全量使用 XML 里的同步/异步发布策略
        SetEnvironmentVariable(name='RMW_FASTRTPS_USE_QOS_FROM_XML', value='1'),
        
        # 3. 📂 喂入 XML 配置文件路径
        SetEnvironmentVariable(name='FASTRTPS_DEFAULT_PROFILES_FILE', value=xml_config_path),
        
        # 4. 拉起你编写的带有双发布者（同步、异步话题）的核心算法节点
        Node(
            package='sync_async_node_example_cpp',
            executable='SyncAsyncWriter',
            name='sync_async_publisher_node',
            output='screen'
        ),
        
        # 5. 拉起接收节点
        Node(
            package='sync_async_node_example_cpp',
            executable='SyncAsyncReader',
            name='sync_async_subscriber_node',
            output='screen'
        )
    ])