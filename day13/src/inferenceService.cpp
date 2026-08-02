#include"inferenceService.hpp"
InferenceResult InferenceService::run(
    const ModelBackend& backend,
    const InferenceRequest& request
) const {
    if(!backend.is_ready())
        throw std::runtime_error(
            "后端未就绪: "
            + backend.name()
        );
    return backend.infer(request);
}