#include"testRunner.hpp"
#include"request_result.hpp"
#include"inferenceService.hpp"
#include"string"
#include"whisper.hpp"
#include"qwen.hpp"
#include"memory"
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
void test_backend_destroyed_with_service(
    TestRunner& runner
) {
    const int alive_before =
        ModelBackend::alive_count();
    {
        auto backend = 
            std::make_unique<QwenOmniBackend>(
                "Qwen2.5-Omni"
            );
        InferenceService service(
            std::move(backend)
        );
        runner.expect_true(
            backend == nullptr,
            "service takes backend ownership"
        );
        runner.expect_equal(
            ModelBackend::alive_count(),
            alive_before + 1,
            "backend is alive inside service scope"
        );
    }
    runner.expect_equal(
        ModelBackend::alive_count(),
        alive_before,
        "backend is destroyed with serivce"
    );
}
void test_replace_destroys_old_backend(
    TestRunner& runner
) {
    const int destroyed_before =
        ModelBackend::destroyed_count();
    InferenceService service(
        std::make_unique<QwenOmniBackend>(
            "Qwen2.5-Omni"
        )
    );
    service.replace_backend(
        std::make_unique<WhisperBackend>(
            "Whisper"
        )
    );
    runner.expect_equal(
        ModelBackend::destroyed_count(),
        destroyed_before+1,
        "replacing backend destroys old backend"
    );
}
/*异常后端自动清理*/
void run_with_failure(){
    auto backend = 
        std::make_unique<QwenOmniBackend>(
            "Qwen2.5-Omni"
        );
    throw std::runtime_error(
        "simulated configuration failure"
    );
}
void test_failure_backend_auto_clean(
    TestRunner& runner
) {
    const int alive_before = 
        ModelBackend::alive_count();
    try{
        run_with_failure();
    } catch(const std::runtime_error& error){
    }
    runner.expect_equal(
        ModelBackend::alive_count(),
        alive_before,
        "exception path releases local backend"
    );
}
/*替换失败保持旧后端*/
/*后端工厂*/
std::unique_ptr<ModelBackend> create_backend(
    const std::string& type
) {
    if(type=="QwenOmniBackend")
        return std::make_unique<QwenOmniBackend>(
            "Qwen2.5-Omni"
        );
    else if(type=="WhisperBackend")
        return std::make_unique<WhisperBackend>(
            "Whisper"
        );
    else{
        throw std::invalid_argument(
            "invalid type: "+ type
        );
        return nullptr;
    }
}
void replace_error_keeping_old_backend(
    TestRunner& runner
) {
    const int alive_count = 
        ModelBackend::alive_count();
    auto backend =
        std::make_unique<QwenOmniBackend>(
            "Qwen2.5-Omni"
        );
    const InferenceRequest request(
        "meeting.wav",
        "What happend?"
    );
    InferenceService service(
        std::move(backend)
    );
    InferenceResult result = service.run(request);
    result.print();
    try{
        auto new_backend =  
            create_backend(
                "unknown"
            );
        service.replace_backend(
            std::move(new_backend)
        );
    } catch(const std::invalid_argument& error){
        std::cout << std::string(error.what());
    }
    runner.expect_true(
        service.run(request).backend_name().find("Qwen")!=std::string::npos,
        "keeping old backend"
    );
    runner.expect_equal(
        ModelBackend::alive_count(),
        alive_count+1,
        "alive count not change"
    );
}
int main(){
    TestRunner runner;
    test_valid_request(runner);
    test_empty_audio_path(runner);
    test_unavailable_backend(runner);
    test_polymorphic_dispatch(runner);
    test_whisper_count(runner);
    test_backend_destroyed_with_service(runner);
    test_replace_destroys_old_backend(runner);
    test_failure_backend_auto_clean(runner);
    replace_error_keeping_old_backend(runner);
}