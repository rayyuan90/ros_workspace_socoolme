    #include <iostream>
    #include <vector>
    #include <cstddef>
    #include <new>

    // ============================================================================
    // 1. THE BACKING MEMORY ARENA
    // ============================================================================
    class MemoryArena {
    private:
        char* m_buffer;
        size_t m_size;
        size_t m_offset;

    public:
        MemoryArena(size_t bytes) : m_size(bytes), m_offset(0) {
            m_buffer = new char[m_size]; // Allocate the giant pool once
        }

        ~MemoryArena() {
            delete[] m_buffer;
        }

        // Allocate raw chunks from our buffer
        void* allocate(size_t bytes) {
            // Ensure alignment (e.g., 8-byte alignment for modern CPUs)
            size_t aligned_bytes = (bytes + 7) & ~7;

            if (m_offset + aligned_bytes > m_size) {
                std::cout << "[Arena] OUT OF MEMORY!\n";
                throw std::bad_alloc();
            }

            void* ptr = &m_buffer[m_offset];
            m_offset += aligned_bytes;
            std::cout << "[Arena] Allocated " << aligned_bytes << " bytes. Offset is now: " << m_offset << "\n";
            return ptr;
        }

        // Reset the offset to reuse the entire block of memory instantly
        void reset() {
            m_offset = 0;
            std::cout << "[Arena] Reset completely!\n";
        }

        // Disable copying
        MemoryArena(const MemoryArena&) = delete;
        void operator=(const MemoryArena&) = delete;
    };

    // ============================================================================
    // 2. THE C++ COMPLIANT CUSTOM ALLOCATOR
    // ============================================================================
    template <typename T>
    class ArenaAllocator {
    public:
        // Required boilerplate traits for C++ containers
        using value_type = T;

        MemoryArena& arena; // Reference to our backing arena

        // Constructor hooks into our specific arena instance
        ArenaAllocator(MemoryArena& backing_arena) noexcept : arena(backing_arena) {}

        // Type-converting constructor (required for containers to allocate internal nodes)
        template <typename U>
        ArenaAllocator(const ArenaAllocator<U>& other) noexcept : arena(other.arena) {}

        // The actual allocation mechanism called by std::vector
        T* allocate(std::size_t n) {
            return static_cast<T*>(arena.allocate(n * sizeof(T)));
        }

        // Deallocate is a NO-OP because the Arena manages memory as a whole block
        void deallocate(T* p, std::size_t n) noexcept {
            // We do nothing here! Individual elements aren't freed.
            // The memory is reclaimed all at once when arena.reset() is called.
        }

        // Boilerplate equality operators
        template <typename U>
        bool operator==(const ArenaAllocator<U>& other) const noexcept { return &arena == &other.arena; }
        
        template <typename U>
        bool operator!=(const ArenaAllocator<U>& other) const noexcept { return !(*this == other); }
    };

    // ============================================================================
    // 3. EXECUTION & VERIFICATION
    // ============================================================================
    int main() {
        std::cout << "--- Initializing 1KB Arena ---\n";
        MemoryArena drone_hardware_arena(1024); // 1024 bytes of pre-allocated memory

        std::cout << "\n--- Creating Vector Using Custom Allocator ---\n";
        // We pass our custom allocator type and inject our specific arena instance
        ArenaAllocator<int> custom_alloc(drone_hardware_arena);
        std::vector<int, ArenaAllocator<int>> telemetry_data(custom_alloc);

        std::cout << "\n--- Pushing Data (Triggers Allocations) ---\n";
        // Watch the console output closely here as the vector grows and reallocates!
        telemetry_data.push_back(100); 
        telemetry_data.push_back(200);
        telemetry_data.push_back(300);
        telemetry_data.push_back(400);
         telemetry_data.push_back(500);

        std::cout << "\n--- Vector Contents ---\n";
        for (int val : telemetry_data) {
            std::cout << val << " ";
        }
        std::cout << "\n";

        std::cout << "\n--- Clearing Vector and Resetting Arena ---\n";
        telemetry_data.clear(); // Clears vector tracking logic
        drone_hardware_arena.reset(); // Instant reclamation of all 1024 bytes

        return 0;
    }
