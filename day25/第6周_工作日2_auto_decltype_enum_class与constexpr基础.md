# 第6周·工作日2：掌握 `auto`、`decltype`、`enum class`和 `constexpr`基础

## 0. 今天要完成什么

今天学习四组现代 C++基础工具：

```text
auto        -> 根据初始化表达式推导变量类型
decltype    -> 查询表达式或名字对应的类型
enum class  -> 创建有作用域、类型安全的枚举
constexpr   -> 表达可以参与编译期计算的值或函数
```

今天结束后，你应当能够：

1. 正确判断 `auto`是否保留 `const`和引用；
2. 解释 `decltype(name)`与 `decltype((name))`的不同；
3. 用 `enum class`替代魔法整数和不安全枚举；
4. 区分 `const`、`constexpr`和运行时常量；
5. 编写简单 `constexpr`函数和 `if constexpr`；
6. 使用 `static_assert`验证编译期结果；
7. 完成类型安全的音频处理配置与编译期工具项目。

---

## 1. 前置知识与超纲说明

### 今天必须掌握

- `auto`必须有可用于推导的初始化信息；
- `auto`按值推导通常去掉顶层 `const`和引用；
- `auto&`、`const auto&`、`auto&&`的基础区别；
- 范围 `for`中 `auto`、`auto&`和 `const auto&`；
- `decltype(variable)`；
- `decltype((variable))`；
- `decltype(expression)`的值类别基础；
- `enum class`声明、作用域和底层类型；
- 枚举与整数之间显式转换；
- `const`和 `constexpr`区别；
- `constexpr`变量和函数；
- `static_assert`；
- C++17 `if constexpr`基础。

### 今天只要求认识

- `decltype(auto)`；
- 转发引用；
- 引用折叠；
- 完美转发；
- `std::underlying_type_t`；
- 枚举位掩码运算符重载；
- `std::integral_constant`；
- 模板元编程；
- C++20 `consteval`和 `constinit`。

### 超纲提醒

出现 `auto&&`时，今天只分析常见范围 `for`与临时对象场景，不深入转发引用规则。出现 `decltype(auto)`时只解释其保留类型能力，不要求用于正式项目接口。

### C++版本

继续使用：

```bash
-std=c++17
```

以下不是今天可用的 C++17关键字：

```cpp
consteval
constinit
```

---

## 2. 今日目录结构

```text
day25/
├── include/
│   ├── type_inspection.hpp
│   ├── audio_types.hpp
│   ├── audio_math.hpp
│   └── audio_processing_config.hpp
├── src/
│   ├── 01_auto_basics.cpp
│   ├── 02_auto_cv_reference.cpp
│   ├── 03_decltype_rules.cpp
│   ├── 04_auto_return_type.cpp
│   ├── 05_enum_class_basics.cpp
│   ├── 06_enum_conversion.cpp
│   ├── 07_constexpr_variables.cpp
│   ├── 08_constexpr_functions.cpp
│   ├── 09_if_constexpr.cpp
│   └── 10_audio_processing_config.cpp
├── tests/
│   └── modern_type_tests.cpp
├── target/
│   └── report/
│       └── auto_decltype_constexpr_report.md
└── 第6周_工作日2_auto_decltype_enum_class与constexpr基础.md
```

每个练习均标明文件名。源码由你实现，文档不直接给综合项目完整答案。

---

## 3. 建议时间

```text
60分钟：auto及const/引用推导
75分钟：decltype规则
45分钟：enum class
75分钟：constexpr与if constexpr
120分钟：练习1～9
90分钟：综合项目和测试
30分钟：报告与复盘
```

优先顺序：

```text
auto的const/引用规则
> decltype括号差异
> enum class类型安全
> const与constexpr
> constexpr函数
> 综合项目
```

---

## 4. `auto`不等于动态类型

```cpp
auto value = 42;
```

编译器在编译期推导：

```text
value的类型是int
```

类型确定后不能在运行时变成另一种类型：

```cpp
value = 3.5;
```

这里不是让 `value`变成 `double`，而是把 `3.5`转换成 `int`再赋值，可能得到3。

所以 `auto`只是省略显而易见的静态类型，不是 Python式动态变量。

---

## 5. `auto`需要初始化表达式

正确：

```cpp
auto count = 10;
auto rate = 16000.0;
auto name = std::string("audio");
```

错误：

```cpp
auto value;
```

没有初始化表达式，编译器无法知道类型。

函数返回类型使用 `auto`时，规则不同，后面单独讲。

---

## 6. 基本类型推导

```cpp
auto count = 10;            // int
auto rate = 16000.0;        // double
auto ratio = 0.5F;          // float
auto enabled = true;        // bool
auto character = 'a';       // char
auto text = "audio";        // const char*
```

最后一项尤其重要：

```cpp
auto text = "audio";
```

不会自动得到 `std::string`，而是得到指向字符串字面量的指针类型。

需要字符串对象应写：

```cpp
auto text = std::string("audio");
```

或者：

```cpp
std::string text = "audio";
```

---

## 7. `auto`按值推导会去掉顶层 `const`

```cpp
const int original = 10;
auto copy = original;
```

`copy`推导为：

```cpp
int
```

不是 `const int`。

因此可以：

```cpp
copy = 20;
```

这不会修改 `original`，因为 `copy`是独立副本。

这里去掉的是“顶层 `const`”，即对象本身不能被修改的限定。

---

## 8. 底层 `const`通常会保留

```cpp
const int value = 10;
const int* pointer = &value;

auto copied_pointer = pointer;
```

`copied_pointer`仍然是：

```cpp
const int*
```

指向的 `int`仍然不能通过该指针修改。

可以粗略区分：

```text
const int value       -> 顶层const修饰对象
const int* pointer    -> 底层const修饰所指对象
```

更复杂的 cv推导后续结合模板推导继续学习。

---

## 9. `auto&`保留引用关系

```cpp
int original = 10;
auto& reference = original;
```

推导结果：

```cpp
int&
```

修改：

```cpp
reference = 20;
```

会同时改变 `original`。

如果原对象为常量：

```cpp
const int original = 10;
auto& reference = original;
```

推导为：

```cpp
const int&
```

不能通过它修改对象。

---

## 10. `const auto&`

```cpp
const auto& record = records.front();
```

它表示：

- 不复制元素；
- 通过 `record`不能修改元素；
- `record`生命周期依赖被引用对象；
- 容器修改可能使引用失效。

只读遍历通常推荐：

```cpp
for (const auto& record : records) {
    // 只读
}
```

它尤其适合 `std::string`或结构体等复制成本高于整数的元素。

---

## 11. 范围 `for`中的三种 `auto`

### 复制每个元素

```cpp
for (auto record : records) {
}
```

`record`是副本，修改它不影响容器。

### 修改容器元素

```cpp
for (auto& record : records) {
    record.quality_score = 1.0;
}
```

### 只读且避免复制

```cpp
for (const auto& record : records) {
}
```

这三种语义必须主动选择，不要无脑使用 `auto`。

---

## 12. `auto&&`基础认识

```cpp
auto&& value = expression;
```

它可能绑定左值或右值，具体类型涉及引用折叠。

范围 `for`中有时看到：

```cpp
for (auto&& element : container) {
}
```

它可以保留代理引用等特殊元素语义，但今天不深入。普通容器按需求优先使用：

```cpp
auto&
const auto&
```

不要把所有引用都改成 `auto&&`。

---

## 13. `auto`与花括号初始化

```cpp
auto first{1};
```

推导为 `int`。

```cpp
auto second = {1};
```

通常推导为：

```cpp
std::initializer_list<int>
```

```cpp
auto third = {1, 2, 3};
```

也是 `std::initializer_list<int>`。

混合元素类型可能推导失败：

```cpp
auto values = {1, 2.5};
```

为了避免含糊，业务代码需要明确容器时直接写：

```cpp
std::vector<double> values{1, 2.5};
```

---

## 14. 什么时候适合使用 `auto`

适合：

- 迭代器类型很长；
- Lambda闭包类型无法直接拼写；
- 初始化表达式已经清楚表达类型；
- 避免重复复杂模板类型；
- 接收算法返回的迭代器。

```cpp
const auto iterator = index.find(path);
```

不适合：

- 类型决定重要业务语义却被隐藏；
- 数值精度必须一眼明确；
- 隐式转换容易造成错误；
- 没有初始化；
- 使用者无法快速判断复制还是引用。

---

## 15. 函数返回类型 `auto`

C++14起可以根据 `return`表达式推导：

```cpp
auto add(int left, int right) {
    return left + right;
}
```

返回类型推导为 `int`。

不同返回分支必须推导为一致类型：

```cpp
auto choose(bool flag) {
    if (flag) {
        return 1;
    }

    return 2.5; // int与double不一致，通常报错
}
```

另外，普通 `auto`返回通常采用类似按值推导，可能丢失引用和顶层 `const`。

---

## 16. 什么是 `decltype`

`decltype`用于获取名字或表达式对应的类型：

```cpp
int value = 10;
decltype(value) another = 20;
```

`another`类型为：

```cpp
int
```

`decltype`在编译期工作，通常不会执行其括号中的普通表达式。

---

## 17. `decltype(name)`的特殊规则

如果括号中是没有额外括号的变量名或成员访问，`decltype`得到其声明类型。

```cpp
const int value = 10;
decltype(value) copy = 20;
```

`copy`是：

```cpp
const int
```

与 `auto`对比：

```cpp
auto first = value;       // int
decltype(value) second{}; // const int
```

`decltype`保留声明中的 `const`和引用信息。

---

## 18. `decltype((name))`为什么不同

```cpp
int value = 10;
```

```cpp
decltype(value) first = value;
```

`first`是：

```cpp
int
```

但：

```cpp
decltype((value)) second = value;
```

`(value)`作为表达式是左值，因此类型为：

```cpp
int&
```

`second`是 `value`的引用，修改会影响 `value`。

这是今天必须记住的高频区别：

```text
decltype(value)   -> 变量声明类型
decltype((value)) -> 根据表达式值类别，左值通常得到T&
```

---

## 19. `decltype(expression)`与值类别

简化规则：

```text
表达式是左值    -> T&
表达式是将亡值  -> T&&
表达式是纯右值  -> T
```

例子：

```cpp
int value = 10;

decltype(value + 1) result = 20;
```

`value + 1`产生临时结果，是纯右值，因此得到 `int`。

```cpp
decltype(*pointer)
```

解引用表达式通常是左值，所以可能得到引用类型。

将亡值和完整值类别体系后续再深入。

---

## 20. `decltype`表达式通常不执行

```cpp
int function();

decltype(function()) result = 10;
```

编译器分析 `function()`的类型，但不会因为 `decltype`而调用函数。

但是表达式必须在语法和类型上有效。

不能用 `decltype`绕过不存在成员：

```cpp
decltype(object.not_existing())
```

仍然可能编译失败。

---

## 21. 尾置返回类型

模板中返回类型依赖参数表达式时，可以写：

```cpp
template <typename Left, typename Right>
auto add_values(const Left& left,
                const Right& right)
    -> decltype(left + right) {
    return left + right;
}
```

为什么 `decltype(left + right)`写在后面？

因为参数名 `left/right`在参数列表之后才可用于该返回类型表达式。

C++14之后许多简单场景可以直接：

```cpp
template <typename Left, typename Right>
auto add_values(const Left& left,
                const Right& right) {
    return left + right;
}
```

但尾置返回类型仍常见于模板和接口声明。

---

## 22. `decltype(auto)`只要求认识

```cpp
decltype(auto) access() {
    return expression;
}
```

它使用 `decltype`规则推导返回类型，可以保留引用。

危险点：

```cpp
decltype(auto) get() {
    int value = 10;
    return (value);
}
```

括号导致返回 `int&`，但 `value`在函数结束时销毁，结果悬空。

所以 `decltype(auto)`不是“更高级就更好”。今天正式项目接口不要求使用它。

---

## 23. 用类型特征验证推导

需要：

```cpp
#include <type_traits>
```

```cpp
static_assert(
    std::is_same_v<decltype(value), int>
);
```

验证引用：

```cpp
static_assert(
    std::is_same_v<
        decltype((value)),
        int&
    >
);
```

它比通过 `typeid().name()`输出类型更可靠，因为 `typeid`输出名称实现相关，而且引用/cv信息可能不按你期望展示。

---

## 24. 普通枚举的问题

传统枚举：

```cpp
enum AudioFormat {
    Wav,
    Mp3,
    Flac
};
```

枚举值进入外围作用域：

```cpp
AudioFormat format = Wav;
```

不同枚举的名称可能冲突，而且传统枚举可能隐式转换为整数。

现代代码优先考虑 `enum class`。

---

## 25. `enum class`

```cpp
enum class AudioFormat {
    wav,
    mp3,
    flac
};
```

使用时必须带作用域：

```cpp
AudioFormat format = AudioFormat::wav;
```

不能直接写：

```cpp
AudioFormat format = wav;
```

优点：

- 枚举值不污染外围作用域；
- 不同枚举可以有同名项；
- 不会随意隐式转成整数；
- 函数参数语义更清晰。

---

## 26. 不同 `enum class`不能混用

```cpp
enum class AudioFormat {
    wav,
    flac
};

enum class ProcessingState {
    pending,
    completed
};
```

即使内部数值相同，也不能比较：

```cpp
AudioFormat::wav
    == ProcessingState::pending; // 编译错误
```

这种限制能防止把语义完全不同的状态混在一起。

---

## 27. 指定底层类型

```cpp
#include <cstdint>

enum class AudioFormat : std::uint8_t {
    wav = 1,
    mp3 = 2,
    flac = 3
};
```

指定底层类型可能用于：

- 序列化协议；
- 文件格式；
- ABI或存储布局要求；
- 明确可表示范围。

普通业务枚举不一定必须指定。不要随意依赖枚举对象大小，除非协议有明确要求。

---

## 28. 枚举与整数转换

`enum class`不会隐式转整数：

```cpp
int value = AudioFormat::wav; // 编译错误
```

需要显式转换：

```cpp
const int value = static_cast<int>(
    AudioFormat::wav
);
```

整数转枚举也需显式：

```cpp
const auto format = static_cast<AudioFormat>(1);
```

但转换成功不代表数值一定对应合法枚举项。外部输入转换前应校验范围。

---

## 29. 枚举转字符串

C++17没有标准反射自动得到枚举名称。可以写函数：

```cpp
const char* to_string(AudioFormat format) {
    switch (format) {
        case AudioFormat::wav:
            return "wav";
        case AudioFormat::mp3:
            return "mp3";
        case AudioFormat::flac:
            return "flac";
    }

    return "unknown";
}
```

如果所有枚举值都覆盖，某些编译器可通过警告帮助发现新增枚举未处理。

不要为了消除警告随意写巨大 `default`吞掉未来枚举项；是否使用 `default`由错误策略决定。

---

## 30. `const`不一定表示编译期常量

```cpp
int read_value();

const int value = read_value();
```

`value`初始化后不能修改，但它来自运行时函数，因此不一定能用于编译期计算。

```text
const强调只读
constexpr强调可以成为常量表达式
```

---

## 31. `constexpr`变量

```cpp
constexpr int sample_rate = 16000;
constexpr int seconds = 2;
constexpr int sample_count =
    sample_rate * seconds;
```

这些可以在编译期计算。

`constexpr`变量也具有 `const`性质，初始化后不能修改。

必须使用常量表达式初始化：

```cpp
int runtime_value = read_value();
constexpr int invalid = runtime_value; // 编译错误
```

---

## 32. `const`与 `constexpr`对比

| 写法 | 初始化后可修改 | 一定可用于编译期计算 |
|---|---|---|
| `int` | 是 | 否 |
| `const int` | 否 | 不一定 |
| `constexpr int` | 否 | 满足常量表达式要求时是 |

例如：

```cpp
const int runtime_constant = read_value();
constexpr int compile_constant = 16000;
```

二者都不能重新赋值，但只有后者明确要求编译期可用。

---

## 33. `constexpr`函数

```cpp
constexpr int samples_for_seconds(
    int sample_rate,
    int seconds
) {
    return sample_rate * seconds;
}
```

编译期调用：

```cpp
constexpr int count =
    samples_for_seconds(16000, 2);
```

运行时调用也可以：

```cpp
int rate = read_value();
int count = samples_for_seconds(rate, 2);
```

因此：

> `constexpr`函数表示它在满足条件时可以用于编译期计算，不表示每次调用都必然在编译期执行。

---

## 34. 如何强制验证编译期计算

使用：

```cpp
static_assert(
    samples_for_seconds(16000, 2)
        == 32000
);
```

或者初始化 `constexpr`变量：

```cpp
constexpr int count =
    samples_for_seconds(16000, 2);
```

仅仅写：

```cpp
int count = samples_for_seconds(16000, 2);
```

不等于你强制要求编译器在编译期完成它。

---

## 35. C++17 `constexpr`函数可以包含什么

C++17比早期标准允许更多函数体逻辑，例如局部变量、条件和循环：

```cpp
constexpr int clamp_rate(int rate) {
    if (rate < 8000) {
        return 8000;
    }

    if (rate > 192000) {
        return 192000;
    }

    return rate;
}
```

但要在编译期执行，实际执行路径中的操作必须满足常量表达式要求。

动态分配等能力在不同标准版本有不同限制，今天不要在 `constexpr`函数中使用复杂运行时资源。

---

## 36. `constexpr`对象与构造函数基础

简单结构体可以有 `constexpr`构造函数：

```cpp
struct AudioWindow {
    int frame_size;
    int hop_size;

    constexpr AudioWindow(
        int frame,
        int hop
    )
        : frame_size(frame),
          hop_size(hop) {}
};
```

创建编译期对象：

```cpp
constexpr AudioWindow window(400, 160);
static_assert(window.frame_size == 400);
```

今天只处理简单字面值类型，不把包含复杂动态容器的服务类强行改成 `constexpr`。

---

## 37. `if constexpr`

C++17允许在模板中根据编译期条件选择分支：

```cpp
template <typename T>
constexpr const char* numeric_kind() {
    if constexpr (std::is_integral_v<T>) {
        return "integral";
    } else if constexpr (
        std::is_floating_point_v<T>
    ) {
        return "floating";
    } else {
        return "other";
    }
}
```

未选择分支不会以相同方式参与该实例的代码生成，可以处理不同类型拥有不同合法操作的情况。

普通 `if`是运行时分支；两个分支通常都必须能通过编译。

---

## 38. `if constexpr`不是普通优化提示

它要求条件是编译期可判断：

```cpp
if constexpr (std::is_integral_v<T>) {
}
```

不能使用任意运行时变量：

```cpp
int value = read_value();

if constexpr (value > 0) { // 编译错误
}
```

运行时输入继续使用普通 `if`。

---

## 39. `constexpr`与模板参数

非类型模板参数要求编译期值：

```cpp
constexpr std::size_t capacity = 128;
FixedBuffer<float, capacity> buffer;
```

运行时变量不能作为模板实参：

```cpp
std::size_t capacity = read_value();
FixedBuffer<float, capacity> buffer; // 错误
```

这与昨天的非类型模板参数相连接。

---

## 40. 常见错误

### 错误1：以为 `auto`是动态类型

类型在编译期确定。

### 错误2：忘记 `auto`按值会去掉顶层 `const`

需要保留只读引用时写 `const auto&`。

### 错误3：范围 `for`写 `auto`却以为修改了容器

它修改的是副本。

### 错误4：`auto text = "audio"`以为得到 `std::string`

实际通常是 `const char*`。

### 错误5：混淆 `auto{1}`和 `auto = {1}`

后者通常涉及 `initializer_list`。

### 错误6：混淆 `decltype(value)`和 `decltype((value))`

额外括号可能让结果变成引用。

### 错误7：使用 `decltype(auto)`返回局部变量引用

产生悬空引用。

### 错误8：传统枚举值污染作用域

现代接口优先 `enum class`。

### 错误9：以为 `enum class`自动转整数

需要显式转换。

### 错误10：把任意整数转枚举后直接使用

转换不验证枚举值合法性。

### 错误11：认为所有 `const`都是编译期常量

运行时初始化的 `const`不一定可用于常量表达式。

### 错误12：认为 `constexpr`函数每次都在编译期执行

运行时参数会产生运行时求值。

### 错误13：使用普通 `if`代替需要丢弃非法分支的 `if constexpr`

模板两个分支可能都被检查并报错。

### 错误14：滥用 `auto`隐藏关键精度和所有权语义

重要业务类型可以显式书写。

---

## 41. 练习1：`auto`基础推导

对应文件：`src/01_auto_basics.cpp`

声明并用 `static_assert(std::is_same_v<...>)`验证：

```text
整数
double
float
bool
字符
字符串字面量
std::string
vector迭代器
Lambda对象
```

要求：

- 说明字符串字面量的推导结果；
- 不依赖 `typeid().name()`判断引用和 `const`；
- 测试 `auto first{1}`与 `auto second={1}`；
- 混合花括号推导失败案例只保留注释。

---

## 42. 练习2：`auto`、`const`和引用

对应文件：`src/02_auto_cv_reference.cpp`

验证：

```text
const int -> auto得到int
const int -> auto&得到const int&
int -> auto&得到int&
const int* -> auto保留指向const
```

再对 `vector<AudioRecord>`分别使用：

```cpp
auto
auto&
const auto&
```

要求：

- 证明副本修改不影响容器；
- 证明引用修改影响容器；
- 只读引用不能修改；
- 记录容器结构变化可能导致引用失效。

---

## 43. 练习3：`decltype`规则

对应文件：`src/03_decltype_rules.cpp`

使用 `static_assert`验证：

```cpp
decltype(value)
decltype((value))
decltype(const_value)
decltype((const_value))
decltype(value + 1)
decltype(*pointer)
```

要求写出每个结果：

```text
T
T&
const T
const T&
```

并通过一个引用修改实验验证 `decltype((value))`确实绑定原对象。

---

## 44. 练习4：返回类型推导

对应文件：`src/04_auto_return_type.cpp`

完成：

- 普通 `auto`返回；
- 两类型模板相加；
- 尾置返回类型 `-> decltype(left + right)`；
- 对比 `int+double`结果；
- 观察普通 `auto`返回如何按值；
- 只在注释中分析 `decltype(auto)`悬空返回案例。

要求不能从函数返回局部对象引用。

---

## 45. 练习5：`enum class`基础

对应文件：`src/05_enum_class_basics.cpp`

定义：

```text
AudioFormat：wav、flac、mp3、unknown
ProcessingState：pending、running、completed、failed
```

要求：

- 使用作用域名称；
- 两个枚举可以拥有同名枚举项；
- 编写 `to_string()`；
- 使用 `switch`覆盖全部状态；
- 不允许不同枚举相互比较；
- 测试所有枚举值输出。

---

## 46. 练习6：枚举转换与校验

对应文件：`src/06_enum_conversion.cpp`

为 `AudioFormat`指定 `std::uint8_t`底层类型。

完成：

- 枚举显式转整数；
- 合法整数转枚举；
- 非法整数输入先校验；
- 未知值返回明确错误；
- 不使用C风格强制转换；
- 说明显式转换不等于合法性验证。

---

## 47. 练习7：`constexpr`变量

对应文件：`src/07_constexpr_variables.cpp`

定义：

```text
sample_rate=16000
frame_duration_ms=25
hop_duration_ms=10
channels=1
```

在编译期计算：

```text
每帧样本数
每个hop样本数
2秒音频样本数
字节数（假设每样本2字节）
```

要求：

- 使用 `static_assert`验证；
- 避免整数除法顺序造成错误；
- 检查乘法类型是否可能溢出；
- 对比运行时初始化的 `const`。

---

## 48. 练习8：`constexpr`函数

对应文件：`src/08_constexpr_functions.cpp`

在 `include/audio_math.hpp`中实现：

```text
milliseconds_to_samples
samples_to_milliseconds
clamp_sample_rate
is_supported_sample_rate
bytes_for_pcm
```

要求：

- 合法输入用 `static_assert`验证；
- 同一函数也使用运行时变量调用；
- 明确整数和浮点返回类型；
- 非法除数不允许；
- 检查字节数乘法溢出风险；
- 不在 `constexpr`函数中执行I/O。

---

## 49. 练习9：`if constexpr`

对应文件：`src/09_if_constexpr.cpp`

实现数值类型说明和归一化辅助模板：

```text
整数类型 -> integral
浮点类型 -> floating
其他类型 -> other
```

要求：

- 使用 `std::is_integral_v`；
- 使用 `std::is_floating_point_v`；
- 测试 `int/float/double/std::string`；
- 解释未选择分支的处理；
- 对比普通运行时 `if`；
- 不扩展到SFINAE或Concepts。

---

## 50. 练习10：类型安全的音频处理配置

对应文件：

- 类型定义：`include/audio_types.hpp`
- 编译期计算：`include/audio_math.hpp`
- 配置声明/实现：`include/audio_processing_config.hpp`
- 演示：`src/10_audio_processing_config.cpp`
- 单元测试：`tests/modern_type_tests.cpp`
- 实验报告：`target/report/auto_decltype_constexpr_report.md`

### 项目定位

实现一个纯软件的音频处理配置与计算工具，不访问硬件、不采集或解码音频。

### 类型安全枚举

至少定义：

```text
AudioFormat
ProcessingState
SampleEncoding
ErrorPolicy
```

要求所有枚举使用 `enum class`。

### 编译期常量

至少定义：

```text
默认采样率
默认帧长
默认hop长度
支持的最小/最大采样率
质量分数边界
```

### 配置字段

```text
format
encoding
sample_rate
channels
frame_duration_ms
hop_duration_ms
error_policy
```

### 必须实现

```text
validate
frame_samples
hop_samples
bytes_per_sample
estimated_pcm_bytes
format_name
encoding_name
state_name
```

### 技术要求

- 能在编译期计算的纯函数声明为 `constexpr`；
- 用 `static_assert`验证默认配置相关结果；
- 运行时配置继续使用普通校验；
- 使用 `enum class`避免状态混用；
- 局部迭代器等复杂类型使用 `auto`；
- 关键数值类型不要全部隐藏在 `auto`后；
- 用 `decltype`测试推导，不为炫技写复杂返回类型；
- 不使用拥有型裸 `new/delete`。

---

## 51. 正式测试清单

文件：`tests/modern_type_tests.cpp`

至少覆盖：

```text
1. auto从int推导int
2. auto从double推导double
3. auto字符串字面量不是std::string
4. auto按值去掉顶层const
5. auto&保留引用
6. auto&从const对象推导const引用
7. const int*经auto仍指向const
8. 范围for使用auto产生副本
9. 范围for使用auto&修改原元素
10. const auto&只读访问
11. auto{1}类型正确
12. auto={1}为initializer_list
13. decltype(value)为声明类型
14. decltype((value))为左值引用
15. decltype(const_value)保留const
16. decltype((const_value))为const引用
17. decltype(value+1)为值类型
18. decltype(*pointer)为引用类型
19. auto返回类型正确
20. 尾置返回类型处理int+double
21. AudioFormat作用域正确
22. 不同枚举具有独立类型
23. AudioFormat转字符串正确
24. ProcessingState转字符串正确
25. 枚举转底层整数正确
26. 合法整数可安全转换枚举
27. 非法整数被拒绝
28. constexpr样本数计算正确
29. constexpr帧长计算正确
30. constexpr PCM字节数正确
31. constexpr函数可被static_assert使用
32. constexpr函数可接受运行时参数
33. const运行时值不被误作constexpr
34. if constexpr识别整数
35. if constexpr识别float
36. if constexpr识别double
37. if constexpr处理其他类型
38. 默认音频配置合法
39. 非正采样率被拒绝
40. 非正声道数被拒绝
41. 非正帧长被拒绝
42. hop大于frame时按规则处理
43. frame_samples正确
44. hop_samples正确
45. 每样本字节数正确
46. PCM估算字节数正确
47. 未知枚举输入有明确错误语义
48. 正式测试ASan零错误
49. 正式测试UBSan零错误
```

---

## 52. 编译命令

### 单文件练习

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
  src/01_auto_basics.cpp \
  -Iinclude \
  -o target/01_auto_basics
```

### 综合项目

如果实现均放在头文件中：

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
  src/10_audio_processing_config.cpp \
  -Iinclude \
  -o target/10_audio_processing_config
```

运行：

```bash
./target/10_audio_processing_config
```

---

## 53. 编译期测试与运行时测试

### 编译期测试

```cpp
static_assert(
    milliseconds_to_samples(16000, 25)
        == 400
);
```

编译失败就表示条件不成立。

### 运行时测试

```cpp
const int rate = read_runtime_rate();
const auto samples =
    milliseconds_to_samples(rate, 25);

assert(samples == expected);
```

同一个 `constexpr`函数应该同时覆盖这两类调用。

---

## 54. Sanitizer边界

ASan/UBSan可能发现：

- 悬空引用；
- 越界；
- 部分整数未定义行为；
- 错误转换后引发的运行时访问。

不能发现或不能代替：

- `auto`隐藏了错误数值精度；
- `decltype`推导不是你期望的类型；
- 枚举业务值是否合法；
- `const`与 `constexpr`设计选择错误；
- 某个函数实际是否在编译期计算；
- 整数除法产生逻辑精度损失。

因此今天大量使用：

```text
static_assert
std::is_same_v
运行时单元测试
ASan/UBSan
```

---

## 55. 实验报告模板

文件：`target/report/auto_decltype_constexpr_report.md`

```markdown
# auto、decltype、enum class与constexpr实验报告

## 环境
- 编译器：
- C++标准：C++17
- 编译参数：

## auto
- 基础推导：
- 顶层const：
- 底层const：
- auto&：
- const auto&：
- 范围for：
- 花括号推导：

## decltype
- decltype(name)：
- decltype((name))：
- 左值表达式：
- 纯右值表达式：
- 尾置返回类型：

## enum class
- 定义的枚举：
- 底层类型：
- 转字符串：
- 整数转换校验：

## constexpr
- const与constexpr：
- 编译期函数：
- 运行时调用：
- static_assert：
- if constexpr：

## 综合项目
- 类型安全设计：
- 编译期计算：
- 运行时校验：
- 已知边界：

## Sanitizer
- ASan：
- UBSan：

## 今日结论
```

---

## 56. 今日思考题

1. `auto`是否意味着动态类型？
2. 为什么 `auto value;`不能推导？
3. `auto text="audio"`是什么类型？
4. `auto`按值如何处理顶层 `const`？
5. `auto`如何处理指针指向对象的 `const`？
6. `auto&`和普通 `auto`有什么区别？
7. 范围 `for`何时使用 `const auto&`？
8. `auto{1}`与 `auto={1}`有什么差别？
9. 哪些场景不应滥用 `auto`？
10. `decltype(value)`返回什么？
11. `decltype((value))`为什么可能是引用？
12. `decltype(value+1)`为什么通常不是引用？
13. `decltype`中的普通表达式会执行吗？
14. 尾置返回类型解决什么问题？
15. `decltype(auto)`最大的生命周期风险是什么？
16. 为什么优先使用 `enum class`？
17. 两个不同 `enum class`能直接比较吗？
18. 为什么枚举不能随意隐式转整数？
19. 整数显式转枚举后一定合法吗？
20. C++17如何把枚举转为名称？
21. `const`和 `constexpr`最核心区别是什么？
22. 运行时函数初始化的 `const`能否用于编译期数组长度？
23. `constexpr`函数是否每次都编译期执行？
24. 如何强制验证编译期结果？
25. `static_assert`什么时候执行？
26. `if constexpr`和普通 `if`有什么区别？
27. `if constexpr`条件能否使用运行时变量？
28. `constexpr`如何与非类型模板参数配合？
29. 为什么关键精度类型不应全部写成 `auto`？
30. 这些工具如何改善音频处理配置的类型安全？

---

## 57. 今日验收清单

### 知识

- [ ] 能解释 `auto`是静态推导；
- [ ] 能判断顶层和底层 `const`；
- [ ] 能正确选择值、引用和只读引用；
- [ ] 能解释花括号推导差异；
- [ ] 能判断常见 `decltype`结果；
- [ ] 能解释额外括号为什么改变类型；
- [ ] 能使用 `enum class`；
- [ ] 能安全转换枚举；
- [ ] 能区分 `const`和 `constexpr`；
- [ ] 能编写简单 `constexpr`函数；
- [ ] 能使用 `if constexpr`。

### 编码

- [ ] 完成 `01_auto_basics.cpp`；
- [ ] 完成 `02_auto_cv_reference.cpp`；
- [ ] 完成 `03_decltype_rules.cpp`；
- [ ] 完成 `04_auto_return_type.cpp`；
- [ ] 完成 `05_enum_class_basics.cpp`；
- [ ] 完成 `06_enum_conversion.cpp`；
- [ ] 完成 `07_constexpr_variables.cpp`；
- [ ] 完成 `08_constexpr_functions.cpp`；
- [ ] 完成 `09_if_constexpr.cpp`；
- [ ] 完成音频处理配置项目。

### 验证与输出

- [ ] 至少49项测试通过；
- [ ] 使用 `std::is_same_v`验证推导；
- [ ] 使用 `static_assert`验证编译期结果；
- [ ] 运行时路径也完成测试；
- [ ] ASan/UBSan零错误；
- [ ] 完成实验报告；
- [ ] 完成Git提交。

---

## 58. 与就业方向的联系

### AI部署与推理工程

这些工具常见于：

- Tensor数据类型推导；
- 模板API返回类型；
- 推理精度枚举；
- 状态机；
- 编译期维度和缓冲参数；
- CUDA和性能库中的模板分支；
- 配置校验；
- ONNX Runtime/TensorRT枚举接口。

### C++工程岗位

常见问题：

- `auto`如何处理 `const/reference`；
- `decltype(x)`和 `decltype((x))`；
- `decltype(auto)`风险；
- `enum class`优势；
- `const`和 `constexpr`；
- `constexpr`函数是否一定编译期执行；
- `if constexpr`用途。

---

## 59. 今日产出

```text
9个专项练习
1个类型安全音频处理配置项目
1个测试文件
1份类型推导与编译期计算报告
1次可复现Git提交
```

---

## 60. Git提交建议

```bash
git status
git add day25
git commit -m "Learn type deduction scoped enums and constexpr"
```

提交前检查：

- 编译失败案例只保留注释或报告；
- 不提交二进制文件；
- 测试覆盖编译期与运行时；
- 不滥用 `auto`隐藏关键类型；
- 枚举转换先验证输入；
- 编译命令可复现。

---

## 61. 最终速记

```text
auto：编译期根据初始化表达式推导静态类型

auto按值：通常去掉顶层const和引用
auto&：保留引用关系
const auto&：只读且避免复制

decltype(name)：名字的声明类型
decltype((name))：按表达式值类别，左值通常得到T&

enum class：有作用域、强类型、不随意隐式转整数

const：初始化后只读，但不一定编译期可用
constexpr：要求可以构成常量表达式

constexpr函数：满足条件时可编译期计算，也可运行时调用

static_assert：编译期验证

if constexpr：编译期选择模板分支
```
