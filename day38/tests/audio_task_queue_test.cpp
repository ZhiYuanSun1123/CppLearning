#include "audio_test_report/audio_task_queue.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

using audio_test_report::AudioTaskQueue;

TEST(AudioTaskQueueTest, StartsEmpty) {
    const AudioTaskQueue queue;
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0U);
}

TEST(AudioTaskQueueTest, PopsHigherPriorityFirst) {
    AudioTaskQueue queue;
    queue.push("normal.wav", 1);
    queue.push("urgent.wav", 10);

    EXPECT_EQ(queue.pop_next().path, "urgent.wav");
    EXPECT_EQ(queue.pop_next().path, "normal.wav");
}

TEST(AudioTaskQueueTest, KeepsSubmissionOrderWhenPrioritiesAreEqual) {
    AudioTaskQueue queue;
    queue.push("first.wav", 5);
    queue.push("second.wav", 5);
    queue.push("third.wav", 5);

    EXPECT_EQ(queue.pop_next().path, "first.wav");
    EXPECT_EQ(queue.pop_next().path, "second.wav");
    EXPECT_EQ(queue.pop_next().path, "third.wav");
}

TEST(AudioTaskQueueTest, RejectsEmptyPath) {
    AudioTaskQueue queue;
    EXPECT_THROW(queue.push("", 1), std::invalid_argument);
}

TEST(AudioTaskQueueTest, RejectsPopFromEmptyQueue) {
    AudioTaskQueue queue;
    EXPECT_THROW(static_cast<void>(queue.pop_next()), std::runtime_error);
}
