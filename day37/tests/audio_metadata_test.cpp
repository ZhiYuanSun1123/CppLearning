#include "audio_ci/audio_metadata.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

TEST(AudioMetadataTest, StoresMetadata) {
    const audio_ci::AudioMetadata metadata("speech.wav", 16000, 1, 32000);
    EXPECT_EQ(metadata.path(), "speech.wav");
    EXPECT_EQ(metadata.sample_rate(), 16000);
    EXPECT_EQ(metadata.channels(), 1);
    EXPECT_EQ(metadata.frame_count(), std::size_t{32000});
}

TEST(AudioMetadataTest, CalculatesFractionalDuration) {
    const audio_ci::AudioMetadata metadata("speech.wav", 16000, 1, 40000);
    EXPECT_DOUBLE_EQ(metadata.duration_seconds(), 2.5);
}

TEST(AudioMetadataTest, AcceptsZeroFrames) {
    const audio_ci::AudioMetadata metadata("empty.wav", 16000, 1, 0);
    EXPECT_DOUBLE_EQ(metadata.duration_seconds(), 0.0);
}

TEST(AudioMetadataTest, RejectsInvalidInput) {
    EXPECT_THROW(static_cast<void>(audio_ci::AudioMetadata("", 16000, 1, 10)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(audio_ci::AudioMetadata("a.wav", 0, 1, 10)),
                 std::invalid_argument);
}
