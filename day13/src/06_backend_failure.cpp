#include"inferenceService.hpp"
#include"request_result.hpp"
#include"model.hpp"
#include"testRunner.hpp"
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
void backend_failure(
    TestRunner& runner
) {
    const FailingBackend backend;
    const InferenceService service;
    const InferenceRequest request(
        "meeting.wav",
        "What happend?"
    );
    bool received_expect_exception = false;
    try{
        InferenceResult result =
            service.run(
                backend,
                request
            );
    } catch(std::runtime_error error){
        received_expect_exception =
            std::string(error.what()).find("推理失败")
                != std::string::npos;
    } catch(...){
        received_expect_exception = false;
    }
    runner.expect_true(
        received_expect_exception,
        "错误信息包含推理错误"
    );
}
int main(){
    TestRunner runner;
    backend_failure(runner);
    return runner.final();
}