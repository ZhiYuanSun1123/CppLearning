#include "audio_toolkit/audio_clip.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>
#include <string>

TEST(AudioClipBoundaryTest, AcceptsZeroFramesAsEmptyClip) {
    const audio_toolkit::AudioClip clip("empty.wav", 16000, 1, 0);
    EXPECT_EQ(clip.frame_count(), std::size_t{0});
    EXPECT_DOUBLE_EQ(clip.duration_seconds(), 0.0);
}

TEST(AudioClipBoundaryTest, AcceptsSmallestPositiveRateAndChannelCount) {
    const audio_toolkit::AudioClip clip("one-frame.wav", 1, 1, 1);
    EXPECT_EQ(clip.sample_rate(), 1);
    EXPECT_EQ(clip.channels(), 1);
    EXPECT_DOUBLE_EQ(clip.duration_seconds(), 1.0);
}

TEST(AudioClipExceptionTest, RejectsEmptyPath) {
    EXPECT_THROW(
        static_cast<void>(audio_toolkit::AudioClip("", 16000, 1, 10)),
        std::invalid_argument
    );
}

TEST(AudioClipExceptionTest, RejectsZeroAndNegativeSampleRate) {
    EXPECT_THROW(
        static_cast<void>(audio_toolkit::AudioClip("a.wav", 0, 1, 10)),
        std::invalid_argument
    );
    EXPECT_THROW(
        static_cast<void>(audio_toolkit::AudioClip("a.wav", -1, 1, 10)),
        std::invalid_argument
    );
}

TEST(AudioClipExceptionTest, RejectsZeroAndNegativeChannelCount) {
    EXPECT_THROW(
        static_cast<void>(audio_toolkit::AudioClip("a.wav", 16000, 0, 10)),
        std::invalid_argument
    );
    EXPECT_THROW(
        static_cast<void>(audio_toolkit::AudioClip("a.wav", 16000, -1, 10)),
        std::invalid_argument
    );
}

TEST(AudioClipExceptionTest, ReportsSpecificReason) {
    try {
        static_cast<void>(audio_toolkit::AudioClip("a.wav", 0, 1, 10));
        FAIL() << "expected std::invalid_argument";
    } catch (const std::invalid_argument& error) {
        EXPECT_EQ(std::string(error.what()), "sample rate must be positive");
    } catch (...) {
        FAIL() << "expected std::invalid_argument, but caught another type";
    }
}
