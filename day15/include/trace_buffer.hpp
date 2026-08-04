#pragma once
#include<cstddef>

class TraceBuffer{
public:
    explicit TraceBuffer(std::size_t size);
    ~TraceBuffer();
    
    TraceBuffer(
        const TraceBuffer& other
    );
    TraceBuffer& operator=(
        const TraceBuffer& other
    );
    TraceBuffer(
        TraceBuffer&& other
    ) noexcept;
    TraceBuffer& operator=(
        TraceBuffer&& other
    ) noexcept;

    std::size_t size() const noexcept;
    const double* data() const noexcept;

    void set(
        std::size_t index,
        double value
    );
    double get(std::size_t index) const;
    static int copy_constructor_count() noexcept;
    static int copy_assignment_count() noexcept;
    static int move_constructor_count() noexcept;
    static int move_assignment_count() noexcept;
    static void reset_counts() noexcept;
private:
    std::size_t size_;
    double* data_;

    inline static int copy_constructor_count_ = 0;
    inline static int copy_assignment_count_ = 0;
    inline static int move_constructor_count_ = 0;
    inline static int move_assignment_count_ = 0;
};