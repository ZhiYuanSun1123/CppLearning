# 练习一：制造并识别一个不稳定测试

## 目的

观察一个测试“有时通过、有时失败”的现象。

## 操作

在 `tests/audio_task_queue_test.cpp` 末尾临时加入：

```cpp
#include <chrono>

TEST(AudioTaskQueueTest, BadTestDependsOnCurrentTime) {
    const auto now = std::chrono::steady_clock::now()
                         .time_since_epoch()
                         .count();
    EXPECT_EQ(now % 2, 0);
}
```

这个测试没有验证 `AudioTaskQueue` 的业务行为。它根据运行时钟的数值决定成功或失败，因此结果不稳定。

重新构建：

```bash
cmake --build --preset debug
```

重复运行最多 50 次：

```bash
bash scripts/repeat-tests.sh 50
```

观察是否出现失败，并记录失败测试的名称。

> 即使连续 50 次都通过，也不能证明它稳定。因为测试结果仍然依赖当前时刻。这正是“不稳定”的根源。

完成观察后删除这个错误测试。
