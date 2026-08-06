#include"testRunner.hpp"
#include"request_result.hpp"
#include"inferenceService.hpp"
#include"string"
#include"whisper.hpp"
#include"qwen.hpp"
//基类引用推理的辅助函数
InferenceResult run_through_interface(
    const ModelBackend& backend,
    const InferenceRequest& request
) {
    return backend.infer(request);
}
// 测试模型不可用
class UnavailableBackend
    : public ModelBackend {
public:
    UnavailableBackend()
        : ModelBackend(
              "UnavailableBackend"
          ) {
    }

    bool is_ready() const override {
        return false;
    }

    InferenceResult infer(
        const InferenceRequest& request
    ) const override {
        return InferenceResult(
            name(),
            "Unexpected inference: "
                + request.audio_path()
        );
    }
};
// 测试推理失败
class FailingBackend
    : public ModelBackend {
public:
    FailingBackend()
        : ModelBackend(
              "FailingBackend"
          ) {
    }

    bool is_ready() const override {
        return true;
    }

    InferenceResult infer(
        const InferenceRequest& request
    ) const override {
        static_cast<void>(request);

        throw std::runtime_error(
            "模拟推理失败"
        );
    }
};


/*测试用函数 */
void test_valid_request(
    TestRunner& runner
) {
    const InferenceRequest request(
        "meeting.wav",
        "What happend?"
    );
    runner.expect_equal(
        request.audio_path(),
        "meeting.wav",
        "valid request stores audio path"
    );
    runner.expect_equal(
        request.question(),
        "What happend?",
        "valid request stores question"
    );
}
void test_empty_audio_path(
    TestRunner& runner
) {
    bool received_expected_exception = false;
    try{
        const InferenceRequest request(
            "",
            "What happened?"
        );
        static_cast<void>(request);// 防止出现未使用参数的警告
    } catch(
        const std::invalid_argument& error
    ) {
        received_expected_exception = 
            std::string(error.what()).find(
                "音频路径"
            ) !=std::string::npos;
    } catch(...){
        received_expected_exception = false;
    }
    runner.expect_true(
        received_expected_exception,
        "empty audio path throws invalid_argument"
    );
}
void test_unavailable_backend(
    TestRunner& runner
) {
    auto backend = std::make_unique<UnavailableBackend>();
    const InferenceRequest request(
        "meeting.wav",
        "What happend?"
    );
    const InferenceService service(std::move(backend));
    bool received_expected_exception = false;
    try{
        const InferenceResult result = 
            service.run(
                request
            );
    } catch(const std::runtime_error& error){
        received_expected_exception = 
            std::string(error.what()).find(
                "未就绪"
            )  != std::string::npos;
    } catch(...){
        received_expected_exception = 
            false;
    }

    runner.expect_true(
        received_expected_exception,
        "unavailable backend is rejected"
    );
}
// 测试多态
void test_polymorphic_dispatch(
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
    const InferenceResult first = 
        run_through_interface(
            whisper,
            request
        );
    const InferenceResult second = 
        run_through_interface(
            qwen,
            request
        );
    runner.expect_equal(
        first.backend_name(),
        "Whisper",
        "interface dispatch Whisper"
    );
    runner.expect_equal(
        second.backend_name(),
        "Qwen2.5-Omni",
        "interface dispatch Qwen"
    );
}
// 测试静态调用计数
void test_whisper_count(
    TestRunner& runner
) {
    const int before =
        WhisperBackend::inference_count();
    const WhisperBackend backend(
        "Whisper"
    );
    const InferenceRequest request(
        "meeting.wav",
        "What happend?"
    );
    const InferenceResult result =
        backend.infer(request);
    const int after =
        WhisperBackend::inference_count();
    runner.expect_equal(
        after,
        before+1,
        "Whisper count increase by one"
    );
}
int main(){
    TestRunner runner;
    test_valid_request(runner);
    test_empty_audio_path(runner);
    test_unavailable_backend(runner);
    test_polymorphic_dispatch(runner);
    test_whisper_count(runner);
}