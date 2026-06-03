cmake_minimum_required(VERSION 3.5)
project(cpp_pubsub)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(tutorial_interfaces REQUIRED)

# Build a shared library with both components registered
add_library(cpp_pubsub_components SHARED
  src/publisher_lambda_function.cpp
  src/subscriber_lambda_function.cpp)

target_compile_definitions(cpp_pubsub_components PRIVATE "COMPOSITION_BUILDING_DLL")
ament_target_dependencies(cpp_pubsub_components rclcpp rclcpp_components tutorial_interfaces)

# Register components (done in source with macros as well)
rclcpp_components_register_node(cpp_pubsub_components PLUGIN "cpp_pubsub::MinimalPublisher" EXECUTABLE talker)
rclcpp_components_register_node(cpp_pubsub_components PLUGIN "cpp_pubsub::MinimalSubscriber" EXECUTABLE listener)

install(TARGETS
  cpp_pubsub_components
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

# Install launch files
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()