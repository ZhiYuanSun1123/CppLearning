#include "audio_toolkit/audio_clip.hpp"
#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

TEST(BatchExercise, SumsDurationAndPreservesInput) {
    // TODO：构造文档指定的三个片段，先 ASSERT 大小，再求和并检查。
    std::vector<audio_toolkit::AudioClip> list;
    list.push_back(audio_toolkit::AudioClip("a.wav",16000,1,16000));
    list.push_back(audio_toolkit::AudioClip("b.wav",48000,2,120000));
    list.push_back(audio_toolkit::AudioClip("c.wav",16000,1,20000));
    // 检查总时长为 4.75 秒，且三个片段路径与帧数不变。
    double sum = 0.0;
    for(auto item:list){
        sum+=item.duration_seconds();
    }
    EXPECT_DOUBLE_EQ(sum,4.75);
    // GTEST_SKIP() << "练习2尚未完成";
}
