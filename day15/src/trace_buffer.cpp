#include"trace_buffer.hpp"
#include"stdexcept"
TraceBuffer::TraceBuffer(
    std::size_t size
) : size_(size),
    data_(size == 0
                ? nullptr
                : new double[size]{}){}
TraceBuffer::~TraceBuffer(){
    delete[] data_;
}
TraceBuffer::TraceBuffer(
    const TraceBuffer& other
) : size_(other.size_),
    data_(nullptr) {
    ++copy_constructor_count_;
    if(size_ == 0)
        return;
    data_ = new double[size_]{};
    for(
        std::size_t i = 0;
        i < size_;
        i++
    ){
        data_[i] = other.data_[i];
    }
}
TraceBuffer& TraceBuffer::operator=(
    const TraceBuffer& other
) {
    ++copy_assignment_count_;
    if(this == &other)
        return *this;
    double* new_data = nullptr;
    if(other.size_>0){
        new_data = new double[other.size_];
        for(
            std::size_t i = 0;
            i < other.size_;
            i++
        ) {
            new_data[i] = other.data_[i];
        }
    }
    delete[] data_;
    data_ = new_data;
    size_ = other.size_;
    return *this;
}
TraceBuffer::TraceBuffer(
    TraceBuffer&& other
) noexcept
    : size_(other.size_),
      data_(other.data_){
    ++move_constructor_count_;

    other.size_ = 0;
    other.data_ = nullptr;
}
TraceBuffer& TraceBuffer::operator=(
    TraceBuffer&& other
) noexcept{
    ++move_assignment_count_;
    if(this == &other)
        return *this;
    delete[] data_;
    data_ = other.data_;
    size_ = other.size_;
    
    other.size_ = 0;
    other.data_ = nullptr;

    return *this;
}
std::size_t TraceBuffer::size() const noexcept{
    return size_;
}
const double* TraceBuffer::data() const noexcept{
    return data_;
}
void TraceBuffer::set(
    std::size_t index,
    double value
) {
    if(index >= size_)
        throw std::out_of_range(
            "TraceBuffer index out of range"
        );
    data_[index] = value;
}
double TraceBuffer::get(
    std::size_t index
) const {
    if(index >= size_)
        throw std::out_of_range(
            "TraceBuffer index out of range"
        );
    return data_[index];
}
int TraceBuffer::copy_constructor_count() noexcept {
    return copy_constructor_count_;
}

int TraceBuffer::copy_assignment_count() noexcept {
    return copy_assignment_count_;
}

int TraceBuffer::move_constructor_count() noexcept {
    return move_constructor_count_;
}

int TraceBuffer::move_assignment_count() noexcept {
    return move_assignment_count_;
}
void TraceBuffer::reset_counts() noexcept {
    copy_constructor_count_ = 0;
    copy_assignment_count_ = 0;
    move_constructor_count_ = 0;
    move_assignment_count_ = 0;
}