#include "audio_toolkit/audio_clip.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

TEST(AudioClipTest, StoresMetadata) {
    const audio_toolkit::AudioClip clip(
        "speech.wav",
        16000,
        1,
        32000
    );

    EXPECT_EQ(clip.path(), "speech.wav");
    EXPECT_EQ(clip.sample_rate(), 16000);
    EXPECT_EQ(clip.channels(), 1);
    EXPECT_EQ(clip.frame_count(), 32000U);
}

TEST(AudioClipTest, CalculatesDuration) {
    const audio_toolkit::AudioClip clip(
        "speech.wav",
        16000,
        1,
        40000
    );

    EXPECT_DOUBLE_EQ(clip.duration_seconds(), 2.5);
}

TEST(AudioClipTest, RejectsInvalidMetadata) {
    EXPECT_THROW(
        audio_toolkit::AudioClip("", 16000, 1, 100),
        std::invalid_argument
    );

    EXPECT_THROW(
        audio_toolkit::AudioClip("a.wav", 0, 1, 100),
        std::invalid_argument
    );

    EXPECT_THROW(
        audio_toolkit::AudioClip("a.wav", 16000, 0, 100),
        std::invalid_argument
    );
}

