#include "audio_service/error_code.hpp"

#include <gtest/gtest.h>

TEST(ErrorCodeTest, UsesStableNumericRanges) {
    EXPECT_EQ(static_cast<int>(audio_service::ErrorCode::ok), 0);
    EXPECT_EQ(static_cast<int>(audio_service::ErrorCode::invalid_audio_path), 1001);
    EXPECT_EQ(static_cast<int>(audio_service::ErrorCode::audio_not_found), 2001);
    EXPECT_EQ(static_cast<int>(audio_service::ErrorCode::model_unavailable), 3001);
    EXPECT_EQ(static_cast<int>(audio_service::ErrorCode::internal_error), 9001);
}

TEST(ErrorCodeTest, MapsCodesToNamesAndCategories) {
    using audio_service::ErrorCode;
    EXPECT_EQ(audio_service::to_string(ErrorCode::invalid_question), "invalid_question");
    EXPECT_EQ(audio_service::error_category(ErrorCode::invalid_question), "input");
    EXPECT_EQ(audio_service::error_category(ErrorCode::decode_failed), "io");
    EXPECT_EQ(audio_service::error_category(ErrorCode::inference_failed), "model");
    EXPECT_EQ(audio_service::error_category(ErrorCode::internal_error), "internal");
}

TEST(ErrorCodeTest, OnlyOkIsSuccess) {
    EXPECT_TRUE(audio_service::is_success(audio_service::ErrorCode::ok));
    EXPECT_FALSE(audio_service::is_success(
        audio_service::ErrorCode::model_unavailable
    ));
}
