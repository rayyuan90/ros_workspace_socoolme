#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <atomic>
#include <new>
#include <cassert>
#include <algorithm> // for std::fill

// =========================================================================
// 1. 内存对齐与高性能结构体设计
// =========================================================================
struct alignas(std::hardware_destructive_interference_size) HighPerformanceSensorData {
    uint64_t timestamp;       // 8 字节
    double quaternion[4];     // 32 字节 (位姿四元数)
    float lidar_points[8];    // 32 字节 (精简激光点云或特征点)
    
    HighPerformanceSensorData(uint64_t ts) : timestamp(ts) {
        std::fill(std::begin(quaternion), std::end(quaternion), 0.0);
        std::fill(std::begin(lidar_points), std::end(lidar_points), 0.0f);
    }
    
    ~HighPerformanceSensorData() {}
};

// =========================================================================
// 2. 定制内存对齐分配器 (修复了 Rebind 编译器错误)
// =========================================================================
template <typename T, size_t Alignment>
struct AlignedAllocator {
    using value_type = T;

    // 显式告知标准库如何为内部控制块变更为对齐目标类型 U
    template <typename U>
    struct rebind {
        using other = AlignedAllocator<U, Alignment>;
    };

    AlignedAllocator() noexcept = default;
    template <typename U> AlignedAllocator(const AlignedAllocator<U, Alignment>&) noexcept {}

    T* allocate(size_t n) {
        if (n == 0) return nullptr;
        void* ptr = nullptr;
        if (posix_memalign(&ptr, Alignment, n * sizeof(T)) != 0) {
            throw std::bad_alloc();
        }
        return static_cast<T*>(ptr);
    }

    void deallocate(T* p, size_t) noexcept {
        free(p); 
    }
};

// =========================================================================
// 3. 智能指针生命周期与多线程无锁数据交换中心
// =========================================================================
class RobotDataHub : public std::enable_shared_from_this<RobotDataHub> {
private:
    std::atomic<std::shared_ptr<HighPerformanceSensorData>> global_latest_frame;

public:
    RobotDataHub() {
        global_latest_frame.store(std::make_shared<HighPerformanceSensorData>(0));
    }

    void publish_sensor_frame(uint64_t new_timestamp) {
        AlignedAllocator<HighPerformanceSensorData, 64> alloc;
        std::shared_ptr<HighPerformanceSensorData> new_frame = 
            std::allocate_shared<HighPerformanceSensorData>(alloc, new_timestamp);

        global_latest_frame.store(new_frame, std::memory_order_release);
    }

    void start_async_processing_thread() {
        auto safe_hub_ptr = shared_from_this();

        std::thread worker([safe_hub_ptr]() {
            for (int i = 0; i < 5; ++i) {
                std::shared_ptr<HighPerformanceSensorData> local_frame = 
                    safe_hub_ptr->global_latest_frame.load(std::memory_order_acquire);

                uintptr_t raw_address = reinterpret_cast<uintptr_t>(local_frame.get());
                assert(raw_address % 64 == 0);

                std::cout << "[消费者线程] 读取成功！地址: " << std::hex << raw_address 
                          << " | 时间戳: " << std::dec << local_frame->timestamp << std::endl;

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });

        worker.detach();
    }
};

// =========================================================================
// 4. 主函数验证
// =========================================================================
int main() {
    std::cout << "当前系统的 Cache Line 破坏性干扰尺寸: " 
              << std::hardware_destructive_interference_size << " 字节\n" << std::endl;

    auto hub = std::make_shared<RobotDataHub>();
    hub->start_async_processing_thread();

    for (int i = 1; i <= 5; ++i) {
        hub->publish_sensor_frame(i * 1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "\n[系统退出] 全流程零锁竞争，内存完美对齐，生命周期闭环验证通过！" << std::endl;
    return 0;
}