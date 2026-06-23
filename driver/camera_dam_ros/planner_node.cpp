// planner_node.cpp
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <geometry_msgs/msg/twist.hpp>

class PlannerNode : public rclcpp::Node {
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr sub;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub;

public:
    PlannerNode() : Node("planner_node") {
        // 【关键】订阅图像数据
        sub = this->create_subscription<sensor_msgs::msg::Image>(
            "camera/image_raw", 10,
            std::bind(&PlannerNode::decision_callback, this, std::placeholders::_1));
        cmd_pub = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
    }

    void decision_callback(const sensor_msgs::msg::Image::SharedPtr msg) {
        // 1. 在此处理路径规划算法
        // 2. 根据结果发布 Twist 指令
        auto cmd = geometry_msgs::msg::Twist();
        cmd.linear.x = 0.5; // 直行
        cmd_pub->publish(cmd);
    }
};