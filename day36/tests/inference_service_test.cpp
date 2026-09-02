#include "audio_service/inference_service.hpp"

#include <gtest/gtest.h>

TEST(InferenceServiceTest, ReturnsSuccessForValidRequest) {
    const audio_service::InferenceService service;
    const auto response = service.run({"speech.wav", "What is spoken?"});

    EXPECT_TRUE(response.ok());
    EXPECT_EQ(response.code, audio_service::ErrorCode::ok);
    EXPECT_EQ(response.answer, "simulated answer");
}

TEST(InferenceServiceTest, ReturnsInputCodeForEmptyAudioPath) {
    const audio_service::InferenceService service;
    const auto response = service.run({"", "question"});

    EXPECT_FALSE(response.ok());
    EXPECT_EQ(response.code, audio_service::ErrorCode::invalid_audio_path);
    EXPECT_TRUE(response.answer.empty());
}

TEST(InferenceServiceTest, ReturnsIoCodeForMissingAudio) {
    const audio_service::InferenceService service;
    const auto response = service.run({"missing.wav", "question"});
    EXPECT_EQ(response.code, audio_service::ErrorCode::audio_not_found);
    EXPECT_EQ(audio_service::error_category(response.code), "io");
}

TEST(InferenceServiceTest, ReturnsModelCodeWhenBackendIsUnavailable) {
    const audio_service::InferenceService service(false);
    const auto response = service.run({"speech.wav", "question"});
    EXPECT_EQ(response.code, audio_service::ErrorCode::model_unavailable);
    EXPECT_EQ(audio_service::error_category(response.code), "model");
}

TEST(InferenceServiceTest, BuildsInfoOrErrorLogFromResult) {
    const audio_service::InferenceService service;
    const auto success = service.run({"speech.wav", "question"});
    const auto failure = service.run({"", "question"});

    const auto success_log = audio_service::make_log_record(success, "r1", "t1");
    const auto failure_log = audio_service::make_log_record(failure, "r2", "t2");
    EXPECT_EQ(success_log.level, audio_service::LogLevel::info);
    EXPECT_EQ(success_log.event, "inference_completed");
    EXPECT_EQ(failure_log.level, audio_service::LogLevel::error);
    EXPECT_EQ(failure_log.error_code, audio_service::ErrorCode::invalid_audio_path);
}
