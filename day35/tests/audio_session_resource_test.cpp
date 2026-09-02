#include "audio_toolkit/audio_session.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

TEST(AudioSessionResourceTest, ReleasesResourceAfterNormalScopeExit) {
    const auto state = std::make_shared<audio_toolkit::ResourceState>();
    {
        const audio_toolkit::AudioSession session(state, "qwen-omni");
        EXPECT_EQ(session.model_name(), "qwen-omni");
        EXPECT_EQ(state->active_count, 1);
        EXPECT_EQ(state->acquire_count, 1);
        EXPECT_EQ(state->release_count, 0);
    }

    EXPECT_EQ(state->active_count, 0);
    EXPECT_EQ(state->acquire_count, 1);
    EXPECT_EQ(state->release_count, 1);
}

TEST(AudioSessionResourceTest, ReleasesMemberWhenConstructorThrows) {
    const auto state = std::make_shared<audio_toolkit::ResourceState>();

    EXPECT_THROW(
        static_cast<void>(audio_toolkit::AudioSession(
            state, "broken-model", true
        )),
        std::runtime_error
    );

    // AudioSession 没有构造成功，但已经构造成功的 resource_ 会被析构。
    EXPECT_EQ(state->active_count, 0);
    EXPECT_EQ(state->acquire_count, 1);
    EXPECT_EQ(state->release_count, 1);
}

TEST(AudioSessionResourceTest, ReleasesMemberWhenModelNameIsInvalid) {
    const auto state = std::make_shared<audio_toolkit::ResourceState>();

    EXPECT_THROW(
        static_cast<void>(audio_toolkit::AudioSession(state, "")),
        std::invalid_argument
    );

    EXPECT_EQ(state->active_count, 0);
    EXPECT_EQ(state->acquire_count, state->release_count);
}

TEST(AudioSessionResourceTest, NullStateAcquiresNothing) {
    EXPECT_THROW(
        static_cast<void>(audio_toolkit::AudioSession(nullptr, "model")),
        std::invalid_argument
    );
}
