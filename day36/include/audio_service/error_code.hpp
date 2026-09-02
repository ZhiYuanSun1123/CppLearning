#ifndef DAY36_AUDIO_SERVICE_ERROR_CODE_HPP
#define DAY36_AUDIO_SERVICE_ERROR_CODE_HPP

#include <string_view>

namespace audio_service {

// 千位用于错误大类，后三位用于类内具体错误。
enum class ErrorCode : int {
    ok = 0,
    invalid_audio_path = 1001,
    invalid_question = 1002,
    audio_not_found = 2001,
    decode_failed = 2002,
    model_unavailable = 3001,
    inference_failed = 3002,
    internal_error = 9001
};

std::string_view to_string(ErrorCode code) noexcept;
std::string_view error_category(ErrorCode code) noexcept;
bool is_success(ErrorCode code) noexcept;

} // namespace audio_service

#endif
