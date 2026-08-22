# 第6周·工作日5：重构前五周代码并通过 `clang-tidy`基础检查

## 0. 今天真正要完成什么

今天不是继续堆新语法，也不是把前五周188个源文件全部改一遍。

今天的目标是建立第一次完整的工程质量闭环：

```text
选择代表性项目
    ↓
建立重构前基线
    ↓
配置clang-tidy基础规则
    ↓
阅读并分类诊断
    ↓
人工重构代码
    ↓
重新编译与测试
    ↓
再次运行clang-tidy
    ↓
使用Sanitizer做回归检查
    ↓
形成可展示的重构报告
```

完成后你应该能够：

1. 解释编译器警告、`clang-tidy`和Sanitizer的区别；
2. 在macOS上安装并定位 `clang-tidy`；
3. 为C++17项目编写基础 `.clang-tidy`配置；
4. 给单个翻译单元提供正确的编译参数；
5. 读懂诊断中的检查名称、文件、行号和修复建议；
6. 区分真实问题、设计建议和教学故意错误；
7. 在保持外部行为不变的前提下重构代码；
8. 使用警告、测试、ASan和UBSan验证重构；
9. 比较重构前后的诊断数量；
10. 输出一份可以放进GitHub仓库的工程质量报告。

---

## 1. 今天采用合并练习方式

按照你刚提出的要求，今天不再拆成八九个独立小程序，而是三个综合任务：

```text
综合任务1：盘点代码、建立基线并配置clang-tidy

综合任务2：集中重构三个代表性模块

综合任务3：统一编译、测试、静态检查和Sanitizer验证
```

每个任务同时覆盖多个知识点。

今天不会为了完成数量而重复创建很多 `main()`。

---

## 2. 今日目录结构

请建立：

```text
day28/
├── config/
│   └── .clang-tidy
├── scripts/
│   └── run_basic_tidy.sh
├── report/
│   ├── refactor_inventory.md
│   ├── clang-tidy-before.txt
│   ├── clang-tidy-after.txt
│   ├── compiler-warnings.txt
│   ├── sanitizer-output.txt
│   └── refactor_report.md
├── tests/
│   └── regression_checklist.md
└── 第6周_工作日5_重构前五周代码并通过clang-tidy基础检查.md
```

今天重构的是已有项目，不需要再复制一套相同的 `.cpp`文件到 `day28`。

推荐在Git中先创建独立分支或至少先提交当前版本：

```bash
git status
git add .
git commit -m "Save baseline before clang-tidy refactor"
```

如果当前目录还没有Git仓库，也可以先备份准备修改的文件，但不要用自动脚本覆盖原代码。

---

## 3. 建议时间

```text
理解工具和安装：45分钟
选择目标并建立基线：45分钟
第一次clang-tidy检查：45分钟
集中重构：2.5小时
编译、测试和Sanitizer：1.5小时
第二次检查和报告：45分钟
```

今天的完成标准不是“零诊断”，而是：

> 选定的正常项目通过约定的基础规则，剩余诊断都有明确解释。

---

## 4. 为什么不能检查并强改全部前五周代码

你的前五周目录中包含不同性质的文件。

### 正常功能代码

例如：

```text
day13：模型接口、异常和测试
day17：智能指针重构后的推理服务
day23：音频元数据索引工具
```

这些适合做质量检查和重构。

### 故意错误示例

例如 `day6`中包含：

```text
heap-buffer-overflow
stack-buffer-overflow
use-after-free
double-free
alloc-dealloc-mismatch
```

这些代码的目的就是触发ASan。

如果把它们改到“零错误”，就破坏了教学用途。

### 独立语法小练习

前几周还有很多一次性小程序。它们可以运行检查，但今天没必要逐个重构，因为同类问题会重复出现。

所以今天使用代表性抽样：

```text
一个多态接口项目
一个资源所有权项目
一个STL数据处理项目
```

---

## 5. 今天选择的三个重构目标

### 目标A：模型接口与错误处理

建议检查：

```text
day13/include/
day13/src/inferenceService.cpp
day13/src/model.cpp
day13/src/qwen.cpp
day13/src/request_result.cpp
day13/src/whisper.cpp
day13/src/test.cpp
day13/src/testRunner.cpp
```

覆盖知识：

- 抽象类和运行时多态；
- 虚析构函数；
- `override`；
- 构造函数；
- const-correctness；
- 异常处理；
- 多文件编译；
- 单元测试。

### 目标B：智能指针与独占后端

建议检查：

```text
day17/include/
day17/src/inferenceService.cpp
day17/src/model.cpp
day17/src/qwen.cpp
day17/src/request_result.cpp
day17/src/whisper.cpp
day17/src/test.cpp
day17/src/testRunner.cpp
```

覆盖知识：

- `unique_ptr<Base>`；
- 所有权表达；
- 禁止不必要复制；
- `std::move`；
- RAII；
- 空指针检查；
- 多态后端生命周期。

### 目标C：音频元数据索引

建议检查：

```text
day23/include/audio_metadata.hpp
day23/include/audio_metadata_index.hpp
day23/include/index_statistics.hpp
day23/src/audio_metadata_index.cpp
```

覆盖知识：

- `vector`和 `unordered_map`；
- 迭代器；
- lambda；
- `algorithm`；
- 避免不必要复制；
- 返回值与引用；
- 容器查找；
- const-correctness。

如果某个项目当前还不能正常编译，先选择另外两个正常项目完成流程。`clang-tidy`不能替代修复基础编译错误。

---

## 6. 什么是重构

重构是：

> 在不改变程序外部可观察行为的前提下，改善代码内部结构。

例如，重构前：

```cpp
ModelBackend* backend = nullptr;
backend = new QwenBackend("qwen");

// 使用backend

delete backend;
```

重构后：

```cpp
auto backend =
    std::make_unique<QwenBackend>("qwen");
```

程序的业务行为不变，但所有权更清楚，异常路径也不会泄漏。

不属于纯重构的修改：

- 改变输出答案；
- 更改错误码语义；
- 删除原有功能；
- 改变输入格式；
- 用不同算法改变结果；
- 为了消除告警随意吞掉异常。

所以重构前必须建立行为基线。

---

## 7. 什么是行为基线

行为基线用于回答：

```text
修改前程序能不能编译？
现有测试是否通过？
正常输入输出是什么？
错误输入输出是什么？
Sanitizer是否已经报告问题？
```

如果修改前没有记录，重构后就无法证明没有破坏功能。

在 `report/refactor_inventory.md`中记录：

```markdown
# 前五周代码重构盘点

## 目标A：day13模型接口

- 当前是否编译：
- 当前测试结果：
- 正常输出摘要：
- 已知问题：
- 是否纳入重构：

## 目标B：day17智能指针推理服务

- 当前是否编译：
- 当前测试结果：
- 正常输出摘要：
- 已知问题：
- 是否纳入重构：

## 目标C：day23音频元数据索引

- 当前是否编译：
- 当前测试结果：
- 正常输出摘要：
- 已知问题：
- 是否纳入重构：

## 明确排除

- day6故意错误案例：排除原因是用于Sanitizer教学
- 不能独立编译的片段：
- 尚未完成的练习：
```

---

## 8. 编译器警告、clang-tidy和Sanitizer的区别

### 编译器警告

例如：

```bash
-Wall -Wextra -Wpedantic
```

主要发生在编译阶段，检查：

- 未使用变量；
- 未使用参数；
- 可疑类型转换；
- 部分控制流问题；
- 非标准语法。

### `clang-tidy`

它是静态分析和代码质量工具，不运行程序，主要检查：

- 潜在bug；
- 不必要复制；
- 现代C++改进；
- 可读性；
- 一些资源和生命周期问题；
- Clang静态分析器能够推导的问题。

### Sanitizer

ASan和UBSan需要运行程序，检查实际执行路径中的：

- 越界；
- use-after-free；
- 重复释放；
- 部分未定义行为。

关系：

```text
编译器警告
    检查基础编译期问题

clang-tidy
    深入分析源码和编码模式

ASan/UBSan
    运行时检查实际执行路径

单元测试
    检查业务结果是否符合预期
```

它们互相补充，不能彼此替代。

---

## 9. 当前Mac上的 `clang-tidy`

你当前可以使用Apple自带的：

```text
/usr/bin/clang++
```

但Apple Command Line Tools通常不单独提供可直接调用的 `clang-tidy`。

当前环境检查没有发现 `clang-tidy`命令，因此需要安装Homebrew LLVM：

```bash
brew install llvm
```

Apple Silicon Mac通常位于：

```text
/opt/homebrew/opt/llvm/bin/clang-tidy
```

验证：

```bash
/opt/homebrew/opt/llvm/bin/clang-tidy \
  --version
```

如果希望当前终端直接输入 `clang-tidy`：

```bash
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
```

再次验证：

```bash
command -v clang-tidy
clang-tidy --version
```

不要把安装路径写死到可提交给所有人的项目代码中。脚本可以允许通过环境变量覆盖：

```bash
TIDY_BIN="${TIDY_BIN:-clang-tidy}"
```

如果终端仍找不到，再显式运行完整路径。

---

## 10. `clang-tidy`最基本的命令结构

```bash
clang-tidy 源文件 [clang-tidy选项] -- [编译器参数]
```

例如：

```bash
clang-tidy \
  day23/src/audio_metadata_index.cpp \
  -- \
  -std=c++17 \
  -I day23/include
```

`--`前面是 `clang-tidy`自己的选项。

`--`后面是编译这个源文件所需要的参数：

```text
-std=c++17
-I day23/include
```

如果缺少头文件路径，可能出现：

```text
file not found
```

这不是代码质量问题，而是没有给工具正确的编译环境。

---

## 11. 什么是翻译单元

`clang-tidy`通常以一个 `.cpp`文件作为入口分析翻译单元。

翻译单元大致包括：

```text
一个.cpp文件
    +
它#include进来的头文件
    +
宏展开后的内容
```

它不会执行链接步骤。

因此：

```bash
clang-tidy day23/src/audio_metadata_index.cpp -- ...
```

可以分析这个源文件及其包含的项目头文件，但不会帮你生成最终可执行文件。

所以即使 `clang-tidy`没有报链接错误，也仍然需要用 `clang++`完整编译项目。

---

## 12. 建立基础 `.clang-tidy`配置

在 `day28/config/.clang-tidy`中写：

```yaml
---
Checks: >
  -*,
  clang-analyzer-*,
  bugprone-use-after-move,
  bugprone-sizeof-expression,
  bugprone-unused-return-value,
  performance-for-range-copy,
  performance-unnecessary-copy-initialization,
  modernize-use-nullptr,
  modernize-use-override,
  modernize-make-unique
WarningsAsErrors: ''
HeaderFilterRegex: '.*/CppLearning/(day13|day17|day23)/.*'
FormatStyle: none
...
```

这是一组学习阶段的基础规则，不是所有企业的统一标准。

### 为什么先关闭所有规则

```yaml
Checks: -*
```

表示先关闭默认检查，再明确启用需要的规则。

这样可以避免第一次运行出现数百条风格告警，使你无法分辨重点。

### 为什么先启用分析器

```yaml
clang-analyzer-*
```

用于发现一部分：

- 空指针路径；
- 资源泄漏路径；
- 未初始化值；
- 可疑控制流。

### 为什么不把警告当错误

```yaml
WarningsAsErrors: ''
```

今天处于学习和盘点阶段。先理解诊断，再决定哪些规则作为项目门禁。

以后稳定后，可以只把关键检查升级为错误。

---

## 13. 基础规则分别在检查什么

### `clang-analyzer-*`

Clang静态分析器规则集合，尝试沿着控制流推导潜在错误。

### `bugprone-use-after-move`

检查对象移动后又被不恰当地使用：

```cpp
std::string target = std::move(source);
std::cout << source; // 状态有效但内容未指定，通常值得检查
```

注意：被移动对象仍然必须是可析构、可重新赋值的有效对象，但业务代码不能继续假设它保留原值。

### `bugprone-sizeof-expression`

检查可疑的 `sizeof`表达式，例如错误地把指针大小当数组大小。

### `bugprone-unused-return-value`

检查某些不应忽略的返回结果。

### `performance-for-range-copy`

检查范围for循环中不必要的元素复制：

```cpp
for (std::string value : values) {
    // 每次复制string
}
```

可能改为：

```cpp
for (const std::string& value : values) {
    // 只读借用，不复制
}
```

但对于 `int`、枚举等小类型，按值通常更清楚，不要机械改成引用。

### `performance-unnecessary-copy-initialization`

检查可以通过引用避免的对象复制。

### `modernize-use-nullptr`

把表示空指针的：

```cpp
NULL
0
```

改为类型更安全的：

```cpp
nullptr
```

### `modernize-use-override`

派生类重写虚函数时显式写：

```cpp
void infer() const override;
```

这样签名不一致时由编译器直接发现。

### `modernize-make-unique`

把适合的：

```cpp
std::unique_ptr<Model> model(
    new Model("qwen")
);
```

改为：

```cpp
auto model =
    std::make_unique<Model>("qwen");
```

---

## 14. 规则是否存在与版本有关

不同LLVM版本提供的检查可能不同。

查询当前版本全部检查：

```bash
clang-tidy \
  -list-checks \
  -checks='*'
```

查询指定规则：

```bash
clang-tidy \
  -list-checks \
  -checks='bugprone-use-after-move'
```

如果配置中某项在当前版本不存在，应当：

1. 确认是否拼写错误；
2. 确认LLVM版本；
3. 从配置中暂时移除不存在的规则；
4. 在报告中记录版本差异。

不要为了凑“零错误”而删除所有检查。

---

## 15. 如何读取一条诊断

示例：

```text
day17/src/inferenceService.cpp:18:9:
warning: use nullptr [modernize-use-nullptr]
```

结构：

```text
文件：day17/src/inferenceService.cpp
行：18
列：9
级别：warning
说明：use nullptr
检查名称：modernize-use-nullptr
```

最重要的是最后的检查名：

```text
[modernize-use-nullptr]
```

它告诉你是哪条规则提出建议，可以进一步查询文档和判断是否适用于当前代码。

---

## 16. 诊断应该如何分类

不要看到warning就立即自动修改。先放入以下类别。

### A类：真实正确性风险

例如：

- 可能空指针解引用；
- use-after-move；
- 未初始化读取；
- 所有权泄漏；
- 可疑 `sizeof`。

优先处理。

### B类：明确的现代化改进

例如：

- `NULL`改为 `nullptr`；
- 重写函数添加 `override`；
- 裸 `new`改为 `make_unique`。

确认行为不变后处理。

### C类：性能建议

例如：

- 范围for中复制大对象；
- 不必要的局部副本。

需要确认生命周期和可读性，不能机械改。

### D类：教学故意代码

例如：

- day6的悬空指针案例；
- 故意展示浅拷贝问题的旧练习。

不修复，记录排除原因。

### E类：误报或不适合当前设计

静态分析不是绝对正确。如果拒绝修复，必须在报告中写清依据，不能只写“我不想改”。

---

## 17. 第一次运行：只检查，不自动修改

以 `day23`为例：

```bash
TIDY=/opt/homebrew/opt/llvm/bin/clang-tidy

"$TIDY" \
  day23/src/audio_metadata_index.cpp \
  --config-file=day28/config/.clang-tidy \
  -- \
  -std=c++17 \
  -I day23/include
```

保存输出：

```bash
"$TIDY" \
  day23/src/audio_metadata_index.cpp \
  --config-file=day28/config/.clang-tidy \
  -- \
  -std=c++17 \
  -I day23/include \
  > day28/report/clang-tidy-before.txt \
  2>&1
```

注意：

```text
> 只保存标准输出
2>&1 把标准错误也合并进同一个文件
```

今天第一次运行禁止直接加入：

```bash
--fix
```

因为自动修改可能：

- 影响大量文件；
- 改变你尚未理解的代码；
- 产生不符合当前设计的修改；
- 让你无法学习诊断原因。

---

## 18. 头文件为什么可能重复报告

一个头文件可能被多个 `.cpp`包含。

例如：

```text
model.hpp
    被qwen.cpp包含
    被whisper.cpp包含
    被inferenceService.cpp包含
```

同一个头文件诊断可能在多个翻译单元中重复出现。

因此报告中的“诊断行数”不等于“独立问题数量”。

统计时至少区分：

```text
原始诊断数量
去重后的文件+行号+规则数量
```

今天不要求写复杂去重工具，但报告中不能把重复诊断当成多个不同bug夸大结果。

---

## 19. 为什么 `--fix`不能盲用

有些检查支持自动修复，但工具不知道你的全部业务意图。

例如把复制改为引用：

```cpp
const std::string value = map.at(key);
```

工具可能建议：

```cpp
const std::string& value = map.at(key);
```

如果引用只在容器不发生修改的短作用域内使用，通常安全。

但如果后续容器重新哈希、删除元素，长期保存的引用可能失效。

所以正确流程是：

```text
阅读建议
    ↓
理解对象所有权和生命周期
    ↓
判断是否适用
    ↓
人工修改
    ↓
测试验证
```

---

## 20. 目标A的重点重构项

在 `day13`模型接口项目中检查：

### 虚析构函数

作为多态基类：

```cpp
class ModelBackend {
public:
    virtual ~ModelBackend() = default;
};
```

### `override`

派生类：

```cpp
InferenceResult infer(
    const InferenceRequest& request
) const override;
```

### const-correctness

只读函数：

```cpp
const std::string& name() const noexcept;
```

但不要为了消除建议，错误地把所有函数都设为 `const`。

### 错误信息和异常边界

检查：

- 是否捕获后完全忽略异常；
- 是否通过错误消息文字判断错误类型；
- 是否返回悬空引用；
- 测试是否覆盖正常与失败路径。

### include完整性

每个头文件应直接包含自己需要的标准头，而不是依赖其他头文件间接包含。

---

## 21. 目标B的重点重构项

在 `day17`智能指针项目中检查：

### 独占所有权

服务独占一个多态后端时：

```cpp
std::unique_ptr<ModelBackend> backend_;
```

构造函数接收：

```cpp
explicit InferenceService(
    std::unique_ptr<ModelBackend> backend
);
```

初始化：

```cpp
InferenceService::InferenceService(
    std::unique_ptr<ModelBackend> backend
)
    : backend_(std::move(backend)) {
    if (!backend_) {
        throw std::invalid_argument(
            "backend cannot be null"
        );
    }
}
```

同时复习：

- 为什么按值接收便于表达所有权转移；
- 为什么需要 `std::move`；
- 移动后调用方的 `unique_ptr`会变空；
- 为什么服务不需要手写 `delete`。

### 工厂函数

检查是否可以使用：

```cpp
std::make_unique<QwenBackend>(...)
```

代替裸 `new`。

### 非拥有访问

只读借用后端时优先：

```cpp
const ModelBackend&
```

只有“允许没有后端”时才考虑指针。

不要创建 `shared_ptr`来掩盖所有权设计不清的问题。

---

## 22. 目标C的重点重构项

在 `day23`音频元数据索引中检查：

### 范围for复制

大对象只读遍历：

```cpp
for (const AudioMetadata& metadata
     : metadata_list) {
    // 只读使用
}
```

小型标量按值：

```cpp
for (int score : scores) {
}
```

不要把所有循环变量都机械改成引用。

### 查找结果生命周期

```cpp
const auto iterator = index.find(path);

if (iterator == index.end()) {
    // 未找到
}
```

如果保存：

```cpp
const AudioMetadata* pointer =
    &iterator->second;
```

必须确认后续不会删除该元素或触发使引用失效的容器操作。

### 不必要副本

如果函数只读取参数：

```cpp
void add(const AudioMetadata& metadata);
```

但如果函数需要把对象长期保存到容器中，按值接收再移动也可能更合理：

```cpp
void add(AudioMetadata metadata) {
    entries_.push_back(
        std::move(metadata)
    );
}
```

不能仅根据“复制越少越好”判断接口优劣。

### `contains`版本提醒

```cpp
unordered_map.contains(key)
```

是C++20接口。你的项目当前使用C++17，因此继续使用：

```cpp
map.find(key) != map.end()
```

不要为了现代化检查误用超出标准版本的接口。

---

## 23. `NOLINT`应该谨慎使用

如果确认某条诊断是有依据的例外，可以写：

```cpp
legacy_call(); // NOLINT(check-name)
```

或者：

```cpp
// NOLINTNEXTLINE(check-name)
legacy_call();
```

但今天的要求是：

- 必须写具体检查名称；
- 必须在旁边解释原因；
- 不允许使用没有范围的 `NOLINT`掩盖所有规则；
- 不允许为了得到“零告警”批量添加抑制；
- 教学错误示例应通过排除目录保留，而不是逐行NOLINT。

---

## 24. 先修编译错误，再讨论代码质量

正确顺序：

```text
1. 能预处理并找到头文件
2. 能编译每个翻译单元
3. 能成功链接
4. 测试能运行
5. 再处理clang-tidy建议
```

如果出现：

```text
undefined symbols for architecture arm64
```

那是链接阶段缺少实现文件或签名不一致，不是 `clang-tidy`替你解决的问题。

如果出现：

```text
file not found
```

先修复 `-I`头文件路径。

工具必须获得和真实编译相同的语言标准、宏和包含路径，否则分析结果不可靠。

---

## 25. 编译数据库是什么

今天可以直接在 `--`后传编译参数，但真实项目通常使用：

```text
compile_commands.json
```

它记录每个 `.cpp`实际使用的：

- 编译器；
- C++标准；
- include路径；
- 宏定义；
- 其他编译选项。

CMake可以生成：

```bash
cmake \
  -S . \
  -B build \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

然后：

```bash
clang-tidy \
  -p build \
  path/to/source.cpp
```

今天只要求认识。当前学习目录还不是统一的CMake工程，不要为了生成数据库先重构全部目录。

后续CMake阶段会系统使用它。

---

## 26. 脚本的职责

在 `day28/scripts/run_basic_tidy.sh`中集中保存重复命令。

脚本应该：

1. 开启失败即停止；
2. 接受可覆盖的 `TIDY_BIN`；
3. 验证工具存在；
4. 依次检查选定翻译单元；
5. 为不同项目提供正确 `-I`路径；
6. 不使用 `--fix`；
7. 返回真实退出码。

示例骨架：

```bash
#!/usr/bin/env bash

set -euo pipefail

TIDY_BIN="${TIDY_BIN:-clang-tidy}"
CONFIG_FILE="day28/config/.clang-tidy"

if ! command -v "$TIDY_BIN" >/dev/null 2>&1; then
    echo "clang-tidy not found: $TIDY_BIN" >&2
    exit 1
fi

"$TIDY_BIN" \
    day23/src/audio_metadata_index.cpp \
    --config-file="$CONFIG_FILE" \
    -- \
    -std=c++17 \
    -I day23/include
```

扩展到其他源文件时，每个翻译单元都要使用对应项目的include目录。

注意：Shell脚本属于今天的辅助工具，只要求能读懂和修改，不把Shell作为新的学习主线。

---

## 27. 综合任务1：盘点、基线与工具配置

对应产出：

```text
day28/config/.clang-tidy
day28/scripts/run_basic_tidy.sh
day28/report/refactor_inventory.md
day28/report/clang-tidy-before.txt
```

任务：

1. 检查并安装 `clang-tidy`；
2. 记录LLVM版本；
3. 从A、B、C三个目标中至少选择两个；
4. 对选择的项目先完成正常编译；
5. 运行现有测试或主程序；
6. 保存重构前输出；
7. 创建基础配置；
8. 第一次只检查，不自动修复；
9. 保存完整诊断；
10. 按A～E五类给诊断分类；
11. 明确记录排除的教学错误代码；
12. 写出准备修正的具体文件和行号。

本任务同时练习：

- 工具安装；
- 命令行参数；
- 翻译单元；
- include路径；
- 静态分析；
- Git基线；
- 问题分类。

验收：

```text
至少获得一份真实的重构前clang-tidy报告
并且每条准备修复的问题都有规则名称
```

---

## 28. 综合任务2：集中重构代表性模块

对应修改：

```text
从day13、day17、day23中选定的已有代码
```

至少处理以下五类中的三类：

### 类型与接口

- 多态重写添加 `override`；
- 多态基类具有虚析构函数；
- 只读成员函数补充正确的 `const`；
- 空指针使用 `nullptr`；
- 不混用不同业务含义的裸整数。

### 所有权与生命周期

- 独占后端使用 `unique_ptr`；
- 适合时使用 `make_unique`；
- 移动后不依赖源对象原值；
- 删除手写 `delete`；
- 非拥有关系使用引用或明确的观察指针。

### STL与性能

- 大对象只读遍历避免不必要复制；
- 不长期保存可能失效的迭代器、指针或引用；
- 查找只执行一次；
- 不为小型标量滥用引用；
- 需要保存参数时明确复制或移动策略。

### 错误处理

- 不忽略重要返回结果；
- 不吞掉异常；
- 不通过错误字符串判断错误类型；
- 测试正常路径与失败路径；
- 错误信息包含必要上下文。

### 头文件与构建

- 头文件自给自足；
- 声明和定义签名一致；
- include guard完整；
- 不在头文件使用 `using namespace std;`；
- 每个源文件使用正确include路径。

每次只处理一组相关诊断，然后立即：

```text
编译
运行测试
查看差异
```

不要积累几十处修改后才第一次编译。

---

## 29. 综合任务3：统一验证与报告

对应产出：

```text
day28/report/clang-tidy-after.txt
day28/report/compiler-warnings.txt
day28/report/sanitizer-output.txt
day28/report/refactor_report.md
day28/tests/regression_checklist.md
```

验证顺序：

### 第一步：重新编译

必须启用：

```bash
-std=c++17
-Wall
-Wextra
-Wpedantic
```

### 第二步：运行已有测试

记录：

- 通过数量；
- 失败数量；
- 退出码；
- 与基线输出是否一致。

### 第三步：再次运行 `clang-tidy`

使用和第一次完全相同的：

- LLVM版本；
- 配置；
- 源文件集合；
- include路径；
- C++标准。

否则前后数量不能公平比较。

### 第四步：ASan和UBSan

重新编译正常项目：

```bash
-fsanitize=address,undefined
-fno-omit-frame-pointer
-g
-O0
```

运行测试，保存输出。

### 第五步：检查Git差异

```bash
git diff --check
git diff
```

逐个确认修改与诊断对应，没有混入无关改动。

---

## 30. 编译示例：day23模块

`day23`当前只有模块实现，不一定自带可独立运行的 `main()`。静态检查不要求链接可执行文件：

```bash
/opt/homebrew/opt/llvm/bin/clang-tidy \
  day23/src/audio_metadata_index.cpp \
  --config-file=day28/config/.clang-tidy \
  -- \
  -std=c++17 \
  -I day23/include
```

只做编译检查：

```bash
clang++ \
  -std=c++17 \
  -Wall \
  -Wextra \
  -Wpedantic \
  -I day23/include \
  -c day23/src/audio_metadata_index.cpp \
  -o day28/report/audio_metadata_index.o
```

`-c`表示只编译为目标文件，不进行链接。

验证后可以删除报告目录中的临时 `.o`文件，不需要提交二进制产物。

---

## 31. 多文件项目检查方法

以 `day17`为例，先对每个实现文件运行 `clang-tidy`：

```bash
for source in day17/src/*.cpp; do
    /opt/homebrew/opt/llvm/bin/clang-tidy \
        "$source" \
        --config-file=day28/config/.clang-tidy \
        -- \
        -std=c++17 \
        -I day17/include
done
```

这只负责静态检查。

然后仍然需要使用原项目的完整编译命令进行链接。

不要把多个 `.cpp`同时写在一个 `clang-tidy`命令的源文件位置，最简单的方式是逐个翻译单元检查。

---

## 32. 如何统计前后结果

可以先粗略统计带检查名称的诊断：

```bash
grep -c '\[.*\]' \
  day28/report/clang-tidy-before.txt

grep -c '\[.*\]' \
  day28/report/clang-tidy-after.txt
```

但它只是粗略数量。

报告中更应该使用表格：

```markdown
|检查规则|修改前|修改后|处理方式|
|---|---:|---:|---|
|modernize-use-nullptr|3|0|改为nullptr|
|modernize-use-override|2|0|补充override|
|performance-for-range-copy|4|1|3处改引用，1处保留复制|
|clang-analyzer-*|1|0|修复空指针路径|
```

保留诊断时要说明：

```text
保留原因
是否误报
是否属于教学案例
为什么当前设计需要复制
是否计划后续处理
```

---

## 33. 回归检查清单

在 `day28/tests/regression_checklist.md`中写：

```markdown
# 重构回归检查

## day13模型接口

- [ ] 所有源文件编译
- [ ] 基类析构正确
- [ ] Qwen后端正常推理
- [ ] Whisper后端正常推理
- [ ] 空路径错误仍被拒绝
- [ ] 空问题错误仍被拒绝
- [ ] 原有测试通过

## day17所有权重构

- [ ] 服务成功接管unique_ptr
- [ ] 调用方指针移动后为空
- [ ] 空后端被拒绝
- [ ] 后端仅析构一次
- [ ] 无裸delete
- [ ] ASan无泄漏和重复释放

## day23元数据索引

- [ ] 添加元数据行为不变
- [ ] 路径查询行为不变
- [ ] 重复项处理行为不变
- [ ] 统计结果不变
- [ ] 不保存失效迭代器或引用
- [ ] UBSan无错误
```

根据你实际选中的目标删除未执行部分，不能把没有运行的项目勾选为通过。

---

## 34. 重构报告模板

在 `day28/report/refactor_report.md`中写：

```markdown
# 前五周C++代码重构与clang-tidy报告

## 1. 环境

- 操作系统：
- 编译器版本：
- clang-tidy版本：
- C++标准：C++17
- 检查配置：day28/config/.clang-tidy

## 2. 检查范围

- 纳入文件：
- 排除文件：
- 排除原因：

## 3. 重构前基线

- 编译结果：
- 测试结果：
- clang-tidy原始诊断数：
- 去重问题数：
- Sanitizer结果：

## 4. 问题分类

### 正确性风险

### 所有权与生命周期

### 性能问题

### 现代C++改进

### 保留或排除的诊断

## 5. 具体修改

|文件和位置|检查规则|原问题|修改方法|为什么安全|
|---|---|---|---|---|

## 6. 重构后验证

- 编译器警告：
- 测试：
- clang-tidy：
- ASan：
- UBSan：
- 退出码：

## 7. 前后对比

|检查规则|修改前|修改后|备注|
|---|---:|---:|---|

## 8. 未解决问题

- 问题：
- 保留原因：
- 后续计划：

## 9. 本次复习到的知识

- RAII：
- 智能指针：
- 多态：
- const：
- STL：
- 移动语义：
- 错误处理：
```

---

## 35. 常见误区

### 误区1：零告警等于零bug

静态分析只能覆盖部分问题。

### 误区2：警告越少，代码一定越好

批量NOLINT也能让数字变小，但质量没有提高。

### 误区3：所有建议必须接受

工具不知道完整业务语义，需要人工判断。

### 误区4：`--fix`一定安全

自动修复仍然需要审查和测试。

### 误区5：clang-tidy能检查链接错误

它按翻译单元分析，完整链接仍需要编译器驱动完成。

### 误区6：clang-tidy替代ASan

一个是静态分析，一个检查运行时执行路径。

### 误区7：故意错误案例也必须修复

教学案例应明确排除并保留。

### 误区8：复制一定比引用差

复制可能提供独立生命周期；引用可能悬空。必须结合接口语义。

### 误区9：所有参数都改成 `const&`

小型标量和枚举按值通常更合理；需要接管值时按值接收再移动也可能更好。

### 误区10：工具运行了就代表参数正确

缺少include路径、宏或正确标准会产生不可靠结果。

### 误区11：第三方头文件告警也全部修改

不要修改系统库和第三方库源码。通过头文件过滤和系统include配置控制范围。

### 误区12：重构时顺便增加新功能

这会让回归原因难以定位。重构和功能开发应分开提交。

---

## 36. 今天不要求开启的规则

以下规则今天不作为强制要求：

```text
cppcoreguidelines-*
hicpp-*
google-*
llvm-*
readability-identifier-naming
modernize-use-trailing-return-type
```

原因不是它们没有价值，而是：

- 全量开启诊断太多；
- 部分属于团队风格选择；
- 部分会要求你尚未学习的内容；
- 容易让注意力从正确性转移到格式和命名。

后续工程规范阶段再逐步加入。

---

## 37. 复盘问题

完成后回答：

1. 什么是重构？
2. 为什么重构前要建立行为基线？
3. 编译器警告、clang-tidy和ASan分别在什么时候工作？
4. 为什么clang-tidy不能替代单元测试？
5. 什么是翻译单元？
6. `--`前后参数分别属于谁？
7. 为什么需要 `-std=c++17`？
8. 为什么需要正确的 `-I`路径？
9. `compile_commands.json`解决什么问题？
10. 为什么当前不要求先统一改成CMake？
11. `.clang-tidy`中的 `-*`是什么意思？
12. `clang-analyzer-*`主要检查什么？
13. `modernize-use-override`为什么有价值？
14. `modernize-make-unique`改善了什么？
15. 范围for什么时候应该使用 `const&`？
16. 为什么小型标量不必使用引用？
17. use-after-move和use-after-free有什么区别？
18. 移动后的标准库对象能否析构和重新赋值？
19. 为什么不能继续假设移动后对象保留原值？
20. 为什么同一头文件诊断可能重复出现？
21. 为什么原始诊断行数不等于独立问题数？
22. 为什么今天第一次运行禁止 `--fix`？
23. 什么情况下可以使用具体的NOLINT？
24. 为什么不能批量添加无范围NOLINT？
25. 为什么day6故意错误示例要排除？
26. 为什么重构不能顺便改变业务输出？
27. 为什么每处理一组问题就应该重新编译？
28. 为什么前后检查必须使用相同配置？
29. 零clang-tidy诊断是否能证明没有bug？
30. 这套检查如何进入后续CMake和CI流程？

---

## 38. 验收清单

### 工具与配置

- [ ] 安装或定位clang-tidy；
- [ ] 记录LLVM版本；
- [ ] 创建基础 `.clang-tidy`；
- [ ] 能解释每一项启用规则；
- [ ] 脚本不使用 `--fix`；
- [ ] 脚本能找不到工具时明确失败。

### 基线

- [ ] 至少选择两个代表性模块；
- [ ] 记录重构前编译结果；
- [ ] 记录重构前测试结果；
- [ ] 保存重构前clang-tidy输出；
- [ ] 明确排除故意错误案例；
- [ ] 给诊断完成A～E分类。

### 重构

- [ ] 至少处理三类问题；
- [ ] 每项修改能解释原因；
- [ ] 没有机械把所有参数改成const引用；
- [ ] 没有引入裸new/delete；
- [ ] 没有改变业务输出；
- [ ] 没有批量使用NOLINT；
- [ ] 修改集中在任务范围内。

### 验证

- [ ] `-Wall -Wextra -Wpedantic`无新增警告；
- [ ] 原有测试全部通过；
- [ ] 重构前后正常输出一致；
- [ ] 保存重构后clang-tidy输出；
- [ ] 剩余诊断都有解释；
- [ ] ASan无新增错误；
- [ ] UBSan无新增错误；
- [ ] 检查Git差异；
- [ ] 完成重构报告。

---

## 39. 和实际就业的联系

企业代码质量工作通常不是“会运行一个静态检查命令”这么简单，而是：

```text
定义团队规则
提供准确编译数据库
在本地修正问题
通过代码评审判断建议是否合理
在CI中阻止新增高风险问题
保留测试和Sanitizer验证
```

今天形成的能力对应：

- 阅读遗留C++代码；
- 在不改变行为的前提下重构；
- 处理静态分析报告；
- 理解所有权和生命周期诊断；
- 建立回归验证；
- 为后续AI推理服务建立代码质量门槛。

以后你的ONNX Runtime、TensorRT和AudioLLM C++项目都可以复用：

```text
.clang-tidy
编译器警告
单元测试
ASan/UBSan
Git检查
CI
```

这比只在简历中写“了解clang-tidy”更有证明力。

---

## 40. 今天的最小完成成果

如果时间不足，最低完成：

```text
1. 安装并验证clang-tidy
2. 创建基础配置
3. 选择day17和day23两个模块
4. 保存第一次检查结果
5. 人工修复至少三类问题
6. 重新编译和运行测试
7. 保存第二次检查结果
8. 使用ASan/UBSan做回归检查
9. 写出前后对比报告
```

不要追求一次处理全部前五周代码。今天真正要学会的是：

> 如何建立可重复、可解释、不会破坏原功能的C++代码重构与静态检查流程。
