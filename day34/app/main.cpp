#include "audio_toolkit/audio_clip.hpp"

#include <iostream>

int main() {
    const audio_toolkit::AudioClip clip("speech.wav", 16000, 1, 40000);
    std::cout << clip.path() << ": " << clip.duration_seconds() << " s\n";
}
