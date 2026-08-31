#include "audio_toolkit/audio_clip.hpp"

#include <stdexcept>
#include <utility>

namespace audio_toolkit {

AudioClip::AudioClip(std::string path, int sample_rate, int channels,
                     std::size_t frame_count)
    : path_(std::move(path)), sample_rate_(sample_rate),
      channels_(channels), frame_count_(frame_count) {
    if (path_.empty() || sample_rate_ <= 0 || channels_ <= 0) {
        throw std::invalid_argument("invalid audio metadata");
    }
}

const std::string& AudioClip::path() const noexcept { return path_; }
int AudioClip::sample_rate() const noexcept { return sample_rate_; }
int AudioClip::channels() const noexcept { return channels_; }
std::size_t AudioClip::frame_count() const noexcept { return frame_count_; }

double AudioClip::duration_seconds() const noexcept {
    return static_cast<double>(frame_count_) /
           static_cast<double>(sample_rate_);
}

} // namespace audio_toolkit
