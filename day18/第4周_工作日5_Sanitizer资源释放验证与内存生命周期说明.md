# 第 4 周·工作日 5：使用 Sanitizer 验证资源释放并输出内存生命周期说明

## 0. 前置知识与超纲说明

### 今天必须掌握

- Sanitizer 是通过编译器插桩在运行时检查错误，不是普通静态警告；
- `-Wall -Wextra -Wpedantic`主要发现编译期可诊断问题，不能替代 Sanitizer；
- AddressSanitizer（ASan）重点检查越界、释放后使用、重复释放等错误；
- LeakSanitizer（LSan）用于检测泄漏，但在 macOS/Apple Clang上的可用性取决于环境；
- UndefinedBehaviorSanitizer（UBSan）检查部分未定义行为，但不能覆盖所有 C++错误；
- Sanitizer只检查实际执行到的路径，零报告不等于程序绝对正确；
- 构造/析构日志和 `alive_count`可以补充验证对象生命周期；
- `unique_ptr`移动后源指针为空，资源应由目标所有者最终释放；
- `unique_ptr`替换资源时，旧资源应在赋值过程中自动析构；
- 异常展开期间，局部 RAII对象和已构造成员会自动析构；
- 故意错误案例必须与最终正确测试分开运行；
- 最终报告必须同时说明所有权、生命周期事件、Sanitizer结果和测试覆盖范围。

### 今天会复习

- 栈、堆和对象生命周期；
- RAII；
- 构造函数与析构函数；
- `new/delete`的风险；
- `unique_ptr`、`shared_ptr`和 `weak_ptr`；
- `std::move`与移动后状态；
- 多态基类与虚析构函数；
- 模型服务独占后端；
- 后端替换和异常安全；
- 单元测试与退出码。

### 今天第一次系统理解

- 如何为资源生命周期建立可观察证据；
- 如何区分“对象析构了”和“内存错误检测器没有报警”；
- 如何设计正向测试、负向测试和最终验收测试；
- 如何阅读 Sanitizer报告中的错误类型、访问位置、分配位置和释放位置；
- 如何输出一份可复现的内存生命周期说明；
- 为什么 Sanitizer、断言、日志和代码审查需要组合使用。

### 今天只需了解，后续再深入

- Sanitizer底层影子内存与红区实现；
- 多线程下的 ThreadSanitizer（TSan）；
- MemorySanitizer（MSan）；
- Sanitizer覆盖率与模糊测试；
- CI中同时运行多组 Sanitizer任务；
- ONNX Runtime、TensorRT、CUDA专用资源的生命周期追踪；
- 自定义分配器与堆分析器。

### 超纲提示

今天会出现以下内容，但只要求理解用途：

```text
ASAN_OPTIONS环境变量
UBSan
macOS leaks工具
静态生命周期计数器
故障注入
测试夹具式资源观察
```

不要因为今天看到 UBSan、LSan 就立即扩展成复杂工具链。主线仍是使用 ASan验证 day17 的模型后端所有权。

### 今天明确不做

- 不修改 day17 已完成的基线代码；
- 不把错误案例与正常测试链接成同一个最终程序；
- 不故意运行无法控制后果的系统级破坏代码；
- 不把“没有 ASan输出”直接写成“程序百分之百无内存问题”；
- 不把泄漏检测不可用误判成代码一定泄漏；
- 不设计硬件；
- 不接入真实模型权重、GPU或部署框架；
- 不用 Sanitizer代替单元测试和所有权设计。

> 今天的目标不是得到一份空白 ASan输出，而是建立一条可解释的证据链：资源由谁创建、所有权如何转移、何时析构、错误工具检测到了什么、最终哪些路径得到验证。

---

## 1. 今日目标

完成后你应该能够：

- 从 day17复制模型服务代码到 day18作为验证对象；
- 为模型后端添加清晰的构造、析构和存活计数观察；
- 编写模型创建、移动、替换、异常和销毁测试；
- 使用 ASan编译并运行多文件项目；
- 将标准输出与标准错误同时保存到报告；
- 判断 ASan报告属于越界、释放后使用还是重复释放；
- 解释报告中的错误访问、释放位置和分配位置；
- 在支持的环境中尝试泄漏检测，并正确描述工具限制；
- 使用 `alive_count == 0`补充证明对象都已析构；
- 分离负向错误实验和最终零错误测试；
- 输出完整的 `memory_lifecycle.md`；
- 给出“已验证范围”和“尚未证明内容”。

建议用时：约 **3～4小时**。

```text
25分钟：Sanitizer能力边界复习
30分钟：复制day17基线并建立生命周期观察
35分钟：正常创建、移动与替换测试
30分钟：异常路径和多态析构测试
35分钟：独立负向错误案例
45分钟：ASan运行、报告阅读与修复
40分钟：最终零错误验收和生命周期说明
```

---

## 2. 今日目录

已经建立：

```text
day18/
├── include/
├── src/
├── tests/
├── target/
│   └── report/
└── 第4周_工作日5_Sanitizer资源释放验证与内存生命周期说明.md
```

建议最终结构：

```text
day18/
├── include/
│   ├── inferenceService.hpp
│   ├── model.hpp
│   ├── qwen.hpp
│   ├── request_result.hpp
│   ├── testRunner.hpp
│   └── whisper.hpp
├── src/
│   ├── inferenceService.cpp
│   ├── model.cpp
│   ├── qwen.cpp
│   ├── request_result.cpp
│   ├── testRunner.cpp
│   ├── whisper.cpp
│   ├── 01_lifecycle_trace.cpp
│   ├── 02_move_ownership.cpp
│   ├── 03_replace_backend.cpp
│   ├── 04_exception_cleanup.cpp
│   ├── 05_polymorphic_destruction.cpp
│   └── negative/
│       ├── 01_heap_use_after_free.cpp
│       ├── 02_double_free.cpp
│       └── 03_intentional_leak.cpp
├── tests/
│   └── resource_lifecycle_tests.cpp
├── target/
│   └── report/
│       ├── lifecycle_tests.txt
│       ├── heap_use_after_free.txt
│       ├── double_free.txt
│       ├── intentional_leak.txt
│       └── memory_lifecycle.md
└── 第4周_工作日5_Sanitizer资源释放验证与内存生命周期说明.md
```

负向案例单独放在 `src/negative/`，每次只编译一个，不能加入最终测试程序。

---

## 3. 与 day17 的关系

day17完成的是：

```text
InferenceService独占ModelBackend
unique_ptr负责模型后端生命周期
run使用服务内部后端
replace_backend转移新的独占所有权
```

day18不重新设计接口，而是验证这些承诺是否真实成立：

```text
构造服务后，调用端unique_ptr是否为空？
服务析构时，后端是否只析构一次？
替换后端时，旧后端是否立即析构？
替换失败时，旧后端是否仍然有效？
异常离开作用域时，后端是否释放？
通过基类unique_ptr销毁时，子类析构是否执行？
是否存在重复释放、悬空访问或泄漏？
```

建议复制源文件，不要直接修改 day17：

```bash
cp day17/include/*.hpp day18/include/
cp day17/src/*.cpp day18/src/
```

如果在 `CppLearning`目录执行，上述命令有效。复制后先删除或重命名 day18中不再使用的旧 `test.cpp`，避免出现多个 `main()`。

也可以只复制核心实现，并在 `tests/resource_lifecycle_tests.cpp`提供唯一主函数。

---

## 4. Sanitizer与编译警告的区别

编译警告：

```bash
-Wall -Wextra -Wpedantic
```

主要在编译阶段检查：

- 未使用变量；
- 可疑转换；
- 部分语法和规范问题；
- 编译器能够静态判断的错误。

ASan：

```bash
-fsanitize=address
```

在程序运行时检查实际发生的内存访问，例如：

- heap-buffer-overflow；
- stack-buffer-overflow；
- heap-use-after-free；
- double-free；
- invalid-free；
- 部分 use-after-return/use-after-scope；
- 部分环境中的内存泄漏。

所以：

```text
编译无警告
    ≠
运行时一定无越界和释放错误
```

反过来也一样：ASan无报告不代表代码没有未使用变量或接口设计问题。

---

## 5. ASan编译参数回顾

```bash
clang++ \
  -std=c++17 \
  -g \
  -O0 \
  -Wall \
  -Wextra \
  -Wpedantic \
  -fsanitize=address \
  -fno-omit-frame-pointer \
  source.cpp \
  -o target/program
```

参数作用：

| 参数 | 作用 |
|---|---|
| `-std=c++17` | 使用 C++17 |
| `-g` | 生成调试信息，让报告显示源码位置 |
| `-O0` | 关闭优化，便于初学阶段观察调用路径 |
| `-Wall` | 打开常见警告 |
| `-Wextra` | 打开额外警告 |
| `-Wpedantic` | 检查标准一致性相关问题 |
| `-fsanitize=address` | 启用 ASan插桩和运行库 |
| `-fno-omit-frame-pointer` | 保留帧指针，让调用栈更完整 |

注意必须在**编译和最终链接**阶段保留 Sanitizer参数。对于当前一步完成的 `clang++`命令，只写一次即可同时作用于编译和链接。

---

## 6. 什么叫“内存生命周期证据”

仅仅看到：

```text
程序退出码为0
```

不能证明每个后端都按预期析构。

今天组合四类证据：

### 6.1 行为断言

```text
正确后端执行了推理
替换后调用了新后端
错误后端抛出预期异常
```

### 6.2 构造与析构日志

```text
Construct Qwen
Destroy Qwen
```

用于观察顺序。

### 6.3 生命周期计数

```text
alive_count在构造后增加
析构后减少
测试作用域结束后恢复为0
```

用于自动断言。

### 6.4 Sanitizer报告

```text
没有已检测到的越界
没有已检测到的释放后使用
没有已检测到的重复释放
支持的环境中没有已检测到的泄漏
```

四者关注点不同，组合起来才是较完整的验证。

---

## 7. 为后端增加生命周期观察

可以在 `ModelBackend`中加入测试用计数器：

```cpp
class ModelBackend {
public:
    explicit ModelBackend(
        const std::string& name
    );

    virtual ~ModelBackend();

    static int alive_count() noexcept;
    static int created_count() noexcept;
    static int destroyed_count() noexcept;

private:
    std::string name_;

    inline static int alive_count_ = 0;
    inline static int created_count_ = 0;
    inline static int destroyed_count_ = 0;
};
```

构造：

```cpp
ModelBackend::ModelBackend(
    const std::string& name
)
    : name_(name) {
    ++alive_count_;
    ++created_count_;

    std::cout
        << "Construct backend: "
        << name_
        << '\n';
}
```

析构：

```cpp
ModelBackend::~ModelBackend() {
    --alive_count_;
    ++destroyed_count_;

    std::cout
        << "Destroy backend: "
        << name_
        << '\n';
}
```

访问函数：

```cpp
int ModelBackend::alive_count() noexcept {
    return alive_count_;
}
```

### 注意静态总计数的测试影响

`created_count`和 `destroyed_count`会跨测试累计，因此某个测试不要机械断言绝对值为1。更可靠的是记录测试前值：

```cpp
const int destroyed_before =
    ModelBackend::destroyed_count();
```

测试后验证：

```cpp
destroyed_after == destroyed_before + 1
```

`alive_count`应在测试作用域结束后回到开始值。

---

## 8. 为什么基类析构必须为虚函数

服务保存：

```cpp
std::unique_ptr<ModelBackend> backend_;
```

实际对象可能是：

```cpp
QwenOmniBackend
```

最终 `unique_ptr<ModelBackend>`会通过基类指针释放。基类需要：

```cpp
virtual ~ModelBackend();
```

销毁顺序：

```text
QwenOmniBackend析构
        ↓
ModelBackend析构
        ↓
释放完整对象内存
```

如果基类析构非虚，通过基类指针删除子类对象是未定义行为。今天必须通过子类析构日志验证动态析构链。

---

## 9. 测试一：创建和正常销毁

```cpp
void test_backend_destroyed_with_service(
    TestRunner& runner
) {
    const int alive_before =
        ModelBackend::alive_count();

    {
        auto backend =
            std::make_unique<QwenOmniBackend>(
                "Qwen2.5-Omni"
            );

        InferenceService service(
            std::move(backend)
        );

        runner.expect_true(
            backend == nullptr,
            "service takes backend ownership"
        );

        runner.expect_equal(
            ModelBackend::alive_count(),
            alive_before + 1,
            "backend is alive inside service scope"
        );
    }

    runner.expect_equal(
        ModelBackend::alive_count(),
        alive_before,
        "backend is destroyed with service"
    );
}
```

这项测试证明：

```text
创建后端 → alive增加
移动给服务 → 源unique_ptr为空
服务仍在 → 后端仍在
服务离开作用域 → 后端析构
```

---

## 10. 测试二：替换后端释放旧资源

```cpp
void test_replace_destroys_old_backend(
    TestRunner& runner
) {
    const int destroyed_before =
        ModelBackend::destroyed_count();

    InferenceService service(
        std::make_unique<QwenOmniBackend>(
            "Qwen2.5-Omni"
        )
    );

    service.replace_backend(
        std::make_unique<WhisperBackend>(
            "Whisper"
        )
    );

    runner.expect_equal(
        ModelBackend::destroyed_count(),
        destroyed_before + 1,
        "replacing backend destroys old backend"
    );
}
```

`unique_ptr`移动赋值内部负责释放旧对象，不需要：

```cpp
backend_.reset();
```

也不需要：

```cpp
delete backend_.get();
```

---

## 11. 测试三：异常路径自动清理

```cpp
void run_with_failure() {
    auto backend =
        std::make_unique<QwenOmniBackend>(
            "Qwen2.5-Omni"
        );

    throw std::runtime_error(
        "simulated configuration failure"
    );
}
```

测试：

```cpp
const int alive_before =
    ModelBackend::alive_count();

try {
    run_with_failure();
} catch (const std::runtime_error&) {
}

runner.expect_equal(
    ModelBackend::alive_count(),
    alive_before,
    "exception path releases local backend"
);
```

异常抛出时：

```text
开始栈展开
    ↓
局部unique_ptr析构
    ↓
Qwen后端析构
    ↓
进入catch
```

RAII保证资源清理，但不能保证异常前的业务操作已经完成。

---

## 12. 测试四：替换失败保持旧后端

如果 `replace_backend`直接接收已创建的 `unique_ptr`，创建失败发生在调用之前：

```cpp
auto new_backend = create_backend(type);
service.replace_backend(
    std::move(new_backend)
);
```

当 `create_backend(type)`抛异常时，`replace_backend`尚未执行，服务原后端保持不变。

测试流程：

```text
创建Qwen服务
记录Qwen推理结果
尝试create_backend("unknown")
捕获invalid_argument
再次调用service.run
验证仍是Qwen结果
验证alive_count没有意外变化
```

这验证的是状态异常安全，不只是内存安全。

---

## 13. 负向实验的目的

负向实验故意制造已知错误，用于确认：

- Sanitizer真正启用；
- 报告能够显示源码行；
- 你能识别错误访问、释放和分配位置；
- 修复后相同测试不再报错。

负向程序预计非零退出甚至中止，这是实验成功的一部分。

每个负向案例必须：

```text
独立源文件
独立可执行文件
独立报告
不加入最终测试
文件头注明“故意错误”
```

---

## 14. 负向案例一：heap-use-after-free

文件：`src/negative/01_heap_use_after_free.cpp`

```cpp
#include <iostream>
#include <memory>

int main() {
    auto value = std::make_unique<int>(42);

    int* observed = value.get();

    value.reset();

    std::cout << *observed << '\n';

    return 0;
}
```

错误原因：

```text
observed只借用地址
value.reset释放对象
observed变成悬空指针
解引用发生释放后使用
```

预期 ASan类型：

```text
heap-use-after-free
```

修复方法不是“把 observed设置为另一个随机地址”，而是保证拥有者在观察期间仍然存活，或者不在释放后使用观察指针。

---

## 15. 负向案例二：double-free

文件：`src/negative/02_double_free.cpp`

```cpp
#include <memory>

int main() {
    auto value = std::make_unique<int>(42);

    int* raw = value.get();

    delete raw;

    return 0;
}
```

执行过程：

```text
delete raw第一次释放
    ↓
value仍认为自己拥有该地址
    ↓
main结束，unique_ptr析构
    ↓
第二次释放
```

预期报告可能包含：

```text
attempting double-free
```

修复：永远不要手动释放 `unique_ptr::get()`返回的地址。

---

## 16. 负向案例三：故意泄漏

文件：`src/negative/03_intentional_leak.cpp`

```cpp
int main() {
    int* leaked = new int(42);
    static_cast<void>(leaked);
    return 0;
}
```

该程序没有越界、重复释放或释放后使用，普通 ASan在部分 macOS环境中可能不报告泄漏。

尝试：

```bash
ASAN_OPTIONS=detect_leaks=1 \
./target/03_intentional_leak
```

如果运行环境明确提示不支持泄漏检测，应在报告中写：

```text
当前Apple Clang/macOS运行环境未提供可用的LeakSanitizer结果；
因此使用生命周期计数与macOS leaks工具作为补充，不把空输出解释为无泄漏证明。
```

可选使用 macOS工具：

```bash
leaks --atExit -- ./target/03_intentional_leak
```

`leaks`不是 Sanitizer，只是平台补充工具。

---

## 17. 如何读取 ASan报告

一个典型释放后使用报告包含：

```text
ERROR: AddressSanitizer: heap-use-after-free
READ of size 4
```

先回答：

```text
错误类型是什么？
是读还是写？
访问了多少字节？
错误发生在哪一行？
```

随后通常有三组关键调用栈。

### 17.1 错误访问位置

顶部 `#0`通常指出本次非法读写发生的位置。

### 17.2 释放位置

类似：

```text
freed by thread T0 here:
```

说明该内存之前在哪里被释放。

### 17.3 分配位置

类似：

```text
previously allocated by thread T0 here:
```

说明该区域最初在哪里申请。

正确排查顺序：

```text
非法访问位置
    ↓
对象为何已经释放
    ↓
谁最初拥有并分配它
    ↓
修正所有权或访问顺序
```

不要只盯住最后的 `SUMMARY`。

---

## 18. `SUMMARY`表示什么

报告末尾：

```text
SUMMARY: AddressSanitizer: heap-use-after-free ...
```

是对本次主要错误的简短摘要，便于搜索和自动处理。

它通常包含：

- 检测工具；
- 错误类型；
- 主要源码位置或运行库位置。

`SUMMARY`不是新的第二个错误，也不能替代上面的完整调用栈。

---

## 19. ASan影子内存与红区详解

ASan之所以能在一次非法访问发生时立即报告，核心依赖两部分：

```text
编译器插入的访问检查代码
            +
记录内存可访问状态的影子内存
```

普通程序只知道某个地址中存了什么数据；ASan还要额外记录：

```text
这段地址当前能不能访问？
它属于有效对象、红区，还是已经释放的区域？
```

这种额外的状态记录区域就是 **Shadow Memory（影子内存）**。

### 19.1 应用内存与影子内存不是同一块

假设程序使用了一段普通应用内存：

```text
Application Memory

0x1000  真实变量或堆对象的数据
0x1001  真实变量或堆对象的数据
0x1002  真实变量或堆对象的数据
...
```

ASan在另一片地址空间中维护对应的影子状态：

```text
Shadow Memory

shadow(0x1000)  记录0x1000附近是否允许访问
```

影子内存通常不保存你的整数、字符串或对象内容，只保存“可访问性状态”。

因此：

```text
应用内存回答：这里存的值是什么？
影子内存回答：这里现在允许被访问吗？
```

### 19.2 为什么常说“1个影子字节管理8个应用字节”

ASan通常采用大约 1:8 的映射比例：

```text
8字节应用内存
      ↓
1字节影子内存
```

示意：

```text
应用内存：

[A0][A1][A2][A3][A4][A5][A6][A7]
                    │
                    ▼
影子内存：          [S]
```

`S`描述这8个应用字节的可访问情况。

概念上的地址映射通常可以理解为：

```text
shadow_address = (application_address >> 3) + shadow_offset
```

其中：

```text
>> 3       相当于除以8
offset     把结果映射到影子内存所在区域
```

具体 `shadow_offset`由操作系统、架构和 ASan运行时决定，不要求记忆，也不要在代码中依赖它。

### 19.3 影子字节为 `00`是什么意思

如果一个影子字节为：

```text
00
```

通常表示对应的8个应用字节全部允许访问：

```text
Shadow 00
    ↓
[可][可][可][可][可][可][可][可]
```

例如一个正常存活的 `long long`可能占8字节，它对应的影子状态可以是 `00`。

### 19.4 为什么会出现 `01`到 `07`

对象长度不一定是8的整数倍。

假设申请12字节：

```cpp
char* data = new char[12];
```

前8字节全部有效，后一个8字节分组中只有前4字节有效：

```text
应用字节：

第1组  [可][可][可][可][可][可][可][可]
影子值 00

第2组  [可][可][可][可][禁][禁][禁][禁]
影子值 04
```

所以影子值：

```text
01 到 07
```

通常表示该8字节分组中，前 N 个字节可以访问，剩余字节不可访问。

例如：

```text
01 → 只有第1个字节可访问
04 → 前4个字节可访问
07 → 前7个字节可访问
```

这也是为什么 ASan能发现“只越过对象尾部1个字节”的访问。

### 19.5 什么叫 poisoned（中毒）内存

ASan把不允许程序正常访问的区域标记为 **poisoned**，中文常译为“已中毒”。

这里的“中毒”不是说内存中存了特殊有害数据，而是：

```text
对应影子状态被标记为不可访问
```

当插桩后的代码准备访问一个地址时，会先检查对应影子状态。如果地址处于中毒区域，ASan立即报告并终止或中止当前程序。

解除禁止状态通常称为：

```text
unpoison
```

### 19.6 编译器插入的检查大概做什么

原始代码：

```cpp
int value = data[index];
```

启用 ASan后，编译器会在实际读取前加入概念上类似的检查：

```text
计算data[index]的真实地址
        ↓
映射到对应影子字节
        ↓
检查本次读取覆盖的字节是否都可访问
        ↓
可访问 → 执行读取
不可访问 → 调用ASan报告函数
```

真正生成的机器码会更复杂，也会针对访问大小进行优化；今天只理解这个流程。

---

## 19.7 什么是 Redzone（红区）

红区是 ASan故意放在有效对象周围的不可访问区域。

例如程序申请：

```cpp
int* values = new int[3];
```

有效数据是12字节。ASan管理后的概念布局可能类似：

```text
堆左红区 | values[0] | values[1] | values[2] | 堆右红区
不可访问 |    有效    |    有效    |    有效    | 不可访问
```

正常代码只能访问中间对象区域：

```cpp
values[0];
values[1];
values[2];
```

访问：

```cpp
values[3];
```

会进入右红区，ASan便能报告 `heap-buffer-overflow`。

红区不一定是操作系统完全禁止映射的独立内存页。它通常是 ASan分配和标记的区域，再依靠影子内存检查发现访问。

### 不要混淆两种“red zone”

这里讲的是：

```text
ASan redzone：围绕对象的中毒区域，用于检测越界
```

某些 CPU ABI也有名为 red zone的栈调用约定概念，那是另一个主题，与 ASan报告中的 stack redzone不是一回事。

---

## 19.8 Heap Redzone（堆红区）

使用：

```cpp
new
new[]
malloc
```

申请动态内存时，ASan分配器通常在有效区域两侧加入红区：

```text
[左堆红区][用户可访问堆块][右堆红区]
```

如果向前越界：

```cpp
values[-1] = 10;
```

可能命中左红区。

如果向后越界：

```cpp
values[3] = 10;
```

可能命中右红区。

报告中常见图例值可能包括：

```text
fa  Heap left redzone
```

不同版本的具体布局和图例以报告末尾打印的 legend为准，不要死记某个平台的实现。

---

## 19.9 Stack Redzone（栈红区，也就是你说的“红栈”）

栈变量本来可能紧挨着排列：

```cpp
void run() {
    int first[3];
    int second[3];
}
```

如果没有额外保护，`first`越界写入后可能直接覆盖 `second`，程序未必立即崩溃。

启用 ASan后，编译器会为栈帧设计带红区的布局，概念上类似：

```text
栈左红区
    ↓
first[0..2]
    ↓
栈中间红区
    ↓
second[0..2]
    ↓
栈右红区
```

示意图：

```text
[f1 f1][ first有效区域 ][f2 f2][ second有效区域 ][f3 f3]
 左红区                       中红区                   右红区
```

报告图例中常见：

```text
f1  Stack left redzone
f2  Stack mid redzone
f3  Stack right redzone
```

如果：

```cpp
first[3] = 10;
```

命中 `first`后面的栈中间红区，ASan可能报告：

```text
stack-buffer-overflow
```

### 为什么需要左、中、右三类栈红区

一个函数可能有多个局部变量。不同红区帮助运行时和报告判断越界发生在栈帧的哪个相对位置：

```text
变量之前
两个变量之间
最后一个变量之后
```

你不需要根据 `f1/f2/f3`直接推导全部源码逻辑；报告中的栈帧对象列表更重要。

---

## 19.10 如何读 stack-buffer-overflow中的对象列表

报告可能出现类似：

```text
Address ... is located in stack of thread T0 at offset ...
  This frame has 2 object(s):
    [32, 44) 'first'
    [64, 76) 'second'
```

含义：

```text
first占用栈帧偏移[32,44)
second占用栈帧偏移[64,76)
```

区间写法：

```text
[32,44)
```

表示包含32，不包含44，所以长度是：

```text
44 - 32 = 12字节
```

如果错误地址位于偏移44之后、64之前，就处于 `first`和 `second`之间的红区。

ASan通常还会明确指出：

```text
Memory access at offset ... overflows this variable
```

先根据对象列表确认越界的是哪个变量，再看影子字节验证它进入了哪类红区。

---

## 19.11 Global Redzone（全局红区）

全局或静态数组也可能被红区包围：

```cpp
int global_values[3];
```

概念布局：

```text
[全局红区][global_values有效区域][全局红区]
```

越界访问可能报告：

```text
global-buffer-overflow
```

图例中常见：

```text
f9  Global redzone
```

因此错误类型中的：

```text
heap-buffer-overflow
stack-buffer-overflow
global-buffer-overflow
```

主要区别是被越界对象的存储区域不同，不是越界本质不同。

---

## 19.12 释放后的堆区域为什么常显示 `fd`

执行：

```cpp
int* value = new int(42);
delete value;
```

释放后，ASan会把对应堆区域标记为不可访问。报告 legend中常见：

```text
fd  Freed heap region
```

之后：

```cpp
std::cout << *value;
```

访问命中 `fd`标记区域，报告：

```text
heap-use-after-free
```

这不是因为指针值自动变成了特殊数字。指针中可能仍保存原地址，但该地址对应的影子状态已经被标记为“已释放”。

### Quarantine（隔离区）的作用

ASan通常不会立刻把刚释放的堆块交给下一次分配重复使用，而是暂时放入隔离区：

```text
delete/free
    ↓
标记为已释放
    ↓
暂时进入quarantine
    ↓
延迟重新利用
```

这样悬空指针继续访问时，更容易命中“已释放”状态，而不是刚好访问到一个已经重新分配给其他对象的有效区域。

隔离区大小有限，所以 ASan也不能保证无限期发现所有陈旧指针访问。

---

## 19.13 Stack-use-after-scope与Stack-use-after-return

### 离开局部作用域后使用

```cpp
int* pointer = nullptr;

{
    int value = 42;
    pointer = &value;
}

std::cout << *pointer;
```

`value`离开内部作用域后生命周期结束。支持相应插桩时，影子状态可被标记为：

```text
f8  Stack use after scope
```

报告可能是：

```text
stack-use-after-scope
```

### 函数返回后使用局部地址

```cpp
int* bad() {
    int value = 42;
    return &value;
}
```

函数返回后，原栈帧生命周期结束。相关影子图例可能出现：

```text
f5  Stack after return
```

ASan对 use-after-return的具体检测策略、默认设置和支持情况依赖编译器与平台，有时会使用“fake stack”延长可检测窗口。

可以了解以下运行选项，但不要假设所有环境表现完全一致：

```bash
ASAN_OPTIONS=detect_stack_use_after_return=1
```

对于离开作用域检测，编译器还可能支持：

```bash
-fsanitize-address-use-after-scope
```

是否默认启用以及实际支持程度以当前 Clang版本为准。

---

## 19.14 常见影子字节图例

ASan报告末尾通常会自己打印 legend。常见含义如下：

| 影子值 | 常见含义 |
|---|---|
| `00` | 对应8字节全部可访问 |
| `01`～`07` | 对应8字节中前 N 字节可访问 |
| `fa` | 堆左红区 |
| `fd` | 已释放堆区域 |
| `f1` | 栈左红区 |
| `f2` | 栈中间红区 |
| `f3` | 栈右红区 |
| `f5` | 函数返回后的栈区域 |
| `f8` | 离开作用域后的栈区域 |
| `f9` | 全局红区 |
| `f6` | 全局初始化顺序相关状态 |
| `f7` | 用户主动中毒区域 |
| `fc` | 容器溢出标记 |
| `ac` | 数组cookie相关区域 |
| `bb` | 对象内部红区 |
| `ca` | `alloca`左红区 |
| `cb` | `alloca`右红区 |
| `fe` | ASan内部状态 |

注意：

> 报告中实际打印的 legend才是当前工具版本的最终依据。这张表用于帮助理解，不要求死记全部十六进制值。

---

## 19.15 如何看 `Shadow bytes around the buggy address`

报告可能显示：

```text
Shadow bytes around the buggy address:
  ...: fa fa 00 04 fa fa fa fa
=>...: fa fa fd[fd]fd fa fa fa
  ...: fa fa fa fa fa fa fa fa
```

阅读顺序：

1. 找到带 `=>`的行；
2. 找到方括号包围的影子字节；
3. 根据 legend判断该字节代表什么；
4. 再回到报告顶部确认错误类型和源码行；
5. 用分配/释放调用栈解释为什么该区域处于这个状态。

例如：

```text
[fd]
```

通常说明错误地址落在已释放的堆区域中，与：

```text
heap-use-after-free
```

相互印证。

如果箭头附近是：

```text
[f2]
```

通常表示访问落入栈变量之间的中间红区，需要查看栈对象列表判断哪个局部数组越界。

### 方括号不是应用数据

报告中的：

```text
[fd]
```

不是说你的原始内存中存了十六进制 `fd`。这是对应地址的影子状态。

---

## 19.16 “4 bytes after 12-byte region”如何与红区对应

报告：

```text
0x... is located 4 bytes after 12-byte region
```

假设有效区域从地址 `B`开始：

```text
有效范围：[B, B + 12)
```

也就是最后一个有效字节地址为：

```text
B + 11
```

“4 bytes after”表示错误访问起始地址位于：

```text
B + 12 + 4
```

它已经越过有效对象结尾，并落在右侧红区或相邻的中毒区域。

如果本次是：

```text
READ of size 4
```

表示程序从这个非法起点尝试读取4字节。报告中的“4 bytes after”描述的是访问起点相对对象边界的位置，不是读取大小；读取大小由 `READ of size 4`单独说明。

---

## 19.17 为什么红区能发现越界，却不能发现所有逻辑越界

假设：

```cpp
std::vector<int> values;
values.reserve(100);
values.push_back(1);
```

业务上只有 `values[0]`属于当前 `size()`范围，但底层分配容量可能足够容纳100个元素。

如果程序通过某些低层方式访问了同一已分配块内部、但超出逻辑 `size()`的区域，基础 ASan不一定总能判断这违反了容器语义，因为它主要看到的是“这块内存是否属于有效分配”。部分标准库和工具链支持容器注解，检测能力会更强，但不能统一假设。

同样，错误写入一个对象内部的另一个合法成员区域，也未必跨越红区。

因此：

```text
ASan擅长发现跨越实际内存边界和访问已释放区域
单元测试、边界检查和容器接口负责补充逻辑正确性
```

---

## 19.18 影子内存和红区带来的开销

ASan不是零成本工具。开销来源包括：

- 影子内存本身；
- 对象周围的红区；
- 已释放对象隔离区；
- 每次内存访问前的插桩检查；
- 更完整的调用栈和分配记录。

因此 ASan构建通常：

```text
运行更慢
占用更多内存
可执行文件更大
```

它非常适合开发、测试和 CI，不应该未经评估直接作为生产性能版本。

---

## 19.19 ASan可能检测不到的情况

即使理解影子内存，也必须知道边界：

- 错误代码路径没有执行；
- 未经过 ASan插桩的外部代码执行了某些访问；
- 自定义分配器没有被正确识别；
- 逻辑越界仍位于同一合法分配内部；
- 数据竞争导致的问题应主要使用 TSan等工具；
- 对象业务状态错误但内存访问仍然合法；
- 已释放内存很快重新分配，时间窗口和检测效果受运行时策略影响；
- 内联汇编或特殊硬件访问可能绕开普通插桩；
- 当前平台不支持某些 Sanitizer功能。

所以最终结论必须写成：

```text
在本次测试执行到的路径上，ASan没有报告其能够检测的内存错误。
```

而不是：

```text
程序绝对没有任何内存问题。
```

---

## 19.20 影子内存报告的实际排查口诀

看到 ASan报告后按以下顺序：

```text
第一步：看错误类型
heap/stack/global？overflow还是use-after-free？

第二步：看READ/WRITE和size
程序在读还是写？一次访问多少字节？

第三步：看顶部#0源码位置
真正触发错误的语句在哪里？

第四步：看对象边界描述
在区域前、区域后，还是已释放区域内部？

第五步：看分配与释放调用栈
谁创建？谁释放？为什么仍然访问？

第六步：看Shadow bytes
用fd/fa/f1/f2/f3等状态验证判断

第七步：修复所有权或边界
不要只隐藏报错或把指针设成随机值

第八步：重新运行测试
确认原案例修复且最终测试零错误
```

影子字节用于辅助验证，不应该取代源码调用栈和所有权分析。

---

## 20. 为什么 ASan无输出可能有多种含义

运行正常程序：

```bash
./target/resource_lifecycle_tests
```

ASan默认只在检测到问题时向标准错误输出报告。因此“没有 ASan错误文本”可能意味着：

1. 当前执行路径没有检测到问题；
2. 错误路径没有被测试覆盖；
3. Sanitizer没有正确加入最终程序；
4. 该问题不属于 ASan能够检测的类型；
5. 平台不支持某项检查，例如某些泄漏检测情况。

所以今天先运行故意错误案例，确认 ASan确实生效，再运行最终正确测试。

---

## 21. 检查可执行文件是否链接 ASan

最直接的方法是运行一个已知错误案例。如果报告出现：

```text
ERROR: AddressSanitizer
```

说明插桩和运行库工作。

也可以在 macOS上查看动态库依赖：

```bash
otool -L target/resource_lifecycle_tests
```

输出形式受编译器版本影响，不要求今天依赖具体库名做自动判断。

关键仍是：编译命令确实包含：

```bash
-fsanitize=address
```

而且你运行的是刚编译的那个可执行文件。

---

## 22. 多文件最终测试编译命令

在 `day18`目录执行：

```bash
clang++ \
  -std=c++17 \
  -g \
  -O0 \
  -Wall \
  -Wextra \
  -Wpedantic \
  -fsanitize=address \
  -fno-omit-frame-pointer \
  src/inferenceService.cpp \
  src/model.cpp \
  src/qwen.cpp \
  src/request_result.cpp \
  src/testRunner.cpp \
  src/whisper.cpp \
  tests/resource_lifecycle_tests.cpp \
  -I include \
  -o target/resource_lifecycle_tests
```

这里 `tests/resource_lifecycle_tests.cpp`应当是唯一包含 `main()`的文件。

如果把旧 `src/test.cpp`也加入命令，会出现多个 `main`定义的链接错误。

---

## 23. 负向案例编译与报告命令

### heap-use-after-free

```bash
clang++ \
  -std=c++17 \
  -g \
  -O0 \
  -Wall \
  -Wextra \
  -Wpedantic \
  -fsanitize=address \
  -fno-omit-frame-pointer \
  src/negative/01_heap_use_after_free.cpp \
  -o target/01_heap_use_after_free
```

运行并保存：

```bash
./target/01_heap_use_after_free \
  > target/report/heap_use_after_free.txt \
  2>&1

echo $?
```

预计退出码非0。

### double-free

```bash
clang++ \
  -std=c++17 \
  -g \
  -O0 \
  -Wall \
  -Wextra \
  -Wpedantic \
  -fsanitize=address \
  -fno-omit-frame-pointer \
  src/negative/02_double_free.cpp \
  -o target/02_double_free
```

```bash
./target/02_double_free \
  > target/report/double_free.txt \
  2>&1

echo $?
```

不要因为负向实验退出码非0就把它写成测试失败；预期结果就是被 ASan中止。

---

## 24. 最终正确程序运行命令

```bash
./target/resource_lifecycle_tests \
  > target/report/lifecycle_tests.txt \
  2>&1

exit_code=$?

cat target/report/lifecycle_tests.txt
echo "exit_code=$exit_code"
```

注意 zsh中 `status`是特殊只读变量，不要写：

```bash
status=$?
```

使用：

```bash
exit_code=$?
```

最终期望：

```text
所有断言PASS
没有ERROR: AddressSanitizer
alive_count回到0
created_count与destroyed_count差值回到0
exit_code=0
```

---

## 25. 可选：同时尝试 UBSan

可以单独构建一个 ASan+UBSan版本：

```bash
clang++ \
  -std=c++17 \
  -g \
  -O0 \
  -Wall \
  -Wextra \
  -Wpedantic \
  -fsanitize=address,undefined \
  -fno-omit-frame-pointer \
  src/inferenceService.cpp \
  src/model.cpp \
  src/qwen.cpp \
  src/request_result.cpp \
  src/testRunner.cpp \
  src/whisper.cpp \
  tests/resource_lifecycle_tests.cpp \
  -I include \
  -o target/resource_lifecycle_tests_sanitized
```

运行：

```bash
./target/resource_lifecycle_tests_sanitized \
  > target/report/lifecycle_tests_sanitized.txt \
  2>&1
```

UBSan是补充项。今天主要验收仍以 ASan和生命周期测试为准。

---

## 26. 内存生命周期说明应该写什么

文件：`target/report/memory_lifecycle.md`

至少包含：

```markdown
# 模型服务内存生命周期说明

## 1. 资源清单
- InferenceService独占一个ModelBackend
- backend_类型为unique_ptr<ModelBackend>
- Qwen和Whisper通过虚析构安全销毁

## 2. 创建路径
- make_unique创建具体后端
- unique_ptr持有动态对象
- 构造InferenceService时所有权移动到backend_

## 3. 使用路径
- run只通过backend_调用后端
- run不创建第二个所有者

## 4. 替换路径
- 新后端先创建成功
- replace_backend接收unique_ptr
- 移动赋值自动销毁旧后端
- 服务开始独占新后端

## 5. 异常路径
- 创建失败时服务原状态不变
- 局部unique_ptr在栈展开时自动析构

## 6. 销毁路径
- InferenceService离开作用域
- backend_析构
- 具体子类析构
- ModelBackend基类析构
- 动态内存释放

## 7. 验证证据
- 单元测试结果
- alive_count最终值
- created/destroyed计数
- ASan运行结果
- 负向案例报告

## 8. 验证边界
- 仅覆盖当前测试执行路径
- 未证明多线程安全
- 未接入真实ONNX/TensorRT/CUDA资源
- 当前平台泄漏检测能力说明
```

不能只贴命令，要用自己的话解释事件顺序。

---

## 27. 推荐的生命周期图

将下面内容放入报告：

```text
make_unique<QwenOmniBackend>()
            │
            ▼
调用者unique_ptr拥有Qwen
            │ std::move
            ▼
InferenceService::backend_拥有Qwen
            │
            ├── run()临时使用，不转移所有权
            │
            ├── replace_backend(Whisper)
            │       ├── 旧Qwen析构
            │       └── backend_接管Whisper
            │
            ▼
InferenceService离开作用域
            │
            ▼
backend_析构 → Whisper析构 → 内存释放
```

该图要与实际测试日志顺序一致。

---

## 28. 常见错误

### 错误1：只给某个 `.cpp`加 ASan，最终链接没加

统一使用一条 `clang++`命令可降低这种问题。

### 错误2：运行了旧的可执行文件

修改代码后必须重新编译，并确认输出路径。

### 错误3：负向案例和正确测试一起链接

会出现多个 `main`或让最终验收必然中止。

### 错误4：用 `get()`取得地址后手动 `delete`

会破坏 `unique_ptr`所有权并造成重复释放。

### 错误5：移动后继续解引用源 `unique_ptr`

源指针已经为空。

### 错误6：只检查 `backend != nullptr`

非空不代表地址没有悬空；必须保证所有者仍存在。

### 错误7：基类析构函数不是虚函数

通过基类指针销毁子类对象会产生未定义行为。

### 错误8：看到没有泄漏报告就宣称绝对无泄漏

平台可能未启用泄漏检测，或者泄漏路径没有执行。

### 错误9：只看 `SUMMARY`

必须结合错误访问、释放和分配调用栈。

### 错误10：测试结束时还有故意存活的对象

在对象仍处于作用域内时断言 `alive_count == 0`会错误失败。生命周期断言应放在内层作用域结束后。

---

## 29. 练习1：复制并建立基线

将 day17核心代码复制到 day18，完成：

```text
1. 正常编译
2. 运行原测试
3. 保存原测试输出
4. 确认没有裸拥有型new/delete
5. 确认InferenceService使用unique_ptr成员
6. 确认run不再接收后端参数
```

报告基线结果后再添加生命周期观察，不要同时进行无关重构。

---

## 30. 练习2：构造析构日志

为 `ModelBackend`、`QwenOmniBackend`和 `WhisperBackend`增加足够的析构日志。

要求验证：

```text
Qwen子类析构先执行
ModelBackend基类析构后执行
每个构造对象只析构一次
```

不要依赖日志作为唯一自动测试证据，还要增加计数断言。

---

## 31. 练习3：移动所有权测试

验证：

```cpp
auto backend =
    std::make_unique<QwenOmniBackend>(
        "Qwen2.5-Omni"
    );

InferenceService service(
    std::move(backend)
);
```

要求：

- 移动前 `backend != nullptr`；
- 移动后 `backend == nullptr`；
- 服务可以正常运行；
- 服务作用域内 `alive_count`增加1；
- 服务作用域结束后计数恢复；
- ASan无错误。

---

## 32. 练习4：替换生命周期测试

服务先拥有 Qwen，再替换为 Whisper。

要求记录并验证：

```text
创建Qwen后alive增加1
创建Whisper临时所有者后alive增加2
移动替换时Qwen析构，alive降为1
服务销毁时Whisper析构，alive回到起始值
```

注意对象创建与函数实参求值顺序的日志细节可能受到表达式组织影响。为了观察清楚，先把新后端放入具名 `unique_ptr`，再调用替换。

---

## 33. 练习5：异常路径测试

至少覆盖：

```text
空后端构造服务 → invalid_argument
未就绪后端run → runtime_error
未知工厂类型 → invalid_argument
局部unique_ptr之后抛异常 → 自动析构
替换创建失败 → 原后端保持可用
```

每项异常测试都要验证：

- 捕获到正确异常类型；
- 消息包含预期关键词；
- `alive_count`恢复或保持预期值；
- 后续对象仍处于合法状态。

---

## 34. 练习6：ASan负向报告分析

分别运行释放后使用和重复释放案例，为每份报告填写：

```text
错误类型：
非法操作是READ还是WRITE：
访问字节数：
错误访问源码位置：
释放源码位置：
分配源码位置：
根本原因：
所有权关系：
修复方法：
修复后结果：
```

不要只复制报告原文。

---

## 35. 练习7：泄漏能力验证

运行故意泄漏案例：

```bash
ASAN_OPTIONS=detect_leaks=1 \
./target/03_intentional_leak
```

记录以下三种情况中的实际一种：

```text
1. 成功报告泄漏
2. 明确提示当前环境不支持泄漏检测
3. 没有泄漏报告，也没有明确支持证据
```

第3种不能写成“验证无泄漏”。应依赖 `alive_count`、析构日志以及可用的平台工具补充。

---

## 36. 练习8：最终资源生命周期测试集

至少包含：

```text
1. 创建具体后端增加alive_count
2. unique_ptr移动后源为空
3. 服务接管后端后可以run
4. 服务析构后后端销毁
5. replace_backend销毁旧后端
6. 替换后新后端正常run
7. 空后端被拒绝且无泄漏
8. 未知工厂类型不破坏原服务状态
9. 异常展开释放局部后端
10. 基类unique_ptr触发子类析构
11. created_count与destroyed_count平衡
12. 最终alive_count为0
13. ASan无错误
14. 可选UBSan无错误
```

最终测试不得包含任何故意悬空访问或重复释放。

---

## 37. 今日必须回答的问题

1. 编译警告和 Sanitizer有什么区别？
2. ASan是在编译期还是运行期发现错误？
3. `-g`为什么对报告有帮助？
4. `-fno-omit-frame-pointer`有什么作用？
5. ASan主要能检测哪些资源错误？
6. 为什么 ASan无输出不等于程序绝对正确？
7. 什么叫实际执行路径覆盖？
8. 为什么需要一个故意错误案例确认 ASan生效？
9. 构造析构日志能证明什么？
10. `alive_count`相比日志有什么优势？
11. 静态总计数为什么要使用测试前后差值？
12. 服务接管 `unique_ptr`后，源指针应是什么状态？
13. 替换 `unique_ptr`成员时旧对象何时析构？
14. 为什么不需要先调用 `reset()`？
15. 异常展开时局部 `unique_ptr`为什么会析构？
16. RAII能保证业务操作一定完成吗？
17. 多态销毁为什么要求虚析构函数？
18. `heap-use-after-free`报告最重要的三组位置是什么？
19. `SUMMARY`表示什么？
20. 为什么不能 `delete unique_ptr.get()`？
21. 为什么负向案例不能加入最终测试？
22. macOS上没有泄漏报告时应该如何描述？
23. `leaks`与 Sanitizer是什么关系？
24. 为什么最终报告必须写验证边界？
25. 今天怎样证明模型后端资源完成了一次完整生命周期？
26. 影子内存保存的是业务数据还是可访问状态？
27. 为什么一个影子字节通常可以描述8个应用字节？
28. 影子值 `00`以及 `01`到 `07`分别表示什么？
29. poisoned内存是什么意思？
30. ASan红区为什么能帮助检测数组越界？
31. 堆红区、栈红区和全局红区分别围绕什么对象？
32. 栈左红区、中间红区和右红区分别有什么作用？
33. `fd`与 `f1/f2/f3`常见含义是什么？
34. 如何读取 `Shadow bytes around the buggy address`中的箭头和方括号？
35. 为什么进入同一合法分配内部的逻辑越界不一定被基础ASan发现？

---

## 38. 今日验收清单

### 基线与代码

- [ ] day17代码已复制到 day18；
- [ ] day17原目录未被修改；
- [ ] day18基线编译通过；
- [ ] 正式程序只有一个 `main()`；
- [ ] 服务继续使用 `unique_ptr<ModelBackend>`；
- [ ] 基类析构函数为虚函数；
- [ ] 正式源码没有裸拥有型 `new/delete`。

### 生命周期测试

- [ ] 后端构造时计数增加；
- [ ] 后端析构时计数减少；
- [ ] 移动后源 `unique_ptr`为空；
- [ ] 服务析构释放后端；
- [ ] 后端替换释放旧对象；
- [ ] 异常路径释放局部资源；
- [ ] 失败替换保留原后端；
- [ ] 多态析构链完整；
- [ ] 最终 `alive_count`为0。

### Sanitizer

- [ ] 编译命令包含 ASan参数；
- [ ] 释放后使用案例被检测；
- [ ] 重复释放案例被检测；
- [ ] 两份错误报告已保存；
- [ ] 能指出访问、释放和分配位置；
- [ ] 能解释影子内存的8:1映射；
- [ ] 能解释 `00`和部分可寻址影子值；
- [ ] 能区分堆、栈和全局红区；
- [ ] 能解释栈左、中、右红区；
- [ ] 能根据 `fd/f1/f2/f3`辅助判断错误区域；
- [ ] 能从影子字节箭头定位错误对应状态；
- [ ] 已记录当前平台泄漏检测能力；
- [ ] 最终正确测试无 ASan错误；
- [ ] 最终退出码为0。

### 输出

- [ ] `lifecycle_tests.txt`已生成；
- [ ] 负向案例报告已生成；
- [ ] `memory_lifecycle.md`已完成；
- [ ] 报告包含所有权关系图；
- [ ] 报告包含构造、移动、替换、异常和销毁路径；
- [ ] 报告明确说明验证边界。

---

## 39. 与 AI 部署工程的联系

后续模型部署会管理比模拟后端更多的资源：

```text
模型运行时会话
输入输出张量
音频缓冲区
线程任务
网络请求上下文
CPU内存
GPU相关句柄
框架专用对象
```

不同资源释放方式可能不是普通 `delete`，但验证方法仍然继承今天的思想：

```text
明确所有权
    ↓
封装为RAII对象
    ↓
记录创建和销毁事件
    ↓
测试正常、替换和异常路径
    ↓
使用对应Sanitizer/分析器
    ↓
输出可复现的生命周期说明
```

AI部署岗位真正需要的不是“会运行一次 ASan”，而是能够回答：

- 服务热切换模型时旧资源是否及时释放？
- 请求异常时上下文是否泄漏？
- 异步任务是否访问了已卸载模型？
- 多态后端能否通过统一接口安全销毁？
- 测试覆盖了哪些生命周期路径？

今天的模拟模型服务就是这些工程问题的入门版本。

---

## 40. 今日项目产出

今日应形成以下可保存成果：

```text
1. day18模型资源生命周期测试工程
2. heap-use-after-free负向报告
3. double-free负向报告
4. 当前平台泄漏检测能力记录
5. 最终零ASan错误测试报告
6. memory_lifecycle.md生命周期说明
```

这不是一个新的大型简历项目，而是 day17模型服务项目的质量验证材料。以后它可以并入完整推理服务仓库中的：

```text
tests/
docs/memory-lifecycle.md
sanitizer-reports/
```

用于证明你不仅实现功能，还验证资源安全和异常路径。

---

## 41. 今日 Git 提交建议

```bash
git status
git add day18
git commit -m "Verify model resource lifecycle with sanitizers"
```

README建议记录：

```text
1. 使用构造析构日志和alive_count观察资源生命周期
2. 验证unique_ptr移动、后端替换与服务析构
3. 验证异常路径RAII自动清理
4. 使用ASan识别释放后使用和重复释放
5. 负向案例与最终正确测试分离
6. macOS泄漏检测能力按实际环境记录
7. 最终报告只声明已测试路径上的检测结果
```

---

## 42. 今日一句话总结

```text
资源安全不是“用了智能指针就结束”，而是要用所有权设计、生命周期计数、异常测试和Sanitizer共同证明模型资源在创建、移动、替换和销毁路径上都按预期工作，并诚实说明尚未覆盖的边界。
```
