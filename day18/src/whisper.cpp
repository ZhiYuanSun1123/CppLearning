#include"whisper.hpp"
WhisperBackend::WhisperBackend(
    const std::string& name
)
    : ModelBackend(name){}
bool WhisperBackend::is_ready() const{
    return true;
}
InferenceResult WhisperBackend::infer(
    const InferenceRequest& request
) const{
    ++inference_count_;

    const std::string output = 
        "Transcribe "
        + request.audio_path()
        + " for question "
        + request.question();

    return InferenceResult(
        name(),
        output
    );
}
int WhisperBackend::inference_count(){
    return inference_count_;
}