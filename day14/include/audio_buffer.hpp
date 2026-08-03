#pragma once
#include<cstddef>
class AudioBuffer{
public:
    explicit AudioBuffer(std::size_t size);
    ~AudioBuffer();
    AudioBuffer(
        const AudioBuffer& other
    );
    AudioBuffer& operator=(
        const AudioBuffer& other
    );
    AudioBuffer(
        AudioBuffer&& other
    ) noexcept;
    AudioBuffer& operator=(
        AudioBuffer&& other
    ) noexcept;

    std::size_t size() const noexcept;

    void set(
        std::size_t index,
        double value
    );
    double get(std::size_t index) const;
    const double* data() const noexcept;
private:
    std::size_t size_;
    double* data_;
};