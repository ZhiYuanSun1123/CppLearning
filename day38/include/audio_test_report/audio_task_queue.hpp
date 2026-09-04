#ifndef DAY38_AUDIO_TEST_REPORT_AUDIO_TASK_QUEUE_HPP
#define DAY38_AUDIO_TEST_REPORT_AUDIO_TASK_QUEUE_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace audio_test_report {

struct AudioTask {
    std::string path;
    int priority;
    std::size_t submission_order;
};

class AudioTaskQueue {
  public:
    void push(std::string path, int priority);
    bool empty() const noexcept;
    std::size_t size() const noexcept;
    AudioTask pop_next();

  private:
    std::vector<AudioTask> tasks_;
    std::size_t next_submission_order_ = 0;
};

} // namespace audio_test_report

#endif
