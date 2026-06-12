#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include <rosbag2_cpp/writer.hpp>

class SimpleBagRecorder : public rclcpp::Node
{
public:
  SimpleBagRecorder()
  : Node("simple_bag_recorder")
  {
    writer_ = std::make_unique<rosbag2_cpp::Writer>();

    writer_->open("my_bag");

    auto subscription_callback_lambda = [this](std::shared_ptr<const rclcpp::SerializedMessage> msg){
      rclcpp::Time time_stamp = this->now();

      writer_->write(msg, "chatter", "std_msgs/msg/String", time_stamp);
    };

    subscription_ = create_subscription<std_msgs::msg::String>(
      "chatter", 10, subscription_callback_lambda);
  }

private:

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
  std::unique_ptr<rosbag2_cpp::Writer> writer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SimpleBagRecorder>());
  rclcpp::shutdown();
  return 0;
}