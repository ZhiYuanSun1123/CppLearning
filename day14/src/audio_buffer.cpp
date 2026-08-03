#include"audio_buffer.hpp"
#include<stdexcept>

AudioBuffer::AudioBuffer(
    std::size_t size
) : size_(size),
    data_(
        size == 0
            ? nullptr
            : new double[size]{}
    ) {
}
AudioBuffer::~AudioBuffer(){
    delete[] data_;
}
AudioBuffer::AudioBuffer(
    const AudioBuffer& other
) : size_(other.size_),
    data_(other.data_) {
    if(size_ == 0){
        return;
    }
    data_ = new double[size_]{};
    for(
        std::size_t i = 0;
        i < size_;
        i++
    ) {
        data_[i] = other.data_[i];
    }
}
AudioBuffer::AudioBuffer(
    AudioBuffer&& other
) noexcept 
    : size_(other.size_),
      data_(other.data_) {
    other.size_ = 0;
    other.data_ = nullptr;
}
AudioBuffer& AudioBuffer::operator=(
    const AudioBuffer& other
) {
    if(this == &other){
        return *this;
    }
    // 为了防止new出错所以需要先用一个new_data存储other的信息防止data信息丢失
    double* new_data = nullptr;
    if(other.size_>0){
        new_data = new double[other.size_]{};
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
AudioBuffer& AudioBuffer::operator=(
    AudioBuffer&& other
) noexcept {
    if(this == &other){
        return *this;
    }
    delete[] data_;
    data_ = other.data_;
    size_ = other.size_;

    other.data_ = nullptr;
    other.size_ = 0;
    return *this;
}
std::size_t AudioBuffer::size() const noexcept{
    return size_;
}
void AudioBuffer::set(
    std::size_t index,
    double value
) {
    if(index >= size_){
        throw std::out_of_range(
            "AudioBuffer索引越界"
        );
    }
    data_[index] = value;
}
double AudioBuffer::get(
    std::size_t index
) const {
    if(index >= size_){
        throw std::out_of_range(
            "AudioBuffer索引越界"
        );
    }
    return data_[index];
}
const double* AudioBuffer::data() const noexcept{
    return data_;
}