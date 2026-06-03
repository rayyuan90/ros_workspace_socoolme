#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "tutorial_interfaces/msg/num.hpp"                                            // CHANGE
#include "rclcpp_components/register_node_macro.hpp"

using namespace std::chrono_literals;

namespace cpp_pubsub {

class MinimalPublisher : public rclcpp::Node
{
public:
  explicit MinimalPublisher(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Node("minimal_publisher", options), count_(0)
  {
    publisher_ = this->create_publisher<tutorial_interfaces::msg::Num>("topic", 10);  // CHANGE

    auto timer_callback = [this](){
      auto message = tutorial_interfaces::msg::Num();                                   // CHANGE
      message.num = this->count_++;                                                     // CHANGE
      RCLCPP_INFO_STREAM(this->get_logger(), "Publishing: '" << message.num << "'");    // CHANGE
      publisher_->publish(message);
    };
    timer_ = this->create_wall_timer(500ms, timer_callback);
  }

private:
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<tutorial_interfaces::msg::Num>::SharedPtr publisher_;             // CHANGE
  size_t count_;
};

} // namespace cpp_pubsub

RCLCPP_COMPONENTS_REGISTER_NODE(cpp_pubsub::MinimalPublisher)