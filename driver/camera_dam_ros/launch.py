# 必须以实时策略启动
def generate_launch_description():
    return LaunchDescription([
        Node(package='my_robot', executable='camera_node', 
             prefix='chrt -f 99'), # 将优先级设为最高
        Node(package='my_robot', executable='planner_node')
    ])