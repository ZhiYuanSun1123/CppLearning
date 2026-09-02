#include "audio_service/inference_service.hpp"
#include "audio_service/structured_logger.hpp"

#include <iostream>

int main() {
    const audio_service::InferenceService service;
    const audio_service::InferenceResponse response = service.run({
        "meeting.wav", "What happened in the audio?"
    });

    audio_service::StructuredLogger logger(std::cout);
    logger.log(audio_service::make_log_record(
        response, "req-001", "2026-09-02T09:00:00+08:00"
    ));

    return response.ok() ? 0 : 1;
}
