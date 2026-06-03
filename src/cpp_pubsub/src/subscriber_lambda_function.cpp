#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "tutorial_interfaces/msg/num.hpp"                                       // CHANGE
#include "rclcpp_components/register_node_macro.hpp"

using std::placeholders::_1;

namespace cpp_pubsub {

class MinimalSubscriber : public rclcpp::Node
{
public:
  explicit MinimalSubscriber(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Node("minimal_subscriber", options)
  {
    auto topic_callback = [this](const tutorial_interfaces::msg::Num & msg){     // CHANGE
      RCLCPP_INFO_STREAM(this->get_logger(), "I heard: '" << msg.num << "'");    // CHANGE
    };
    subscription_ = this->create_subscription<tutorial_interfaces::msg::Num>(    // CHANGE
      "topic", 10, topic_callback);
  }

private:
  rclcpp::Subscription<tutorial_interfaces::msg::Num>::SharedPtr subscription_;  // CHANGE
};

} // namespace cpp_pubsub

RCLCPP_COMPONENTS_REGISTER_NODE(cpp_pubsub::MinimalSubscriber)