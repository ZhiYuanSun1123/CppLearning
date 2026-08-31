#include "audio_toolkit/audio_clip.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

// 一个 TEST 是一个测试；多个 EXPECT 是这个测试中的多个断言。
TEST(AudioClipTest, StoresMonoMetadata) {
    const audio_toolkit::AudioClip clip("speech.wav", 16000, 1, 32000);

    EXPECT_EQ(clip.path(), "speech.wav");
    EXPECT_EQ(clip.sample_rate(), 16000);
    EXPECT_EQ(clip.channels(), 1);
    EXPECT_EQ(clip.frame_count(), std::size_t{32000});
}

TEST(AudioClipTest, CalculatesFractionalDuration) {
    // Arrange：准备一个预期长度为 2.5 秒的片段。
    const audio_toolkit::AudioClip clip("speech.wav", 16000, 1, 40000);
    // Act：调用待测接口。
    const double actual = clip.duration_seconds();
    // Assert：检查结果，而不是只把结果打印出来。
    EXPECT_DOUBLE_EQ(actual, 2.5);
}

TEST(AudioClipTest, StereoDurationUsesFramesNotSamples) {
    const audio_toolkit::AudioClip clip("stereo.wav", 48000, 2, 96000);
    EXPECT_EQ(clip.channels(), 2);
    EXPECT_DOUBLE_EQ(clip.duration_seconds(), 2.0);
}

TEST(AudioClipTest, CalculatesNonTerminatingDuration) {
    const audio_toolkit::AudioClip clip("short.wav", 48000, 1, 16000);
    EXPECT_NEAR(clip.duration_seconds(), 0.3333333333333333, 1e-12);
}

TEST(AudioClipTest, ReadingDurationDoesNotChangeMetadata) {
    const audio_toolkit::AudioClip clip("music.wav", 44100, 2, 88200);
    EXPECT_DOUBLE_EQ(clip.duration_seconds(), 2.0);
    EXPECT_DOUBLE_EQ(clip.duration_seconds(), 2.0);
    EXPECT_EQ(clip.path(), "music.wav");
    EXPECT_EQ(clip.frame_count(), std::size_t{88200});
}

TEST(AudioClipTest, DifferentObjectsKeepIndependentMetadata) {
    const audio_toolkit::AudioClip first("a.wav", 16000, 1, 16000);
    const audio_toolkit::AudioClip second("b.wav", 48000, 2, 144000);
    EXPECT_NE(first.path(), second.path());
    EXPECT_DOUBLE_EQ(first.duration_seconds(), 1.0);
    EXPECT_DOUBLE_EQ(second.duration_seconds(), 3.0);
}

TEST(AudioBatchTest, ReadsFirstClipAfterCheckingSize) {
    const std::vector<audio_toolkit::AudioClip> clips{
        audio_toolkit::AudioClip("first.wav", 16000, 1, 32000),
        audio_toolkit::AudioClip("second.wav", 48000, 2, 48000)
    };

    // 若前置条件失败，不能继续访问 clips[0]。
    ASSERT_EQ(clips.size(), std::size_t{2});
    EXPECT_EQ(clips[0].path(), "first.wav");
    EXPECT_DOUBLE_EQ(clips[0].duration_seconds(), 2.0);
}
