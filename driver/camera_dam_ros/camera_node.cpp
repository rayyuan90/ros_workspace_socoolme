// camera_node.cpp
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <opencv2/opencv.hpp>
#include <sys/mman.h>
#include <fcntl.h>

class CameraNode : public rclcpp::Node {
    uint8_t* dma_ptr;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr pub;

public:
    CameraNode() : Node("camera_node") {
        int fd = open("/dev/camera_dma", O_RDWR);
        // 【关键】DMA 指针映射：让 cv::Mat 直接指向 DMA 物理内存
        dma_ptr = (uint8_t*)mmap(NULL, 1280*720*3, PROT_READ, MAP_SHARED, fd, 0);
        pub = this->create_publisher<sensor_msgs::msg::Image>("camera/image_raw", 10);
        
        timer = create_wall_timer(std::chrono::milliseconds(33), [this](){
            // 构造 cv::Mat，内存完全共享
            cv::Mat frame(720, 1280, CV_8UC3, dma_ptr); 
            auto msg = sensor_msgs::msg::Image();
            // 填充 msg 并发布...
            pub->publish(msg);
        });
    }
};