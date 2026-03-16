//
// Created by xiang on 2021/10/9.
//

#include <gflags/gflags.h>
#include <unistd.h>
#include <csignal>

#include <rosbag2_cpp/reader.hpp>
#include <rclcpp/serialization.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <livox_ros_driver2/msg/custom_msg.hpp>

#include "laser_mapping.h"
#include "utils.h"

/// run faster-LIO in offline mode

DEFINE_string(config_file, "./config/avia.yaml", "path to config file");
DEFINE_string(bag_file, "", "path to the ros2 bag");
DEFINE_string(time_log_file, "./Log/time.log", "path to time log file");
DEFINE_string(traj_log_file, "./Log/traj.txt", "path to traj log file");

void SigHandle(int sig) {
    faster_lio::options::FLAG_EXIT = true;
    LOG(WARNING) << "catch sig " << sig;
}

int main(int argc, char **argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    FLAGS_stderrthreshold = google::INFO;
    FLAGS_colorlogtostderr = true;
    google::InitGoogleLogging(argv[0]);

    const std::string bag_file = FLAGS_bag_file;
    const std::string config_file = FLAGS_config_file;

    auto laser_mapping = std::make_shared<faster_lio::LaserMapping>();
    if (!laser_mapping->InitWithoutROS(FLAGS_config_file)) {
        LOG(ERROR) << "laser mapping init failed.";
        return -1;
    }

    /// handle ctrl-c
    signal(SIGINT, SigHandle);

    // just read the bag and send the data
    LOG(INFO) << "Opening rosbag, be patient";
    rosbag2_cpp::Reader reader;
    reader.open(FLAGS_bag_file);

    // serializers for message types
    rclcpp::Serialization<livox_ros_driver2::msg::CustomMsg> livox_serializer;
    rclcpp::Serialization<sensor_msgs::msg::PointCloud2> pcl_serializer;
    rclcpp::Serialization<sensor_msgs::msg::Imu> imu_serializer;

    LOG(INFO) << "Go!";
    while (reader.has_next()) {
        if (faster_lio::options::FLAG_EXIT) {
            break;
        }

        auto bag_msg = reader.read_next();
        auto topic_name = bag_msg->topic_name;
        rclcpp::SerializedMessage serialized_msg(*bag_msg->serialized_data);

        // Try as livox CustomMsg
        if (topic_name.find("livox") != std::string::npos || topic_name.find("lidar") != std::string::npos) {
            // Try to deserialize as CustomMsg first for AVIA lidar type
            if (laser_mapping->GetPreprocess() && laser_mapping->GetPreprocess()->GetLidarType() == faster_lio::LidarType::AVIA) {
                try {
                    auto livox_msg = std::make_shared<livox_ros_driver2::msg::CustomMsg>();
                    livox_serializer.deserialize_message(&serialized_msg, livox_msg.get());
                    faster_lio::Timer::Evaluate(
                        [&laser_mapping, &livox_msg]() {
                            laser_mapping->LivoxPCLCallBack(livox_msg);
                            laser_mapping->Run();
                        },
                        "Laser Mapping Single Run");
                    continue;
                } catch (...) {
                    // fall through to PointCloud2
                }
            }

            // Try as PointCloud2
            try {
                auto point_cloud_msg = std::make_shared<sensor_msgs::msg::PointCloud2>();
                pcl_serializer.deserialize_message(&serialized_msg, point_cloud_msg.get());
                faster_lio::Timer::Evaluate(
                    [&laser_mapping, &point_cloud_msg]() {
                        laser_mapping->StandardPCLCallBack(point_cloud_msg);
                        laser_mapping->Run();
                    },
                    "Laser Mapping Single Run");
                continue;
            } catch (...) {
                // not a point cloud message
            }
        }

        // Try as IMU
        if (topic_name.find("imu") != std::string::npos) {
            try {
                auto imu_msg = std::make_shared<sensor_msgs::msg::Imu>();
                imu_serializer.deserialize_message(&serialized_msg, imu_msg.get());
                laser_mapping->IMUCallBack(imu_msg);
                continue;
            } catch (...) {
                // not an imu message
            }
        }
    }

    LOG(INFO) << "finishing mapping";
    laser_mapping->Finish();

    /// print the fps
    double fps = 1.0 / (faster_lio::Timer::GetMeanTime("Laser Mapping Single Run") / 1000.);
    LOG(INFO) << "Faster LIO average FPS: " << fps;

    LOG(INFO) << "save trajectory to: " << FLAGS_traj_log_file;
    laser_mapping->Savetrajectory(FLAGS_traj_log_file);

    faster_lio::Timer::PrintAll();
    faster_lio::Timer::DumpIntoFile(FLAGS_time_log_file);

    return 0;
}
