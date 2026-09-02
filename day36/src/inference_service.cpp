#include "audio_service/inference_service.hpp"

#include <utility>

namespace audio_service {

bool InferenceResponse::ok() const noexcept {
    return is_success(code);
}

InferenceService::InferenceService(bool model_available) noexcept
    : model_available_(model_available) {}

InferenceResponse InferenceService::run(const InferenceRequest& request) const {
    if (request.audio_path.empty()) {
        return {ErrorCode::invalid_audio_path, "", "audio path is empty"};
    }
    if (request.question.empty()) {
        return {ErrorCode::invalid_question, "", "question is empty"};
    }
    if (request.audio_path == "missing.wav") {
        return {ErrorCode::audio_not_found, "", "audio file was not found"};
    }
    if (!model_available_) {
        return {ErrorCode::model_unavailable, "", "model is unavailable"};
    }
    if (request.question == "force-failure") {
        return {ErrorCode::inference_failed, "", "inference failed"};
    }
    return {ErrorCode::ok, "simulated answer", "inference succeeded"};
}

LogRecord make_log_record(const InferenceResponse& response,
                          std::string request_id,
                          std::string timestamp) {
    return {
        std::move(timestamp),
        response.ok() ? LogLevel::info : LogLevel::error,
        response.ok() ? "inference_completed" : "inference_failed",
        std::move(request_id),
        response.code,
        response.message
    };
}

} // namespace audio_service
