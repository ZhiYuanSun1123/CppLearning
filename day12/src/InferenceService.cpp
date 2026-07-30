#include"inferenceService.hpp"
void InferenceService::run(
    const ModelBackend& backend,
    const InferenceRequest& request
) const {
    if(!backend.is_ready())
        throw std::runtime_error(
            "后段未就绪: "
            + backend.name()
        );
    std::cout
        << "Select backend: "
        << backend.name()
        << std::endl;
    const InferenceResult result =
        backend.infer(request);
    result.print();
}