#ifndef DAY37_AUDIO_CI_AUDIO_METADATA_HPP
#define DAY37_AUDIO_CI_AUDIO_METADATA_HPP

#include <cstddef>
#include <string>

namespace audio_ci {

class AudioMetadata {
  public:
    AudioMetadata(std::string path, int sample_rate, int channels,
                  std::size_t frame_count);

    const std::string& path() const noexcept;
    int sample_rate() const noexcept;
    int channels() const noexcept;
    std::size_t frame_count() const noexcept;
    double duration_seconds() const noexcept;

  private:
    std::string path_;
    int sample_rate_;
    int channels_;
    std::size_t frame_count_;
};

} // namespace audio_ci

#endif
