# 练习二：把不稳定测试改成确定性测试

## 错误方向

不要通过下面的方法“修复”：

- 失败后自动重试直到通过；
- 增加 `sleep`；
- 把失败测试禁用；
- 降低断言要求。

这些方法只是隐藏问题。

## 正确方向

测试应使用固定输入并验证明确的业务规则。本项目已经提供：

```cpp
TEST(AudioTaskQueueTest, KeepsSubmissionOrderWhenPrioritiesAreEqual) {
    AudioTaskQueue queue;
    queue.push("first.wav", 5);
    queue.push("second.wav", 5);
    queue.push("third.wav", 5);

    EXPECT_EQ(queue.pop_next().path, "first.wav");
    EXPECT_EQ(queue.pop_next().path, "second.wav");
    EXPECT_EQ(queue.pop_next().path, "third.wav");
}
```

它具有：

- 固定输入；
- 固定业务规则；
- 固定期望结果；
- 不依赖时间、网络、随机数和运行顺序。

删除练习一的错误测试后，执行：

```bash
cmake --build --preset debug
bash scripts/repeat-tests.sh 50
```

预期所有测试连续运行 50 次都通过。
