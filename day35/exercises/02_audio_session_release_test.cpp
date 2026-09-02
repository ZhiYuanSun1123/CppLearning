#include "audio_toolkit/audio_session.hpp"
#include <gtest/gtest.h>

TEST(AudioSessionExercise, ReleasesExactlyOnceOnBothPaths) {
    // TODO：分别验证正常离开作用域与构造抛异常时，资源都恰好释放一次。
    auto state = std::make_shared<audio_toolkit::ResourceState>();
    // 两条路径使用两个独立的 ResourceState，避免测试相互污染。
    {
        audio_toolkit::AudioSession session(state,"qwen");
        EXPECT_EQ(state->active_count,1);
        EXPECT_EQ(state->acquire_count,1);
        EXPECT_EQ(state->release_count,0);
    }
    EXPECT_EQ(state->active_count,0);
    EXPECT_EQ(state->acquire_count,1);
    EXPECT_EQ(state->release_count,1);
    EXPECT_EQ(state.use_count(),1);
    auto failure_state = std::make_shared<audio_toolkit::ResourceState>();
    EXPECT_THROW(audio_toolkit::AudioSession(failure_state,"broken",true),std::runtime_error);
    EXPECT_EQ(state->active_count,0);
    EXPECT_EQ(state->acquire_count,1);
    EXPECT_EQ(state->release_count,1);
    // GTEST_SKIP() << "练习2尚未完成";
}
