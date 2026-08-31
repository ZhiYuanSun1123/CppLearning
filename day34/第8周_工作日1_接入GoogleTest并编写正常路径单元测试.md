# 第8周工作日1：接入 GoogleTest 并编写正常路径单元测试

## 0. 今天学什么，先不学什么

今天不是再学一次 CMake，而是学习：**如何用测试自动判断自己的 C++ 代码是否符合约定。**

day33 已经出现过 GoogleTest。那时重点是把项目模板跑起来；今天重点是你能自己编写 `TEST` 和断言，并能看懂失败报告。

最终产出：一个有 7 个正常路径参考测试的音频元数据小项目，以及你亲自完成的 2 道综合测试练习。

本日新知识先说明：

| 新内容 | 今天学到什么程度 |
|---|---|
| 单元测试、测试套件、断言 | 能区分含义并亲手写测试 |
| `TEST`、`EXPECT_*`、`ASSERT_*` | 会使用，不研究宏展开源码 |
| `GTest::gtest_main` | 知道它提供测试入口，不必自己写测试 main |
| `gtest_discover_tests` | 知道如何把测试交给 CTest |
| `--gtest_filter`、XML 报告 | 能运行指定测试并保存结果 |
| `GTEST_SKIP` | 仅作为练习未完成标记，跳过不代表完成 |

暂不展开：`TEST_F` 测试夹具、参数化测试、GoogleMock、异常/死亡测试、覆盖率工具。它们不是今天写正常路径测试的前提。

已有知识会继续用：类、构造函数、`const`、`std::vector`、`std::size_t`、`std::move`、CMake target 与 Presets。今日不涉及硬件设计、音频采集或真实模型下载。

## 1. 先找到今天的文件

项目位置：

```text
/Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day34
├── CMakeLists.txt
├── CMakePresets.json
├── include/audio_toolkit/audio_clip.hpp
├── src/audio_clip.cpp
├── app/main.cpp
├── tests/audio_clip_test.cpp                 已完成的7个参考测试
├── exercises/01_metadata_normal_test.cpp    你完成的练习1
├── exercises/02_batch_normal_test.cpp       你完成的练习2
└── report/学习报告.md
```

本目录是独立项目，不会修改 day33。今天不用重新抄写业务类，先读懂它，再把时间花在测试上。

推荐阅读顺序：第 2～5 节概念 → 第 6 节运行 → 第 7 节构建接入 → 第 8～10 节设计和练习 → 第 11 节验收。

## 2. 什么是正常路径单元测试

### 2.1 单元测试

这里把 `AudioClip` 的一个公开行为作为测试对象，例如“传入合法元数据后，能返回正确时长”。输入小而确定，不依赖网络、显卡或真实文件。

普通演示：

```cpp
std::cout << clip.duration_seconds();
```

这只是输出数值，需要你用眼睛判断。

测试：

```cpp
EXPECT_DOUBLE_EQ(clip.duration_seconds(), 2.5);
```

这会自动比较结果，失败时报告位置和数值。测试命令可用退出码告诉脚本是否成功。

### 2.2 正常路径

指合法且有代表性的输入应产生约定结果，例如：

- 合法路径、正采样率、正声道数能够保存；
- 40000 帧、16000 Hz 的时长为 2.5 秒；
- 双声道片段的时长计算依然正确。

不是“只要没崩溃就通过”，而是**输出和可观察状态都要符合要求**。

空路径、负采样率属于非法输入；零帧属于合法边界值。本日主要练正常数据，之后再扩充边界和失败路径。

### 2.3 三个名字

```cpp
TEST(AudioClipTest, CalculatesFractionalDuration) {
    // 测试内容
}
```

| 部分 | 含义 |
|---|---|
| `AudioClipTest` | 测试套件名，把相关测试归为一组 |
| `CalculatesFractionalDuration` | 本次测试名，描述具体行为 |
| `AudioClipTest.CalculatesFractionalDuration` | 完整测试名，可用于筛选 |

`TEST` 是宏，看起来像函数调用，但这里用于定义并注册测试，不是让你在 main 里调用的普通函数。名称不用加引号；同一套件里不要重名。今天统一使用不含下划线的 PascalCase 名称。

一个 `TEST` 可以包含多个断言。7 个 `TEST` 才是 7 个测试，不是数 `EXPECT` 有多少个。

## 3. 读懂被测试的 AudioClip

它只有四项数据：路径、采样率、声道数、音频帧数。构造时验证参数，读取时不会修改对象。

```cpp
const audio_toolkit::AudioClip clip("speech.wav", 16000, 1, 40000);
```

这里的 `speech.wav` 是元数据，不会真的调用文件打开函数，所以你不需要准备这个文件。

**一帧音频包含同一时刻各声道的样本。** 双声道的 1 帧含两个样本，但不是两倍时长。

```text
时长（秒）= 帧数 / 采样率
40000 / 16000 = 2.5 秒
96000 / 48000 = 2 秒，无论这里是单声道还是双声道
```

不要再除一次声道数。只有当输入数值表示“所有声道合计样本数”而不是帧数时，换算才不同。

源码先把整数转为 `double` 再做除法。如果先进行整数除法，`40000 / 16000` 会得到 2，之后再转成浮点也无法恢复 0.5。

## 4. 一个测试分成三步

```cpp
TEST(AudioClipTest, CalculatesFractionalDuration) {
    // Arrange：准备输入和对象。
    const audio_toolkit::AudioClip clip("speech.wav", 16000, 1, 40000);

    // Act：执行被测试的操作。
    const double actual = clip.duration_seconds();

    // Assert：检查实际结果是否符合预期。
    EXPECT_DOUBLE_EQ(actual, 2.5);
}
```

这三个词经常简称 AAA。不是 C++ 语法，只是一种组织测试的方式，不要求每次都写三段注释。

`actual` 是实际计算结果，`2.5` 是你事先根据业务规则确定的预期结果。

不要这样写：

```cpp
const double expected = clip.duration_seconds();
EXPECT_DOUBLE_EQ(clip.duration_seconds(), expected);
```

它只是拿同一个实现和自己比较，通常发现不了计算公式错误。

## 5. 按用途学习断言

### 5.1 整数、字符串、布尔值

```cpp
EXPECT_EQ(clip.sample_rate(), 16000);
EXPECT_EQ(clip.path(), "speech.wav");
EXPECT_NE(first.path(), second.path());
EXPECT_TRUE(clip.channels() > 0);
EXPECT_FALSE(clip.path().empty());
```

- `EQ`：相等；`NE`：不相等。
- `TRUE`：条件应为真；`FALSE`：条件应为假。
- 失败时不会因为一次 `EXPECT_*` 就退出当前测试函数。
- 不必所有检查都写成 `EXPECT_TRUE(a == b)`，`EXPECT_EQ(a, b)` 的失败信息通常更直接。

`std::string` 可以用 `EXPECT_EQ` 比较内容。但是两个 `const char*` 用 `EXPECT_EQ` 比较的是指针值；需要比较 C 字符串内容时使用 `EXPECT_STREQ`。不要为了比较 `std::string` 而特意调用 `c_str()`。

### 5.2 浮点数

```cpp
EXPECT_DOUBLE_EQ(clip.duration_seconds(), 2.5);
EXPECT_NEAR(clip.duration_seconds(), expected, tolerance);
```

`EXPECT_DOUBLE_EQ` 使用 GoogleTest 提供的浮点近似比较，不是逐位完全相等。

`EXPECT_NEAR(actual, expected, tolerance)` 检查绝对差值不超过指定容差。例子中 1/3 无法被有限二进制浮点精确表示，因此采用 `1e-12` 的绝对容差；这是本例简单计算的选择，不是任何模型输出都该使用的阈值。

今天不要用 `EXPECT_EQ` 作为浮点结果比较的默认选择。

### 5.3 EXPECT 与 ASSERT：失败后能不能继续

```cpp
ASSERT_EQ(clips.size(), std::size_t{2});
EXPECT_EQ(clips[0].path(), "first.wav");
```

第一行是访问下标前的前置条件。若数量不对，继续访问可能越界，所以用 `ASSERT_EQ`。

| 宏 | 失败行为 | 何时使用 |
|---|---|---|
| `EXPECT_*` | 记录失败，继续当前函数 | 后续检查仍然安全且有价值 |
| `ASSERT_*` | 记录失败，返回当前函数 | 后续操作依赖这个前提 |

不是“ASSERT 终止整个进程”。如果写在测试函数中，它结束该测试函数，后面的其他测试仍可运行。如果写在辅助函数中，它只返回那个辅助函数，不能自动保证调用者也停止。

`ASSERT_*` 通常用在返回 `void` 的函数中，今天直接放在 `TEST` 内即可。正常作用域退出会析构局部 RAII 对象，但放在断言后面的手工清理代码可能不会执行。

### 5.4 为失败信息补充上下文

```cpp
EXPECT_EQ(clip.channels(), 2) << "正在检查 meeting.wav 的声道数";
```

这里 `<<` 是向断言结果追加文字，不是 `std::cout`。不要求每个断言都附加重复说明。

以上宏的详细行为可核对 [GoogleTest 断言参考](https://google.github.io/googletest/reference/assertions.html)。

## 6. 先跑参考测试

以下命令逐行执行。不用 VS Code 的“编译当前文件”任务：测试需要同时链接业务库和 GoogleTest。

```bash
cd /Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day34
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

首次配置需要从 GitHub 下载 GoogleTest。下载失败是依赖获取问题，不代表你的测试源码错误。

默认只运行 7 个参考测试。运行普通示例：

```bash
./build/debug/audio_demo
```

预期：`speech.wav: 2.5 s`。这只是演示应用，不算单元测试。

### 6.1 直接运行测试程序

```bash
./build/debug/audio_core_tests
./build/debug/audio_core_tests --gtest_list_tests
./build/debug/audio_core_tests --gtest_filter=AudioClipTest.CalculatesFractionalDuration
./build/debug/audio_core_tests --gtest_filter='AudioClipTest.*'
```

通配符用单引号包起来，避免被 zsh 当作文件名模式提前展开。

### 6.2 用 CTest 筛选

```bash
ctest --test-dir build/debug -N
ctest --test-dir build/debug -R 'CalculatesFractionalDuration' --output-on-failure
ctest --test-dir build/debug -V
```

- `-N`：列出已发现的测试，不运行。
- `-R`：按测试名的正则表达式筛选。
- `--output-on-failure`：失败时显示程序输出。
- `-V`：详细运行输出。

`ctest -R` 和 `--gtest_filter` 的模式规则不同；一个是 CTest 的正则表达式，一个是 GoogleTest 的筛选格式，不要混用。

### 6.3 保存报告、检查退出码

```bash
./build/debug/audio_core_tests --gtest_output=xml:report/reference-results.xml
echo $?
```

`echo $?` 必须紧跟想检查的命令。0 通常表示这次测试运行成功；非 0 表示失败。它不是失败测试的精确数量。

`[ RUN ]` 表示开始，`[ OK ]` 表示通过，`[ FAILED ]` 表示失败，`[ SKIPPED ]` 表示跳过。

注意：全部跳过也可能退出 0，甚至没有发现测试时也可能不表现为你期待的错误，所以还必须核对**测试数量和跳过数量**。

## 7. CMake 接入按六类理解

完整文件已经给出；不要再把它当成一团配置。下面只解释测试相关部分。

### 7.1 开启测试功能

```cmake
include(CTest)
```

引入 CTest 支持，并提供 `BUILD_TESTING` 开关；启用时会打开测试注册功能。

### 7.2 获取第三方源码

```cmake
include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(googletest)
```

`Declare` 描述从哪里取；`MakeAvailable` 获取并将依赖加入构建。沿用 day33 的固定版本 v1.17.0，而不是每次追踪主分支。固定版本有利于保持学习环境一致，不等于宣称它永远是最新版本。

### 7.3 创建测试可执行程序

```cmake
add_executable(audio_core_tests tests/audio_clip_test.cpp)
```

它与 `audio_demo` 是两个独立可执行程序。不要把 `app/main.cpp` 加到测试 target 里。

### 7.4 链接业务库和测试入口

```cmake
target_link_libraries(audio_core_tests PRIVATE audio_core GTest::gtest_main)
```

- `audio_core`：提供 `AudioClip` 的实现和公开 include 目录。
- `GTest::gtest_main`：提供 GoogleTest 及其默认 main。
- 依赖 target 会传递所需的头文件目录等使用要求，不需要手工猜测 `gtest.h` 的路径。

测试文件没有 main 不是遗漏。若你自己提供 main，通常应链接 `GTest::gtest` 而不是重复引入默认入口；今天不需要自己写。

### 7.5 发现和注册测试

```cmake
include(GoogleTest)
gtest_discover_tests(audio_core_tests DISCOVERY_TIMEOUT 30)
```

这里 `GoogleTest` 是 **CMake 自带的集成模块**，不是下载第三方库的动作。默认会在构建后运行测试程序列出测试，再注册给 CTest。

`DISCOVERY_TIMEOUT 30` 给“运行程序并列出测试”这一步 30 秒时间，不是允许每个测试运行 30 秒。此次本机验证曾出现默认 5 秒发现超时，因此显式放宽；这不涉及改变业务断言。

### 7.6 各工具分工

```text
CMake：描述业务库、测试程序和依赖
    ↓ 构建
GoogleTest 测试程序：执行 TEST 和断言，判断行为是否正确
    ↑ 调用
CTest：统一发现、筛选、运行测试并汇总结果
```

GoogleTest 不代替 CMake，CTest 也不代替 `EXPECT_EQ`。

官方接入示例：[GoogleTest CMake Quickstart](https://google.github.io/googletest/quickstart-cmake.html)。测试发现行为：[CMake GoogleTest 模块](https://cmake.org/cmake/help/latest/module/GoogleTest.html)。

## 8. 参考测试为什么这样设计

| 测试名 | 验证什么 | 能发现的典型问题 |
|---|---|---|
| StoresMonoMetadata | 四项元数据保存正确 | 构造赋值遗漏、访问器返回错字段 |
| CalculatesFractionalDuration | 2.5 秒时长 | 整数除法截断 |
| StereoDurationUsesFramesNotSamples | 双声道的时长 | 错误地再除声道数 |
| CalculatesNonTerminatingDuration | 1/3 秒时长 | 计算公式错误、浮点比较方法不当 |
| ReadingDurationDoesNotChangeMetadata | 读取不改变可观察数据 | 错误的状态修改 |
| DifferentObjectsKeepIndependentMetadata | 两对象保持独立 | 不应共享的数据被共享 |
| ReadsFirstClipAfterCheckingSize | 先验证容器再取元素 | 测试自身不安全的下标访问 |

这些用例只是当前样例集合，不是穷尽所有正确性证明。

每个测试独立构造自己的对象，不能要求 A 测试先运行、B 测试再使用 A 的结果。不要用全局可变变量保存跨测试状态。

单元测试通过也不证明没有内存错误：GoogleTest 检查你写下的行为断言，ASan 检查执行过程中可检测到的内存问题，两者互补。

## 9. 两道综合练习

不再拆成十几个小文件。业务实现已经提供，你重点写测试。

先开启练习 target：

```bash
cmake --preset practice
cmake --build --preset practice
ctest --preset practice
```

初始状态：7 个参考测试运行，2 个练习显示 `SKIPPED`。这是刻意设置的未完成标记，不是“9 个测试全部完成”。

### 练习1：合法元数据与时长综合测试

文件：`exercises/01_metadata_normal_test.cpp`。

测试名：`MetadataExercise.StoresStereoMetadataAndDuration`。

输入：`meeting.wav`、48000 Hz、2 声道、180000 帧。

要求：

1. 保留 TEST 名称，删除 `GTEST_SKIP()` 并实现测试。
2. 用 `EXPECT_EQ` 检查路径、采样率、声道数、帧数。
3. 用合适的浮点断言检查时长为 3.75 秒。
4. 再读取一次时长，检查结果相同且帧数未改变。
5. 至少为一个断言添加 `<<` 失败说明。

自问：为什么 3.75 秒不应该再除以 2？为什么这个题不需要真实 WAV 文件？

### 练习2：多个对象、容器和结果汇总

文件：`exercises/02_batch_normal_test.cpp`。

测试名：`BatchExercise.SumsDurationAndPreservesInput`。

构造一个 `std::vector<AudioClip>`，按下面顺序放入三个片段：

| 路径 | 采样率 | 声道数 | 帧数 | 预期时长 |
|---|---:|---:|---:|---:|
| a.wav | 16000 | 1 | 16000 | 1 秒 |
| b.wav | 48000 | 2 | 120000 | 2.5 秒 |
| c.wav | 16000 | 1 | 20000 | 1.25 秒 |

要求：

1. 删除 `GTEST_SKIP()`，先用 `ASSERT_EQ` 验证元素数量为 3。
2. 使用范围 for，调用每个片段的 `duration_seconds()`，累计得到总时长。
3. 使用浮点断言检查总时长为 4.75 秒；不能用同一个求和过程再计算一个“预期值”。
4. 检查三个元素的路径和帧数仍然正确。
5. 在学习报告里解释为什么访问下标之前用 ASSERT，而读取后比较数据用 EXPECT。

这里只检查多个对象的公开行为和汇总结果，**不代表实现了音频合并或真实解码**，也不能据此宣称覆盖真实音频系统。

只运行练习：

```bash
./build/practice/audio_exercise_tests
./build/practice/audio_exercise_tests --gtest_filter='MetadataExercise.*'
```

验收时这两个测试都必须是 `OK`，不能是 `SKIPPED`，不能只有空的 TEST 函数体。

## 10. 必做：亲眼观察一次失败

在你自己的练习1中，暂时把预期时长从 3.75 改为 4.0。

```bash
cmake --build --preset practice
./build/practice/audio_exercise_tests --gtest_filter='MetadataExercise.*'
echo $?
```

观察：

1. 失败报告给出的文件名和行号。
2. 实际值是 3.75，预期值是 4.0；具体文字排版可能因断言类型而异。
3. GoogleTest 返回非 0 退出码。
4. 这不是编译错误：程序编译成功了，但业务验证失败。

然后把预期值恢复为 3.75，**重新构建**再运行。只改文件不构建会运行旧二进制。

额外思考，不需要改源码：如果把业务公式改成整数除法，参考用例中的哪个测试会失败？如果只测试整秒，能否发现这个错误？

## 11. 今日验收和运行顺序

### 必做

- [ ] 能解释 TEST、测试套件和断言的区别。
- [ ] 能解释 EXPECT 与 ASSERT 的失败行为。
- [ ] Debug 的 7 个参考测试通过。
- [ ] 两道练习自己完成，均通过且没有 SKIPPED。
- [ ] 做过一次故意失败并恢复，记录文件、行号和退出码。
- [ ] 填写 `report/学习报告.md`，说明尚未覆盖的行为。

### 工程验证

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release
./build/practice/audio_exercise_tests --gtest_shuffle --gtest_repeat=5
./build/practice/audio_exercise_tests --gtest_output=xml:report/exercise-results.xml
```

`--gtest_shuffle` 改变测试运行顺序，`--gtest_repeat=5` 重复 5 次。它们有助于发现顺序依赖，但有限次数都通过仍不是绝对证明。

今天不要求测耗时和优化性能，也不要求学习所有 GoogleTest 宏。主线是“把输入、预期结果和验证写清楚”。

## 12. 常见问题，按阶段排查

| 现象 | 优先检查 |
|---|---|
| 配置时下载失败 | GitHub 网络连接；不要去改业务类 |
| 找不到 gtest/gtest.h | 是否通过 CMake 构建并链接了 GoogleTest target |
| Undefined symbols / 找不到 AudioClip 实现 | 测试是否链接 audio_core，而不是只编译当前 cpp |
| main 冲突 | 是否误把 app/main.cpp 加入测试 target |
| CTest 显示 No tests were found | BUILD_TESTING 是否开启、是否构建、build 路径是否正确 |
| 改完源码结果没变 | 是否重新 build，是否运行正确目录里的二进制 |
| zsh: no matches found | 含星号的筛选参数加单引号 |
| 练习被跳过 | 还留着 GTEST_SKIP；请完成断言后移除 |
| 构建成功但 IDE 爆红 | IDE 的编译数据库没有指向当前 build 下的 compile_commands.json |

IDE 爆红与真实编译失败不是同一个概念。若需要配置 VS Code，可将 C/C++ 插件的 compilation database 指向 `day34/build/debug/compile_commands.json`；今日不擅自修改你的全局 IDE 配置。

如果已经在本地有完整的同版本 GoogleTest 源码，可绕过重新下载。将下面路径替换为真实存在的源码目录，不要原样执行占位路径：

```bash
cmake --preset debug -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/真实路径/googletest
```

## 13. 复习问题

1. 为什么“打印 2.5”不等于“测试结果为 2.5”？
2. TEST 的两个名字分别是什么？一个测试能写多个断言吗？
3. ASSERT 失败是否会终止整个测试进程？
4. 为什么 clips[0] 之前要先检查数量？
5. 为什么浮点数比较需要考虑容差？
6. 为什么两个 const char* 不适合直接用 EXPECT_EQ 比较文本内容？
7. 测试源文件没有 main，为什么还能够生成可执行程序？
8. include(GoogleTest) 与 FetchContent 获取 googletest 是同一件事吗？
9. 构建通过、测试通过、ASan 无报告，各说明什么，又不能证明什么？
10. 如果所有测试都 SKIPPED 且退出码为 0，今天是否算完成？

## 14. 官方查阅资料

- [GoogleTest CMake 快速开始](https://google.github.io/googletest/quickstart-cmake.html)：核对接入方式。
- [GoogleTest Primer](https://google.github.io/googletest/primer.html)：测试及断言入门。
- [GoogleTest Assertions Reference](https://google.github.io/googletest/reference/assertions.html)：查询具体宏。
- [GoogleTest Advanced Guide](https://google.github.io/googletest/advanced.html)：需要时查筛选、重复运行和 XML 输出；不要今天整篇学完。
- [CMake GoogleTest 模块](https://cmake.org/cmake/help/latest/module/GoogleTest.html)：查询测试发现与注册。

学习建议：先运行参考测试，再独立写两道综合题，最后做故意失败实验。不要停在“把模板复制过来，看到绿色就结束”。
