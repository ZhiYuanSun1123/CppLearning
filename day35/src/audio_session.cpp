#include "audio_toolkit/audio_session.hpp"

#include <stdexcept>
#include <utility>

namespace audio_toolkit {

TrackedResource::TrackedResource(std::shared_ptr<ResourceState> state)
    : state_(std::move(state)) {
    if (state_ == nullptr) {
        throw std::invalid_argument("resource state must not be null");
    }
    ++state_->active_count;
    ++state_->acquire_count;
}

TrackedResource::~TrackedResource() noexcept {
    --state_->active_count;
    ++state_->release_count;
}

AudioSession::AudioSession(std::shared_ptr<ResourceState> state,
                           std::string model_name,
                           bool fail_after_acquire)
    : resource_(std::move(state)), model_name_(std::move(model_name)) {
    if (model_name_.empty()) {
        throw std::invalid_argument("model name must not be empty");
    }
    if (fail_after_acquire) {
        throw std::runtime_error("simulated model initialization failure");
    }
}

const std::string& AudioSession::model_name() const noexcept {
    return model_name_;
}

} // namespace audio_toolkit
