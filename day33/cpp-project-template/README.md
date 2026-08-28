# AudioCppTemplate

一个最小但完整的现代C++17项目模板，包含：

- 一个可复用库 `audio_core`；
- 一个可执行程序 `audio_demo`；
- GoogleTest单元测试；
- Debug和Release预设；
- 编译警告配置；
- clang-format配置；
- Git忽略规则。

## Debug

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
./build/debug/audio_demo
```

## Release

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release
./build/release/audio_demo
```

第一次配置测试时需要网络下载GoogleTest。
