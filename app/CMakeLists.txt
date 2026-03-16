add_executable(run_mapping_online run_mapping_online.cc)
target_link_libraries(run_mapping_online ${PROJECT_NAME}_lib gflags)
ament_target_dependencies(run_mapping_online rclcpp)

add_executable(run_mapping_offline run_mapping_offline.cc)
target_link_libraries(run_mapping_offline ${PROJECT_NAME}_lib gflags)
ament_target_dependencies(run_mapping_offline rclcpp rosbag2_cpp livox_ros_driver2)

install(TARGETS
    run_mapping_online
    run_mapping_offline
    DESTINATION lib/${PROJECT_NAME}
)
