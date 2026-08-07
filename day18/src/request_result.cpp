#include"request_result.hpp"
InferenceRequest::InferenceRequest(
    const std::string& audio_path,
    const std::string& question
)
    : audio_path_(audio_path),
      question_(question){
    if(audio_path_.empty())
        throw std::invalid_argument("音频路径不能为空");
    if(question_.empty())
        throw std::invalid_argument("问题不能为空");
}
const std::string& InferenceRequest::audio_path() const {
    return audio_path_;
}
const std::string& InferenceRequest::question() const {
    return question_;
}
InferenceResult::InferenceResult(
    const std::string& backend_name,
    const std::string& output
)
    : backend_name_(backend_name),
      output_(output) {}
const std::string& InferenceResult::backend_name() const {
    return backend_name_;
}
const std::string& InferenceResult::output() const {
    return output_;
}
void InferenceResult::print() const {
    std::cout << "["
              << backend_name_
              << "] "
              << output_
              << std::endl;
}