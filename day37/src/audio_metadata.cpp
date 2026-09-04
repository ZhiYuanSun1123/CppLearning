#include "audio_ci/audio_metadata.hpp"

#include <stdexcept>
#include <utility>

namespace audio_ci {

AudioMetadata::AudioMetadata(std::string path, int sample_rate, int channels,
                             std::size_t frame_count)
    : path_(std::move(path)), sample_rate_(sample_rate), channels_(channels),
      frame_count_(frame_count) {
    if (path_.empty()) {
        throw std::invalid_argument("audio path must not be empty");
    }
    if (sample_rate_ <= 0) {
        throw std::invalid_argument("sample rate must be positive");
    }
    if (channels_ <= 0) {
        throw std::invalid_argument("channel count must be positive");
    }
}

const std::string& AudioMetadata::path() const noexcept { return path_; }
int AudioMetadata::sample_rate() const noexcept { return sample_rate_; }
int AudioMetadata::channels() const noexcept { return channels_; }
std::size_t AudioMetadata::frame_count() const noexcept { return frame_count_; }

double AudioMetadata::duration_seconds() const noexcept {
    return static_cast<double>(frame_count_) / static_cast<double>(sample_rate_);
}

} // namespace audio_ci
