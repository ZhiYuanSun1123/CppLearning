#include"whisper.hpp"
#include"qwen.hpp"
#include"request_result.hpp"
#include"inferenceService.hpp"
int main() {
    try {
        const WhisperBackend whisper(
            "Whisper"
        );

        const QwenOmniBackend qwen(
            "Qwen2.5-Omni"
        );

        const InferenceRequest request(
            "meeting.wav",
            "What happened in the audio?"
        );

        const InferenceService service;

        service.run(
            whisper,
            request
        );

        service.run(
            qwen,
            request
        );

        std::cout
            << "Whisper calls: "
            << WhisperBackend::inference_count()
            << '\n';

        std::cout
            << "Qwen calls: "
            << QwenOmniBackend::inference_count()
            << '\n';
    } catch (
        const std::exception& error
    ) {
        std::cerr
            << "Error: "
            << error.what()
            << '\n';

        return 1;
    }

    return 0;
}