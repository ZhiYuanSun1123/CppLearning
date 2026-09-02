#ifndef DAY36_AUDIO_SERVICE_INFERENCE_SERVICE_HPP
#define DAY36_AUDIO_SERVICE_INFERENCE_SERVICE_HPP

#include "audio_service/error_code.hpp"
#include "audio_service/structured_logger.hpp"

#include <string>

namespace audio_service {

struct InferenceRequest {
    std::string audio_path;
    std::string question;
};

struct InferenceResponse {
    ErrorCode code{ErrorCode::ok};
    std::string answer;
    std::string message;

    bool ok() const noexcept;
};

class InferenceService {
public:
    explicit InferenceService(bool model_available = true) noexcept;
    InferenceResponse run(const InferenceRequest& request) const;

private:
    bool model_available_;
};

LogRecord make_log_record(const InferenceResponse& response,
                          std::string request_id,
                          std::string timestamp);

} // namespace audio_service

#endif
