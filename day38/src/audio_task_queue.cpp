#include "audio_test_report/audio_task_queue.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace audio_test_report {

void AudioTaskQueue::push(std::string path, int priority) {
    if (path.empty()) {
        throw std::invalid_argument("audio path must not be empty");
    }

    tasks_.push_back(
        AudioTask{std::move(path), priority, next_submission_order_++});
}

bool AudioTaskQueue::empty() const noexcept { return tasks_.empty(); }

std::size_t AudioTaskQueue::size() const noexcept { return tasks_.size(); }

AudioTask AudioTaskQueue::pop_next() {
    if (tasks_.empty()) {
        throw std::runtime_error("cannot pop from an empty audio task queue");
    }

    const auto next = std::min_element(
        tasks_.begin(), tasks_.end(), [](const AudioTask& left,
                                        const AudioTask& right) {
            if (left.priority != right.priority) {
                return left.priority > right.priority;
            }
            return left.submission_order < right.submission_order;
        });

    AudioTask result = std::move(*next);
    tasks_.erase(next);
    return result;
}

} // namespace audio_test_report
