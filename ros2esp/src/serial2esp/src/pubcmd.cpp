#include <chrono>
#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

using namespace std::chrono_literals;

class CmdPublisher : public rclcpp::Node
{
public:
    CmdPublisher()
        : Node("pubcmd_node")
    {
        // 建立 Publisher，topic 名稱為 /cmd_vel，queue size 10
        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

        // 建立 timer，每 10ms 發布一次（對應 ROS 1 的 loop_rate 100Hz）
        timer_ = this->create_wall_timer(
            10ms, std::bind(&CmdPublisher::timer_callback, this));
    }

private:
    void timer_callback()
    {
        auto msg = geometry_msgs::msg::Twist();
        msg.linear.x = 0.1;
        msg.angular.z = 0;

        publisher_->publish(msg);

        RCLCPP_INFO(this->get_logger(), 
                    "Publish Info: linear: %f angular: %f", 
                    msg.linear.x, msg.angular.z);
    }

    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CmdPublisher>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}