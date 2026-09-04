#include "audio_test_report/audio_task_queue.hpp"

#include <iostream>

int main() {
    audio_test_report::AudioTaskQueue queue;
    queue.push("meeting.wav", 1);
    queue.push("emergency.wav", 10);
    queue.push("lecture.wav", 1);

    while (!queue.empty()) {
        const auto task = queue.pop_next();
        std::cout << task.path << " priority=" << task.priority << '\n';
    }
}
