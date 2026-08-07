#include"inferenceService.hpp"
InferenceService::InferenceService(
    std::unique_ptr<ModelBackend> backend
) : backend_(std::move(backend)){
    if(!backend_)
        throw std::invalid_argument(
            "不能传入空的后端"
        );
}
InferenceResult InferenceService::run(
    const InferenceRequest& request
) const {
    if(!backend_->is_ready())
        throw std::runtime_error(
            "后端未就绪: "
            + backend_->name()
        );
    return backend_->infer(request);
}
void InferenceService::replace_backend(
    std::unique_ptr<ModelBackend> backend
) {
    auto new_backend = std::move(backend);
    backend_ = std::move(new_backend);
}