# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Faster-LIO is a lightweight tightly-coupled LiDAR-Inertial Odometry system using parallel sparse incremental voxels (iVox). Recently migrated from ROS1 to **ROS2 Humble**.

## Build Commands

```bash
# ROS2 workspace build (both packages must build together)
colcon build --packages-select livox_ros_driver faster_lio

# With PHC iVox variant (default is linear iVox)
colcon build --packages-select faster_lio --cmake-args -DWITH_IVOX_NODE_TYPE_PHC=ON
```

## Run Commands

```bash
# Online mode (with live sensor or bag playback)
ros2 launch faster_lio mapping_avia_launch.py           # Livox AVIA
ros2 launch faster_lio mapping_velodyne_launch.py       # Velodyne
ros2 launch faster_lio mapping_mid360_launch.py         # Livox Mid-360
# ... other variants: hesai, ouster64, robosense, horizon, livox

# Disable rviz
ros2 launch faster_lio mapping_avia_launch.py rviz:=false

# Offline mode (rosbag2 .db3/.mcap)
ros2 run faster_lio run_mapping_offline --bag_file=/path/to/bag --config_file=config/avia.yaml
```

## Architecture

**Signal flow:** LiDAR + IMU → Preprocess → Sync → Undistort → Match (iVox) → ESEKF Update → Map Update → Publish

Key classes and their roles:
- **`LaserMapping`** (`src/laser_mapping.cc`) — Main ROS2 node. Inherits `rclcpp::Node`. Orchestrates the full pipeline: parameter loading, subscription/publishing, sync, ESEKF iteration, and map update. This is ~1000 lines and the primary file for ROS integration changes.
- **`ImuProcess`** (`include/imu_processing.hpp`) — IMU initialization, forward propagation, and point cloud undistortion via backward propagation. Entirely in the header file.
- **`PointCloudPreprocess`** (`src/pointcloud_preprocess.cc`) — Converts 6 LiDAR types (Livox AVIA/Mid360, Velodyne, Ouster, Hesai, RoboSense) into a unified PCL format. LiDAR-specific handlers use `pcl::fromROSMsg`.
- **`IVox`** (`include/ivox3d/ivox3d.h`) — Hash-based incremental voxel map for fast nearest-neighbor queries. Template with two node types: DEFAULT (linear) and PHC (pseudo-Hilbert curve).
- **ESEKF** (`include/use-ikfom.hpp` + `include/IKFoM_toolkit/`) — Error-State Extended Kalman Filter on manifolds. State includes: pos, rot(SO3), velocity, IMU biases, gravity(S2), LiDAR-IMU extrinsics.

**Data flow detail:** `MeasureGroup` (defined in `common_lib.h`) bundles synchronized LiDAR frames + IMU measurements. The `SyncPackages()` method in LaserMapping handles temporal alignment. Point timestamps are stored in the `curvature` field of `pcl::PointXYZINormal` (in milliseconds).

## LiDAR Type System

Enum `LidarType` in `pointcloud_preprocess.h`: AVIA(1), VELO32(2), OUST64(3), HESAIxt32(4), ROBOSENSE(5), LIVOX(6). Type 1 (AVIA) subscribes to `livox_ros_driver::msg::CustomMsg`; all others use `sensor_msgs::msg::PointCloud2`.

## Configuration

YAML configs in `config/` are wrapped with `laserMapping: ros__parameters:` for ROS2 parameter loading. Key tuning parameters:
- `ivox_grid_resolution` (default 0.2) — voxel size, sensitive to environment scale
- `ivox_nearby_type` (6/18/26) — neighbor connectivity for nearest search
- `mapping.extrinsic_T/R` — LiDAR-IMU extrinsic calibration
- `preprocess.lidar_type` — must match your sensor (see enum above)

## Key Dependencies

PCL 1.8+, Eigen3, Intel TBB (parallel algorithms via `std::execution::par_unseq`), glog, yaml-cpp, tf2_ros, rosbag2_cpp. Custom messages: `livox_ros_driver` (in `thirdparty/`, built as separate ament package) and `faster_lio::msg::Pose6D`.

## Conventions

- Time utility functions `common::toROSTime(double)` and `common::toSec(builtin_interfaces::msg::Time)` in `common_lib.h` for all time conversions
- ROS2 parameters use `.` separator (e.g., `common.lid_topic`, `mapping.gyr_cov`)
- All parallel point processing uses `std::execution::par_unseq` with index vectors
- The `curvature` field of PointXYZINormal stores per-point timestamp offset in milliseconds
- Docker setup (`docker/`) still references ROS1 Noetic and needs updating
