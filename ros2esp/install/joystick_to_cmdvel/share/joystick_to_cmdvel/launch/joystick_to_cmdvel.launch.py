from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='joystick_node',
            executable='joystick_node',
            name='joystick_node',
            output='screen',
            parameters=[{
                'joystick_type': 'ps5',
                'joystick_device': '/dev/input/js0',
                'joystick_bits': 16
            }],
            remappings=[
                ('/joystick_msg', '/robot_control/joystick_msgs'),
                # ('cmd_vel', '/cmd_vel')
            ]
        ),
        Node(
            package='joystick_to_cmdvel',
            executable='joystick_to_cmdvel_node',
            name='joystick_to_cmdvel_node',
            output='screen',
            remappings=[
                ('joy', '/robot_control/joystick_msgs'),
                ('cmd_vel', '/cmd_vel')
            ]
        )
    ])
