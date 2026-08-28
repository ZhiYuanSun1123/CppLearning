#include "audio_toolkit/audio_clip.hpp"

#include <iostream>

int main() {
    const audio_toolkit::AudioClip clip(
        "example.wav",
        16000,
        1,
        48000
    );

    std::cout
        << "path=" << clip.path() << '\n'
        << "sample_rate=" << clip.sample_rate() << '\n'
        << "channels=" << clip.channels() << '\n'
        << "duration=" << clip.duration_seconds()
        << "s\n";

    return 0;
}

