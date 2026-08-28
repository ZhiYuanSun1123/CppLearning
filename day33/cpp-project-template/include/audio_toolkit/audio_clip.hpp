#ifndef AUDIO_TOOLKIT_AUDIO_CLIP_HPP
#define AUDIO_TOOLKIT_AUDIO_CLIP_HPP

#include <cstddef>
#include <string>

namespace audio_toolkit {

class AudioClip {
public:
    AudioClip(
        std::string path,
        int sample_rate,
        int channels,
        std::size_t frame_count
    );

    [[nodiscard]] const std::string&
    path() const noexcept;

    [[nodiscard]] int
    sample_rate() const noexcept;

    [[nodiscard]] int
    channels() const noexcept;

    [[nodiscard]] std::size_t
    frame_count() const noexcept;

    [[nodiscard]] double
    duration_seconds() const noexcept;

private:
    std::string path_;
    int sample_rate_;
    int channels_;
    std::size_t frame_count_;
};

} // namespace audio_toolkit

#endif

