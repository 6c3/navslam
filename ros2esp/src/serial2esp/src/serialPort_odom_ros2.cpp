#include <rclcpp/rclcpp.hpp>
#include <serial/serial.h>
#include <std_msgs/msg/string.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <cmath>
#include <cstring>

#define PI 3.141593
#define RAD (PI / 180.0)

class SerialOdomNode : public rclcpp::Node
{
public:
    SerialOdomNode()
        : Node("read_imu_serial_node")
    {
        // Declare and get parameters
        this->declare_parameter<std::string>("device", "/dev/ttyACM0");
        this->get_parameter("device", device_);
        RCLCPP_INFO(this->get_logger(), "Using device: %s", device_.c_str());

        // Subscribers and publishers
        cmd_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/cmd_vel", 100, std::bind(&SerialOdomNode::pose_callback, this, std::placeholders::_1));

        odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("odom_data", 100);
        imu_pub_ = this->create_publisher<sensor_msgs::msg::Imu>("imu_data", 100);

        // Try to open serial port
        try {
            ser_.setPort(device_);
            ser_.setBaudrate(115200);
            serial::Timeout to = serial::Timeout::simpleTimeout(1000);
            ser_.setTimeout(to);
            ser_.open();
        } catch (serial::IOException &e) {
            RCLCPP_ERROR(this->get_logger(), "Unable to open port %s", device_.c_str());
            rclcpp::shutdown();
            return;
        }

        if (ser_.isOpen()) {
            RCLCPP_INFO(this->get_logger(), "Serial port initialized successfully.");
        } else {
            RCLCPP_ERROR(this->get_logger(), "Failed to open serial port!");
            rclcpp::shutdown();
            return;
        }

        // Initialize timers and variables
        last_time_ = this->now();
        timer_ = this->create_wall_timer(std::chrono::milliseconds(10), std::bind(&SerialOdomNode::loop, this));
    }

private:
    struct Var {
        float vector_linear;
        float vector_angular;
    } vector_data_;

    serial::Serial ser_;
    std::string device_;

    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
    rclcpp::TimerBase::SharedPtr timer_;

    rclcpp::Time last_time_;
    double x_ = 0.0, y_ = 0.0, th_ = 0.0;

    void pose_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        vector_data_.vector_linear = static_cast<float>(msg->linear.x);
        vector_data_.vector_angular = static_cast<float>(msg->angular.z);
        

        uint8_t tx_buf[(4 * 2) + 2] = {0};
        tx_buf[0] = 0xA0;
        tx_buf[(4 * 2) + 2 - 1] = 0x0A;
        memcpy(tx_buf + 1, (uint8_t *)(&vector_data_), 4 * 2);
        ser_.write(tx_buf, (4 * 2) + 2);
    }

    void loop()
    {
        
        if (!ser_.isOpen())
            return;

        if (ser_.available()) {
            uint8_t check = 0;
            ser_.read(&check, 1);
            if (check == 0xA0) {
                ser_.read(&check, 1);
                if (check == 0xA0) {
                    uint8_t read_data[32] = {0};
                    ser_.read(read_data, 32);
                    float *data_float = reinterpret_cast<float *>(read_data);

                    auto current_time = this->now();
                    double dt = (current_time - last_time_).seconds();

                    double delta_x = (data_float[0] * cos(th_) - 0 * sin(th_)) * dt;
                    double delta_y = (data_float[0] * sin(th_) + 0 * cos(th_)) * dt;
                    double delta_th = data_float[1] * dt;

                    x_ += delta_x;
                    y_ += delta_y;
                    th_ += delta_th;

                    // ---- Publish Odometry ----
                    tf2::Quaternion q;
                    q.setRPY(0, 0, th_);
                    geometry_msgs::msg::Quaternion odom_quat = tf2::toMsg(q);
                    
                    nav_msgs::msg::Odometry odom;
                    odom.header.stamp = current_time;
                    odom.header.frame_id = "odom";
                    odom.child_frame_id = "base_link";

                    odom.pose.pose.position.x = x_;
                    odom.pose.pose.position.y = y_;
                    odom.pose.pose.position.z = 0.0;
                    odom.pose.pose.orientation = odom_quat;

                    odom.twist.twist.linear.x = data_float[0];
                    odom.twist.twist.angular.z = data_float[1];

                    odom_pub_->publish(odom);

                    // ---- Publish IMU ----
                    sensor_msgs::msg::Imu imu;
                    imu.header.stamp = current_time;
                    imu.header.frame_id = "base_link";

                    if (fabs(data_float[5]) <= 0.4)
                        data_float[5] = 0.0;
                    if (fabs(data_float[6]) <= 0.4)
                        data_float[6] = 0.0;
                    if (fabs(data_float[7]) <= 0.4)
                        data_float[7] = 0.0;

                    imu.angular_velocity.x = data_float[5] * RAD;
                    imu.angular_velocity.y = data_float[6] * RAD;
                    imu.angular_velocity.z = data_float[7] * RAD * -0.5;

                    imu.linear_acceleration.x = data_float[2];
                    imu.linear_acceleration.y = data_float[3] * -1.0f;
                    imu.linear_acceleration.z = data_float[4] * -1.0f;

                    imu_pub_->publish(imu);
                    last_time_ = current_time;

                    ser_.flushInput();
                }
            }
        }
        RCLCPP_INFO(this->get_logger(),
        "vector_data.vector_linear = %f , vector_data.vector_angular = %f",
            vector_data_.vector_linear, vector_data_.vector_angular);

    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SerialOdomNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
