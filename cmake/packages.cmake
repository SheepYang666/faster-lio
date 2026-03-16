list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# glog
find_package(Glog REQUIRED)
include_directories(${Glog_INCLUDE_DIRS})

# for ubuntu 18.04, update gcc/g++ to 9, and download tbb2018 from
# https://github.com/oneapi-src/oneTBB/releases/download/2018/tbb2018_20170726oss_lin.tgz,
# extract it into CUSTOM_TBB_DIR
# specifiy tbb2018, e.g. CUSTOM_TBB_DIR=/home/idriver/Documents/tbb2018_20170726oss
if (CUSTOM_TBB_DIR)
    set(TBB2018_INCLUDE_DIR "${CUSTOM_TBB_DIR}/include")
    set(TBB2018_LIBRARY_DIR "${CUSTOM_TBB_DIR}/lib/intel64/gcc4.7")
    include_directories(${TBB2018_INCLUDE_DIR})
    link_directories(${TBB2018_LIBRARY_DIR})
endif ()

# ROS2 dependencies
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(rosbag2_cpp REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(livox_ros_driver REQUIRED)

# message generation (creates a utility target named ${PROJECT_NAME})
rosidl_generate_interfaces(${PROJECT_NAME}
    "msg/Pose6D.msg"
)

find_package(Eigen3 REQUIRED)
find_package(PCL 1.8 REQUIRED)
find_package(yaml-cpp REQUIRED)

include_directories(
        ${EIGEN3_INCLUDE_DIR}
        ${PCL_INCLUDE_DIRS}
        ${yaml-cpp_INCLUDE_DIRS}
        include
)
