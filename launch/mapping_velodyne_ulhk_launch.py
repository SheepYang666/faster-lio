from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():
    pkg_dir = get_package_share_directory('faster_lio')

    rviz_arg = DeclareLaunchArgument('rviz', default_value='true')

    mapping_node = Node(
        package='faster_lio',
        executable='run_mapping_online',
        name='laserMapping',
        output='screen',
        parameters=[
            os.path.join(pkg_dir, 'config', 'velodyne_ulhk.yaml'),
            {
                'feature_extract_enable': False,
                'point_filter_num': 4,
                'max_iteration': 3,
                'filter_size_surf': 0.5,
                'filter_size_map': 0.5,
                'cube_side_length': 1000.0,
                'runtime_pos_log_enable': False,
            }
        ],
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', os.path.join(pkg_dir, 'config', 'rviz', 'faster_lio.rviz')],
        condition=IfCondition(LaunchConfiguration('rviz')),
        prefix='nice',
    )

    return LaunchDescription([rviz_arg, mapping_node, rviz_node])
