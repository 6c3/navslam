from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='serial2esp',
            executable='serial_odom_node',
            name='serial_odom_node',
            output='screen'
        )
    ])
