#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "more_interfaces/msg/address_book.hpp"                                     // CHANGE

using std::placeholders::_1;

class AddressBookSubscriber : public rclcpp::Node
{
public:
  AddressBookSubscriber()
  : Node("address_book_subscriber")
  {
    auto topic_callback = [this](const more_interfaces::msg::AddressBook & msg){     // CHANGE
      RCLCPP_INFO_STREAM(this->get_logger(), "I heard: '" << msg.first_name << " " << msg.last_name << "'");    // CHANGE
    };
    subscription_ = this->create_subscription<more_interfaces::msg::AddressBook>(    // CHANGE
      "address_book", 10, topic_callback);
  }

private:
  rclcpp::Subscription<more_interfaces::msg::AddressBook>::SharedPtr subscription_;  // CHANGE
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<AddressBookSubscriber>());
  rclcpp::shutdown();
  return 0;
}