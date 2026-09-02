#include "audio_service/error_code.hpp"

namespace audio_service {

std::string_view to_string(ErrorCode code) noexcept {
    switch (code) {
    case ErrorCode::ok: return "ok";
    case ErrorCode::invalid_audio_path: return "invalid_audio_path";
    case ErrorCode::invalid_question: return "invalid_question";
    case ErrorCode::audio_not_found: return "audio_not_found";
    case ErrorCode::decode_failed: return "decode_failed";
    case ErrorCode::model_unavailable: return "model_unavailable";
    case ErrorCode::inference_failed: return "inference_failed";
    case ErrorCode::internal_error: return "internal_error";
    }
    return "unknown_error";
}

std::string_view error_category(ErrorCode code) noexcept {
    const int value = static_cast<int>(code);
    if (value == 0) return "success";
    if (value >= 1000 && value < 2000) return "input";
    if (value >= 2000 && value < 3000) return "io";
    if (value >= 3000 && value < 4000) return "model";
    if (value >= 9000 && value < 10000) return "internal";
    return "unknown";
}

bool is_success(ErrorCode code) noexcept {
    return code == ErrorCode::ok;
}

} // namespace audio_service
