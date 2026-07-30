#include"qwen.hpp"
QwenOmniBackend::QwenOmniBackend(
    const std::string& name
)
    : ModelBackend(name){}
bool QwenOmniBackend::is_ready() const {
    return true;
}
InferenceResult QwenOmniBackend::infer(
    const InferenceRequest& request
) const {
    ++inference_count_;
    const std::string output =
        "Reason about"
        + request.audio_path()
        + " and answer: "
        + request.question();
    return InferenceResult(
        name(),
        output
    );
}
int QwenOmniBackend::inference_count(){
    return inference_count_;
}