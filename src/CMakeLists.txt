# Use a different library name to avoid conflict with rosidl target
set(LIB_NAME ${PROJECT_NAME}_lib)

add_library(${LIB_NAME}
        laser_mapping.cc
        pointcloud_preprocess.cc
        options.cc
        utils.cc
        )

ament_target_dependencies(${LIB_NAME}
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
        livox_ros_driver
)

# link self-generated Pose6D message typesupport
rosidl_get_typesupport_target(self_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(${LIB_NAME} ${self_typesupport_target})

target_link_libraries(${LIB_NAME}
        ${PCL_LIBRARIES}
        tbb
        glog
        yaml-cpp
        )
