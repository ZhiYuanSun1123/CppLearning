#include "audio_toolkit/audio_clip.hpp"
#include <gtest/gtest.h>

TEST(MetadataExercise, StoresStereoMetadataAndDuration) {
    // TODO：meeting.wav、48000 Hz、2 声道、180000 帧。
    const audio_toolkit::AudioClip clip("meeting.wav",48000,2,180000);
    // 检查全部四项元数据、3.75 秒时长，以及重复读取不改变结果。
    EXPECT_EQ(clip.channels(),2);
    EXPECT_EQ(clip.duration_seconds(),3.75);
    EXPECT_EQ(clip.duration_seconds(),3.75);
    EXPECT_DOUBLE_EQ(clip.frame_count(),180000);
    EXPECT_EQ(clip.path(),"meeting.wav") << "路径出错";
    // 完成后删除下面的 GTEST_SKIP；不要只删除却不添加断言。
    // GTEST_SKIP() << "练习1尚未完成";
}
