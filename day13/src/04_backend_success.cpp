#include"qwen.hpp"
#include"whisper.hpp"
#include"inferenceService.hpp"
#include"request_result.hpp"
#include"model.hpp"
#include"testRunner.hpp"
void backend_success(
    TestRunner& runner
) {
    const WhisperBackend whisper(
        "Whisper"
    );
    const QwenOmniBackend qwen(
        "Qwen2.5-Omni"
    );
    const InferenceRequest request(
        "meeting.wav",
        "What happend?"
    );
    const InferenceService service;
    const InferenceResult result_whisper =
        service.run(
            whisper,
            request
        );
    const InferenceResult result_qwen = 
        service.run(
            qwen,
            request
        );
    runner.expect_equal(
        result_whisper.backend_name(),
        "Whisper",
        "Whisper 的结果来自于 Whisper"
    );
    runner.expect_true(
        result_whisper.output().find("Transcribe")
            != std::string::npos,
        "Whisper 输出包含 Transcribe"
    );
    runner.expect_equal(
        result_qwen.backend_name(),
        "Qwen2.5-Omni",
        "Qwen 的结果来自于 Qwen"
    );
    runner.expect_true(
        result_qwen.output().find("Reason about")
            != std::string::npos,
        "Qwen 输出包含 Reason about"
    );
    runner.expect_true(
        result_whisper.output().find(request.audio_path())
            != std::string::npos,
        "输入路径出现在结果中"
    );
    runner.expect_true(
        result_qwen.output().find(request.audio_path()) 
            != std::string::npos,
        "输入路径出现在结果中"
    );
}
int main(){
    TestRunner runner;
    backend_success(runner);
    return runner.final();
}