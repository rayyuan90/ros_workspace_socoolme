#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class SyncAsyncSubscriber : public rclcpp::Node
{
public:

    SyncAsyncSubscriber()
        : Node("sync_async_subscriber")
    {
        // Lambda function to run every time a new message is received
        auto topic_callback = [this](const std_msgs::msg::String & msg){
            RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg.data.c_str());
        };

        // Create the synchronous subscriber on topic 'sync_topic'
        // and tie it to the topic_callback
        sync_subscription_ = this->create_subscription<std_msgs::msg::String>(
            "sync_topic", 10, topic_callback);

        // Create the asynchronous subscriber on topic 'async_topic'
        // and tie it to the topic_callback
        async_subscription_ = this->create_subscription<std_msgs::msg::String>(
            "async_topic", 10, topic_callback);
    }

private:

    // A subscriber that listens to topic 'sync_topic'
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sync_subscription_;

    // A subscriber that listens to topic 'async_topic'
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr async_subscription_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SyncAsyncSubscriber>());
    rclcpp::shutdown();
    return 0;
}