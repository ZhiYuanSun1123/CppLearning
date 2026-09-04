#include "audio_ci/audio_metadata.hpp"

#include <iostream>

int main() {
    const audio_ci::AudioMetadata metadata("speech.wav", 16000, 1, 40000);
    std::cout << metadata.path() << ": " << metadata.duration_seconds() << " s\n";
}
