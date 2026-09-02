#include "audio_service/structured_logger.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>

TEST(StructuredLoggerTest, SerializesAllRequiredFields) {
    const audio_service::LogRecord record{
        "2026-09-02T09:00:00+08:00",
        audio_service::LogLevel::error,
        "inference_failed",
        "req-42",
        audio_service::ErrorCode::model_unavailable,
        "model is unavailable"
    };

    EXPECT_EQ(
        audio_service::to_json(record),
        "{\"timestamp\":\"2026-09-02T09:00:00+08:00\","
        "\"level\":\"ERROR\",\"event\":\"inference_failed\","
        "\"request_id\":\"req-42\",\"error_code\":3001,"
        "\"error_name\":\"model_unavailable\",\"category\":\"model\","
        "\"message\":\"model is unavailable\"}"
    );
}

TEST(StructuredLoggerTest, EscapesJsonSpecialCharacters) {
    EXPECT_EQ(
        audio_service::escape_json("line1\n\"audio\"\\file\tend"),
        "line1\\n\\\"audio\\\"\\\\file\\tend"
    );
}

TEST(StructuredLoggerTest, WritesExactlyOneJsonLine) {
    std::ostringstream output;
    audio_service::StructuredLogger logger(output);
    logger.log({
        "fixed-time", audio_service::LogLevel::info, "completed",
        "req-1", audio_service::ErrorCode::ok, "done"
    });

    const std::string text = output.str();
    ASSERT_FALSE(text.empty());
    EXPECT_EQ(text.back(), '\n');
    EXPECT_EQ(text.find('\n'), text.size() - 1);
    EXPECT_NE(text.find("\"error_code\":0"), std::string::npos);
}
