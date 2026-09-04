# 练习2：让格式检查失败并恢复

1. 在 `app/main.cpp` 临时制造明显不符合 `.clang-format` 的缩进。
2. 先运行 `bash scripts/check-format.sh`，确认非零退出码。
3. 推送实验分支并观察 Format check 失败，而构建测试可能仍通过。
4. 运行 `bash scripts/format.sh` 自动修复。
5. 检查 diff 后提交并推送，确认 Format check 恢复绿色。
6. 将失败与恢复的 run 链接写入学习报告。
