#include "audio_toolkit/audio_clip.hpp"

#include <iostream>

int main() {
    const audio_toolkit::AudioClip empty("empty.wav", 16000, 1, 0);
    std::cout << empty.path() << ": " << empty.duration_seconds() << " s\n";
}
