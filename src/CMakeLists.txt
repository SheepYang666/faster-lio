add_library(${PROJECT_NAME}
        laser_mapping.cc
        pointcloud_preprocess.cc
        options.cc
        utils.cc
        )

ament_target_dependencies(${PROJECT_NAME}
        rclcpp
        sensor_msgs
        nav_msgs
        geometry_msgs
        std_msgs
        tf2_ros
        tf2_geometry_msgs
        tf2_eigen
        pcl_conversions
        rosbag2_cpp
)

# link livox_ros_driver generated message typesupport
rosidl_get_typesupport_target(livox_typesupport_target livox_ros_driver "rosidl_typesupport_cpp")
target_link_libraries(${PROJECT_NAME} ${livox_typesupport_target})

# link self-generated Pose6D message typesupport
rosidl_get_typesupport_target(self_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(${PROJECT_NAME} ${self_typesupport_target})

target_link_libraries(${PROJECT_NAME}
        ${PCL_LIBRARIES}
        tbb
        glog
        yaml-cpp
        )
