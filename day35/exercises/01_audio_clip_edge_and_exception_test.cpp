#include "audio_toolkit/audio_clip.hpp"
#include <gtest/gtest.h>

TEST(AudioClipExercise, CoversBoundaryAndInvalidInputs) {
    // TODO：按文档完成零帧边界、最小正值以及三类非法参数测试。
    const audio_toolkit::AudioClip clip_1("empty.wav",16000,1,0);
    const audio_toolkit::AudioClip clip_2("minimum.wav",1,1,1);
    // 至少使用 EXPECT_EQ、EXPECT_DOUBLE_EQ、EXPECT_THROW。
    EXPECT_EQ(clip_1.frame_count(),std::size_t{0});
    EXPECT_DOUBLE_EQ(clip_1.duration_seconds(),0.0);
    EXPECT_EQ(clip_2.sample_rate(),1);
    EXPECT_EQ(clip_2.channels(),1);
    EXPECT_EQ(clip_2.duration_seconds(),1);
    EXPECT_THROW(audio_toolkit::AudioClip("",16000,1,1),std::invalid_argument);
    EXPECT_THROW(audio_toolkit::AudioClip("empty.wav",0,1,1),std::invalid_argument);
    EXPECT_THROW(audio_toolkit::AudioClip("empty.wav",16000,-1,1),std::invalid_argument);
    // GTEST_SKIP() << "练习1尚未完成";
}
