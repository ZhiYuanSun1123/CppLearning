# 第6周·工作日3：理解 `optional`、`variant`和错误返回设计

## 0. 今天要解决的问题

函数并不总能返回一个正常结果：

```text
查询可能找不到
输入可能非法
解析可能失败
同一个字段可能有多种合法类型
底层操作可能抛出异常
```

今天重点区分：

```text
没有值，但属于正常业务情况
    -> optional

结果可能属于多种合法类型
    -> variant

成功值或结构化错误二选一
    -> variant<Value, Error>
```

今天结束时，你应该能够：

1. 正确构造、检查和读取 `std::optional`；
2. 避免空 `optional`访问和悬空引用；
3. 使用 `std::variant`保存互斥类型；
4. 使用 `holds_alternative/get/get_if/visit`；
5. 解释 `monostate`和默认构造；
6. 根据场景选择 `optional`、`variant`、错误码或异常；
7. 设计 `variant<Success, Error>`返回接口；
8. 完成音频元数据解析器和错误传播测试。

---

## 1. 前置知识与超纲说明

### 今天必须掌握

- `std::optional<T>`表示“有一个 `T`或没有值”；
- `std::nullopt`；
- `has_value()`和 `operator bool`；
- `value()`、`operator*`、`operator->`、`value_or()`；
- `reset()`和 `emplace()`；
- `optional`保存对象而不是自动保存指针；
- `std::variant<Ts...>`任一时刻只保存一个候选类型；
- `index()`、`holds_alternative()`；
- `get()`、`get_if()`和 `visit()`；
- `std::monostate`；
- `std::bad_optional_access`和 `std::bad_variant_access`；
- `optional`与错误信息的边界；
- `variant<Value, Error>`结果设计；
- 错误传播和调用方必须处理结果。

### 今天只要求认识

- `optional<std::reference_wrapper<T>>`；
- `variant`重复候选类型；
- `valueless_by_exception()`；
- 多 `variant`访问；
- overloaded visitor辅助模板；
- monadic操作；
- `std::expected`；
- 错误类别与 `std::error_code`；
- 异常安全保证分级。

### C++版本提醒

继续使用：

```bash
-std=c++17
```

`std::expected`是更新标准中的工具，不属于 C++17标准库。今天使用：

```cpp
std::variant<Value, Error>
```

表达成功或失败。

---

## 2. 今日目录结构

```text
day26/
├── include/
│   ├── audio_metadata.hpp
│   ├── parse_error.hpp
│   ├── parse_result.hpp
│   └── audio_metadata_parser.hpp
├── src/
│   ├── 01_optional_basics.cpp
│   ├── 02_optional_lookup.cpp
│   ├── 03_variant_basics.cpp
│   ├── 04_variant_access.cpp
│   ├── 05_monostate_state.cpp
│   ├── 06_error_return_comparison.cpp
│   ├── 07_value_or_error.cpp
│   ├── 08_error_propagation.cpp
│   ├── 09_batch_parse_summary.cpp
│   ├── 10_audio_metadata_parser.cpp
│   └── audio_metadata_parser.cpp
├── tests/
│   └── optional_variant_tests.cpp
├── target/
│   └── report/
│       └── error_return_design.md
└── 第6周_工作日3_optional_variant与错误返回设计.md
```

所有练习均标注文件名。你自己完成源码，文档不直接给出完整项目答案。

---

## 3. 建议时间

```text
60分钟：optional
75分钟：variant
60分钟：错误返回方案比较
120分钟：练习1～9
90分钟：元数据解析器综合项目
45分钟：测试、错误覆盖和报告
```

优先顺序：

```text
optional空值检查
> variant安全访问
> 正常缺失与错误区别
> Value/Error结果设计
> 错误传播
> 综合项目
```

---

## 4. 为什么不总是返回特殊值

传统函数可能写：

```cpp
int find_sample_rate(const std::string& path) {
    if (not_found) {
        return -1;
    }

    return sample_rate;
}
```

问题：

- `-1`是业务数据还是错误？
- 调用者可能忘记检查；
- 返回类型没有表达“可能没有值”；
- 只能表达一种失败，没有详细原因。

`optional<int>`可以直接表达：

```text
有采样率
或
没有找到
```

---

## 5. `std::optional<T>`是什么

头文件：

```cpp
#include <optional>
```

定义：

```cpp
std::optional<int> sample_rate;
```

它有两种状态：

```text
engaged    -> 内部有一个int对象
disengaged -> 内部没有int对象
```

空状态不是包含一个“空的int”，而是当前没有构造 `T`对象。

---

## 6. 构造有值和无值状态

无值：

```cpp
std::optional<int> first;
std::optional<int> second = std::nullopt;
```

有值：

```cpp
std::optional<int> third = 16000;
std::optional<int> fourth{48000};
```

函数返回空值：

```cpp
return std::nullopt;
```

函数返回实际值时可以直接：

```cpp
return 16000;
```

编译器构造 `optional<int>`。

---

## 7. 检查是否有值

### `has_value()`

```cpp
if (result.has_value()) {
}
```

### 布尔上下文

```cpp
if (result) {
}
```

两者都表达是否包含值。

不要比较内部数值来猜空状态：

```cpp
if (*result != 0) {
}
```

首先必须确定 `result`有值，而且0可能是合法值。

---

## 8. `value()`

```cpp
const int rate = result.value();
```

有值时返回内部对象；无值时抛出：

```cpp
std::bad_optional_access
```

安全写法：

```cpp
if (result) {
    std::cout << result.value() << '\n';
}
```

如果空值在当前逻辑中属于程序错误，也可以允许异常传播，但接口和测试必须明确。

---

## 9. `operator*`和 `operator->`

```cpp
std::optional<AudioMetadata> result;
```

访问对象：

```cpp
const AudioMetadata& metadata = *result;
```

访问成员：

```cpp
std::cout << result->path << '\n';
```

它们不会像 `value()`那样在空状态抛出明确的 `bad_optional_access`。空状态解引用属于未定义行为。

因此必须先检查：

```cpp
if (result) {
    std::cout << result->path << '\n';
}
```

---

## 10. `value_or()`

```cpp
const int rate = result.value_or(16000);
```

有值：返回内部值；无值：返回默认值。

适合默认值语义真实存在的情况。

不适合掩盖错误：

```cpp
const int rate = parse_rate(text).value_or(0);
```

如果解析失败需要报告原因，默认成0会丢失错误信息。

另外，传给 `value_or()`的默认表达式在调用前就会求值，不要假设只有空状态才构造昂贵默认值。

---

## 11. `reset()`与 `emplace()`

清空：

```cpp
result.reset();
```

内部对象若存在会被析构，`optional`变为空。

原地构造：

```cpp
result.emplace(/* T的构造参数 */);
```

如果原来有值，旧对象先被销毁，再构造新对象。

不要在保存内部引用后调用 `reset/emplace`：

```cpp
const auto& reference = *result;
result.reset();
// reference悬空
```

---

## 12. `optional`不是指针

```cpp
std::optional<AudioMetadata>
```

通常直接在自身存储空间中管理一个可选 `AudioMetadata`对象，不要求堆分配。

对比：

```text
optional<T>
    可选拥有一个T值

T*
    指向某个外部T，通常不表达所有权

unique_ptr<T>
    可选拥有一个动态分配的T
```

不能因为都能表示空状态就认为语义相同。

---

## 13. `optional`不能直接保存普通引用

不能写：

```cpp
std::optional<AudioMetadata&>
```

如果想可选地引用外部对象，可以考虑：

```cpp
std::optional<
    std::reference_wrapper<const AudioMetadata>
>
```

需要 `<functional>`。

但它仍不拥有对象，外部对象销毁后引用悬空。今天正式项目优先返回值或普通只读指针，不引入该复杂接口。

---

## 14. `optional`适合什么

适合：

- 查找不存在是正常情况；
- 一个配置字段可以缺省；
- 计算结果可能没有定义，例如空集合没有最大值；
- 调用方只需要区分“有/无”，不需要失败原因。

不适合：

- 需要区分多种失败原因；
- 需要错误位置和输入内容；
- 失败必须携带恢复信息；
- 多个合法返回类型之间切换。

---

## 15. `std::variant`是什么

头文件：

```cpp
#include <variant>
```

```cpp
std::variant<int, double, std::string> value;
```

任一时刻只保存候选类型中的一个：

```text
int
或 double
或 std::string
```

它是类型安全的联合体，不需要手动记录当前有效类型和强制转换裸内存。

---

## 16. 默认构造保存第一个候选类型

```cpp
std::variant<int, std::string> value;
```

默认构造时通常构造第一个候选 `int`，值为0。

这不表示“什么都没有”。

如果需要明确的未初始化状态，可以把：

```cpp
std::monostate
```

放在第一项：

```cpp
std::variant<
    std::monostate,
    int,
    std::string
> value;
```

---

## 17. 给 `variant`赋不同类型

```cpp
std::variant<int, std::string> value = 42;
```

当前保存 `int`。

```cpp
value = std::string("audio");
```

原来的 `int`状态被替换，当前保存 `std::string`。

`variant`负责构造和析构当前候选对象。

---

## 18. `index()`

```cpp
std::variant<int, double, std::string> value;
```

候选索引：

```text
int         -> 0
double      -> 1
std::string -> 2
```

```cpp
const std::size_t active = value.index();
```

索引依赖候选顺序，业务逻辑中通常优先使用类型检查而不是魔法索引数字。

---

## 19. `holds_alternative<T>()`

```cpp
if (std::holds_alternative<int>(value)) {
}
```

判断当前是否保存指定类型。

如果候选列表中同一类型重复，按类型访问可能产生歧义或无法使用，应改用索引访问。初学项目避免重复候选类型。

---

## 20. `std::get<T>()`

```cpp
const int number = std::get<int>(value);
```

当前确实是 `int`时成功；否则抛出：

```cpp
std::bad_variant_access
```

可以先检查：

```cpp
if (std::holds_alternative<int>(value)) {
    std::cout << std::get<int>(value);
}
```

也可以按索引：

```cpp
std::get<0>(value)
```

业务代码通常按类型更清晰。

---

## 21. `std::get_if<T>()`

```cpp
if (const auto* number =
        std::get_if<int>(&value)) {
    std::cout << *number << '\n';
}
```

类型匹配返回内部对象指针，不匹配返回 `nullptr`，不会抛 `bad_variant_access`。

注意：

- 传入的是 `variant`地址；
- 指针不拥有内部对象；
- `variant`切换类型、销毁后，旧指针失效；
- 不要长期保存该指针。

---

## 22. `std::visit()`

```cpp
std::visit(
    [](const auto& active_value) {
        std::cout << active_value << '\n';
    },
    value
);
```

访问器会接收当前有效候选。

泛型 Lambda必须对所有候选类型形成合法调用。如果某个候选不能执行函数体中的操作，编译可能失败。

复杂业务可使用 `if constexpr`按类型处理：

```cpp
std::visit(
    [](const auto& active) {
        using Active =
            std::decay_t<decltype(active)>;

        if constexpr (
            std::is_same_v<Active, int>
        ) {
            // int处理
        } else {
            // 其他类型处理
        }
    },
    value
);
```

`std::decay_t`属于类型特征辅助，今天会用即可，不深入规则。

---

## 23. `std::monostate`

`std::monostate`是一个简单空类型，经常作为第一个候选，让 `variant`可表示“尚未设置”：

```cpp
using ProcessingValue = std::variant<
    std::monostate,
    int,
    std::string
>;
```

默认构造后：

```cpp
std::holds_alternative<std::monostate>(value)
```

为真。

它是一个真实候选类型，不等于 `variant`内部完全不存在对象。

---

## 24. `valueless_by_exception()`只要求认识

极少数情况下，切换候选时构造新值抛异常，`variant`可能进入无值状态：

```cpp
value.valueless_by_exception()
```

此时：

```cpp
value.index() == std::variant_npos
```

对于今天使用的简单数值、字符串和错误结构，该状态不应成为主要设计路径，但需要知道它与 `monostate`不同：

```text
monostate是主动设计的合法候选
valueless_by_exception是异常切换导致的特殊状态
```

---

## 25. `optional`与 `variant`如何选择

### 只需要“有或没有”

```cpp
std::optional<AudioMetadata>
```

### 有多种正常类型

```cpp
std::variant<int, double, std::string>
```

### 成功值或详细错误

```cpp
std::variant<AudioMetadata, ParseError>
```

### 无值也要区分多种错误

不要只返回空 `optional`，使用结构化错误结果。

---

## 26. `bool + 输出参数`

传统接口：

```cpp
bool parse_metadata(
    const std::string& text,
    AudioMetadata& output
);
```

优点：

- 简单；
- 兼容旧接口；
- 避免某些返回值设计限制。

问题：

- 输出参数在失败时处于什么状态？
- 调用者可能误用失败后的 `output`；
- 错误原因丢失；
- 参数方向不够直观；
- 多个输出参数更复杂。

今天需要对比，但正式解析器不采用它作为主要接口。

---

## 27. 特殊错误码

```cpp
int parse_rate(const std::string& text) {
    return -1;
}
```

问题是值域冲突和错误信息不足。

枚举错误码更清晰：

```cpp
enum class ParseErrorCode {
    empty_input,
    wrong_field_count,
    invalid_number,
    out_of_range
};
```

但如果函数只返回错误码，成功值仍需要输出参数。可以把成功值和错误对象放进 `variant`。

---

## 28. 异常

```cpp
AudioMetadata parse_or_throw(
    const std::string& text
);
```

失败时：

```cpp
throw ParseException(...);
```

异常适合：

- 当前层无法合理恢复；
- 构造函数无法返回错误值；
- 失败属于异常路径；
- 调用链希望集中处理错误；
- 与使用异常的库边界一致。

不适合机械用于每个正常的“查找不存在”。异常不是绝对好或坏，需要根据失败频率、恢复位置和项目约定选择。

---

## 29. `variant<Value, Error>`

```cpp
using ParseResult = std::variant<
    AudioMetadata,
    ParseError
>;
```

成功：

```cpp
return metadata;
```

失败：

```cpp
return ParseError{
    ParseErrorCode::invalid_number,
    "sample_rate",
    "sample rate is not an integer"
};
```

调用方必须判断当前候选：

```cpp
if (const auto* metadata =
        std::get_if<AudioMetadata>(&result)) {
    // 成功
} else {
    const auto& error =
        std::get<ParseError>(result);
    // 失败
}
```

---

## 30. 错误对象应包含什么

建议：

```cpp
struct ParseError {
    ParseErrorCode code;
    std::string field;
    std::string message;
    std::size_t position;
};
```

根据实际需求选择：

- 稳定错误码用于程序判断；
- 字段名用于定位；
- 消息用于人类阅读；
- 行号或位置用于批量输入。

不要让调用方通过解析错误消息字符串判断错误种类，应使用 `code`。

---

## 31. 正常缺失不等于错误

索引查询：

```cpp
std::optional<AudioMetadata>
find_by_path(const std::string& path);
```

路径不存在可能是正常业务结果。

解析输入：

```cpp
ParseResult parse_line(const std::string& line);
```

字段数错误、数值非法属于需要报告原因的失败。

不要把所有情况统一成：

```cpp
return std::nullopt;
```

否则调用方无法知道：

```text
是正常不存在
还是输入损坏
还是程序错误
```

---

## 32. 错误传播

假设先解析整数：

```cpp
using IntResult = std::variant<int, ParseError>;
```

上层解析元数据时：

```text
调用parse_integer
    ↓ 成功
取得int并继续

    ↓ 失败
直接把ParseError返回给上层
```

不能在失败后继续使用一个默认的0，除非0明确是业务默认值。

错误传播的基本原则：

- 当前层能补充上下文就补充；
- 不能处理就原样或增强后返回；
- 不吞掉错误；
- 不把错误静默转换成看似成功的数据。

---

## 33. 批量处理的失败策略

批量解析多行时需要明确：

### 遇错停止

```text
第一条错误立即返回
```

适合必须全量一致成功的输入。

### 收集全部结果

```cpp
std::vector<ParseResult>
```

每一行都有成功或失败。

### 分别收集

```text
vector<AudioMetadata> successes
vector<ParseError> errors
```

适合离线数据清洗和报告。

综合项目采用“分别收集”，既保留成功数据又保留全部错误。

---

## 34. 调用方必须处理结果

错误设计不仅是返回类型，还包括调用方纪律。

危险写法：

```cpp
auto result = parse_line(line);
auto metadata = std::get<AudioMetadata>(result);
```

如果失败就抛 `bad_variant_access`，而且错误上下文未被正常处理。

更安全：

```cpp
if (const auto* metadata =
        std::get_if<AudioMetadata>(&result)) {
    // 使用成功值
} else if (const auto* error =
               std::get_if<ParseError>(&result)) {
    // 记录错误
}
```

---

## 35. `optional`嵌套和过度包装

```cpp
std::optional<std::optional<int>>
```

可能表达三种状态，但通常可读性差：

```text
外层无值
外层有值但内层无值
内外都有值
```

如果状态有不同业务含义，更适合：

```cpp
std::variant<StateA, StateB, int>
```

或者明确定义枚举和结构体。

不要为了“类型高级”堆叠包装器。

---

## 36. 常见错误

### 错误1：不检查就解引用空 `optional`

`operator*`不会替你安全返回默认值。

### 错误2：用 `value_or()`吞掉需要报告的错误

默认值必须有真实业务语义。

### 错误3：认为 `optional<T>`一定动态分配

它通常直接管理可选对象状态。

### 错误4：保存内部引用后 `reset/emplace`

旧引用失效。

### 错误5：认为默认构造 `variant`为空

它默认构造第一候选。

### 错误6：错误类型调用 `std::get`

会抛 `bad_variant_access`。

### 错误7：长期保存 `get_if`返回指针

切换候选后指针失效。

### 错误8：访问器只对一个候选类型可编译

`visit`需要处理全部可能候选。

### 错误9：把 `monostate`和异常无值状态混为一谈

前者是合法候选，后者是特殊异常状态。

### 错误10：所有失败都返回 `nullopt`

会丢失错误原因。

### 错误11：通过错误消息文字判断错误类型

应使用稳定错误码。

### 错误12：捕获异常后什么都不做

吞异常会让上层误以为成功。

### 错误13：批量处理中没有明确遇错策略

可能丢数据或部分成功却不报告。

### 错误14：把正常查询不存在当成异常崩溃

正常缺失可用 `optional`表达。

---

## 37. 练习1：`optional`基础

对应文件：`src/01_optional_basics.cpp`

完成：

- 默认构造空 `optional<int>`；
- 使用 `nullopt`；
- 构造有值状态；
- 使用 `has_value()`和布尔判断；
- 使用 `value()`；
- 使用 `operator*`；
- 使用 `value_or()`；
- `reset()`后验证为空；
- `emplace()`后验证新值；
- 捕获一次 `bad_optional_access`。

正式代码不能解引用空状态制造未定义行为。

---

## 38. 练习2：可选查询结果

对应文件：`src/02_optional_lookup.cpp`

创建路径到元数据的索引，提供：

```cpp
std::optional<AudioMetadata>
find_metadata(const std::string& path);
```

要求：

- 存在路径返回副本；
- 不存在返回 `nullopt`；
- 查询不使用 `operator[]`插入；
- 调用方处理有值和无值；
- 返回副本修改不影响索引；
- 解释这里为什么不需要详细错误。

---

## 39. 练习3：`variant`基础

对应文件：`src/03_variant_basics.cpp`

定义：

```cpp
std::variant<int, double, std::string>
```

要求：

- 分别保存三种类型；
- 每次输出 `index()`；
- 使用 `holds_alternative()`；
- 使用正确的 `get<T>()`；
- 捕获一次错误类型的 `bad_variant_access`；
- 说明赋新候选时旧对象生命周期结束。

---

## 40. 练习4：安全访问 `variant`

对应文件：`src/04_variant_access.cpp`

完成：

- 使用 `get_if<int>()`；
- 不匹配时验证返回 `nullptr`；
- 使用 `visit()`统一打印；
- 使用 `if constexpr`为字符串和数值提供不同逻辑；
- 不长期保存内部指针；
- 切换候选后重新调用 `get_if()`。

---

## 41. 练习5：`monostate`状态

对应文件：`src/05_monostate_state.cpp`

定义：

```cpp
std::variant<
    std::monostate,
    AudioMetadata,
    ParseError
>
```

要求：

- 默认状态是 `monostate`；
- 切换到元数据；
- 切换到错误；
- 使用 `visit()`处理三个候选；
- 区分 `monostate`和 `valueless_by_exception()`；
- 不用索引魔法数字决定业务逻辑。

---

## 42. 练习6：错误返回方案比较

对应文件：`src/06_error_return_comparison.cpp`

对“把字符串解析为采样率”分别设计：

```text
特殊值-1
bool + 输出参数
optional<int>
variant<int, ParseError>
抛异常
```

要求记录：

- 成功调用方式；
- 失败调用方式；
- 是否携带错误原因；
- 是否可能误用输出；
- 是否适合高频预期失败；
- API可读性；
- 最终推荐及理由。

不要求得出“任何项目永远只能选一种”的结论。

---

## 43. 练习7：成功值或错误

对应文件：`src/07_value_or_error.cpp`

定义：

```cpp
enum class ParseErrorCode;
struct ParseError;
using IntResult =
    std::variant<int, ParseError>;
```

实现整数解析，至少区分：

```text
空文本
包含非数字
数值超范围
成功
```

要求：

- 错误码稳定；
- 消息用于阅读；
- 调用方用 `get_if()`处理；
- 不把错误静默转0；
- 测试正数、负数、边界和非法输入。

---

## 44. 练习8：错误传播

对应文件：`src/08_error_propagation.cpp`

实现：

```text
parse_integer
parse_sample_rate
parse_duration
parse_channels
```

上层函数调用下层结果：

- 成功则继续；
- 失败则补充 `field`后返回；
- 不捕获后丢弃；
- 不使用默认值伪装成功；
- 第一个失败字段明确；
- 测试每层传播的错误码和字段。

---

## 45. 练习9：批量解析汇总

对应文件：`src/09_batch_parse_summary.cpp`

输入至少10行，包含成功和多种错误。

分别收集：

```cpp
std::vector<AudioMetadata> successes;
std::vector<ParseError> errors;
```

要求：

- 每行结果使用 `variant`；
- 错误保留行号；
- 统计成功/失败数量；
- 按错误码统计；
- 成功记录继续进入后续处理；
- 不因一条错误丢掉全部成功数据；
- 空输入安全。

---

## 46. 练习10：音频元数据解析器综合项目

对应文件：

- 数据结构：`include/audio_metadata.hpp`
- 错误结构：`include/parse_error.hpp`
- 结果别名：`include/parse_result.hpp`
- 解析器声明：`include/audio_metadata_parser.hpp`
- 解析器实现：`src/audio_metadata_parser.cpp`
- 演示：`src/10_audio_metadata_parser.cpp`
- 测试：`tests/optional_variant_tests.cpp`
- 报告：`target/report/error_return_design.md`

### 项目定位

解析一行制表符分隔的音频元数据文本，不访问真实音频文件、不解码音频、不涉及硬件。

输入格式：

```text
path\tsample_rate\tchannels\tduration_seconds\tlabel\tquality_score
```

示例：

```text
meeting.wav\t16000\t1\t12.5\tspeech\t0.95
```

### 错误码至少包含

```text
empty_input
wrong_field_count
empty_path
invalid_sample_rate
invalid_channels
invalid_duration
empty_label
invalid_quality_score
number_out_of_range
```

### `ParseError`至少包含

```text
code
field
message
position或line_number
```

### 结果类型

```cpp
using ParseResult = std::variant<
    AudioMetadata,
    ParseError
>;
```

### 必须实现

```text
parse_line
parse_integer
parse_double
validate_metadata
parse_lines
error_code_name
format_error
```

### 行为要求

- 不使用异常表示普通字段解析失败；
- 可以捕获标准数值转换异常并转成 `ParseError`；
- 成功值和错误值二选一；
- 不返回部分初始化的 `AudioMetadata`；
- 每个错误保留字段上下文；
- 批量解析保留全部错误；
- 调用方不能忽略结果候选；
- 不使用拥有型裸 `new/delete`；
- 不通过错误消息字符串判断错误类型。

### `optional`在项目中的位置

可以用于：

- 查找某个可选配置；
- 从成功集合中寻找可选记录；
- 计算空集合可能没有的最长记录。

解析失败本身使用 `variant<Success, Error>`，因为需要具体错误原因。

---

## 47. 正式测试清单

文件：`tests/optional_variant_tests.cpp`

至少覆盖：

```text
1. optional默认无值
2. nullopt无值
3. optional有值
4. has_value正确
5. bool检查正确
6. value读取正确
7. 空value抛bad_optional_access
8. value_or默认值正确
9. reset销毁值并清空
10. emplace创建新值
11. optional对象成员访问正确
12. variant默认保存第一候选
13. variant保存int
14. variant保存string
15. index正确
16. holds_alternative正确
17. get正确类型成功
18. get错误类型抛bad_variant_access
19. get_if正确类型返回指针
20. get_if错误类型返回nullptr
21. visit处理全部候选
22. monostate作为默认业务状态
23. monostate与异常无值状态区分
24. IntResult成功解析整数
25. 空文本返回对应错误码
26. 非数字返回对应错误码
27. 超范围返回对应错误码
28. 负数解析规则明确
29. 错误字段上下文正确
30. 错误传播不丢失错误码
31. 错误传播补充字段名
32. 正常查找缺失使用optional
33. 解析错误使用variant而非nullopt
34. 合法完整元数据解析成功
35. 空行解析失败
36. 字段数量过少失败
37. 字段数量过多失败
38. 空路径失败
39. 非法采样率失败
40. 非法声道数失败
41. 非法时长失败
42. 空标签失败
43. 质量分数低于0失败
44. 质量分数高于1失败
45. 质量分数边界0成功
46. 质量分数边界1成功
47. 错误中保留行号
48. 批量解析成功数量正确
49. 批量解析失败数量正确
50. 批量解析保留所有错误
51. 按错误码统计正确
52. 空批次安全
53. 成功结果不包含部分错误数据
54. 错误结果不伪装默认元数据
55. format_error输出包含必要上下文
56. 正式测试ASan零错误
57. 正式测试UBSan零错误
```

---

## 48. 编译命令

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
  src/01_optional_basics.cpp \
  -Iinclude \
  -o target/01_optional_basics
```

### 综合项目

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
  src/10_audio_metadata_parser.cpp \
  src/audio_metadata_parser.cpp \
  -Iinclude \
  -o target/10_audio_metadata_parser
```

运行：

```bash
./target/10_audio_metadata_parser
```

---

## 49. Sanitizer边界

ASan/UBSan可能发现：

- 空 `optional`被错误解引用导致的部分异常行为；
- 保存内部引用后 `reset()`造成的悬空使用；
- 保存 `get_if()`指针后切换候选再访问；
- 越界与其他未定义行为。

但不能替代：

- 是否应该用 `optional`还是详细错误；
- 错误码设计；
- 调用方是否吞掉错误；
- `value_or()`是否掩盖失败；
- 批量遇错策略；
- 错误上下文是否完整；
- 正常缺失和错误是否被混淆。

因此需要：

```text
状态测试
+ 每个错误码测试
+ 错误传播测试
+ ASan/UBSan
+ API设计审查
```

---

## 50. 实验报告模板

文件：`target/report/error_return_design.md`

```markdown
# optional、variant与错误返回设计报告

## 环境
- 编译器：
- C++标准：C++17
- 编译参数：

## optional
- 有值/无值语义：
- value与解引用：
- value_or使用边界：
- 生命周期风险：
- 适用场景：

## variant
- 候选类型：
- 默认状态：
- holds/get/get_if：
- visit：
- monostate：
- valueless_by_exception：

## 错误返回方案
- 特殊值：
- bool+输出参数：
- optional：
- variant<Value,Error>：
- 异常：

## 解析器设计
- ParseErrorCode：
- ParseError字段：
- ParseResult：
- 错误传播：
- 批量处理策略：

## 测试
- 成功案例：
- 错误码覆盖：
- 边界值：
- 批量统计：

## Sanitizer
- ASan：
- UBSan：

## 今日结论
```

---

## 51. 今日思考题

1. `optional<T>`表达什么状态？
2. 空 `optional`内部是否有一个默认 `T`？
3. `nullopt`有什么作用？
4. `value()`在空状态做什么？
5. `operator*`在空状态安全吗？
6. `value_or()`什么时候适合？
7. 为什么昂贵默认表达式可能仍被求值？
8. `reset()`后原内部引用怎样？
9. `optional<T>`和 `T*`语义相同吗？
10. 为什么不能直接使用 `optional<T&>`？
11. `variant`任一时刻保存几个候选？
12. 默认构造 `variant`是什么状态？
13. `index()`有什么局限？
14. `holds_alternative()`做什么？
15. 错误类型调用 `get()`会怎样？
16. `get_if()`失败返回什么？
17. `get_if()`返回指针什么时候失效？
18. `visit()`为什么必须能处理全部候选？
19. `monostate`是什么？
20. 它和 `valueless_by_exception`有什么区别？
21. 查找不存在为什么适合 `optional`？
22. 解析失败为什么更适合详细错误？
23. `bool+输出参数`有什么风险？
24. 特殊值错误码有什么值域冲突？
25. 异常适合哪些错误边界？
26. 为什么不能通过错误消息判断错误种类？
27. 什么是错误传播？
28. 为什么不能把解析失败静默转成0？
29. 批量处理有哪些遇错策略？
30. C++17为什么用 `variant<Value,Error>`模拟结果类型？

---

## 52. 今日验收清单

### 知识

- [ ] 能解释 `optional`有/无值语义；
- [ ] 能安全读取和重置 `optional`；
- [ ] 能解释 `optional`与指针区别；
- [ ] 能构造和切换 `variant`；
- [ ] 能使用 `get_if()`和 `visit()`；
- [ ] 能解释 `monostate`；
- [ ] 能区分正常缺失与错误；
- [ ] 能比较五种错误返回方案；
- [ ] 能设计结构化错误对象；
- [ ] 能传播错误而不丢失上下文。

### 编码

- [ ] 完成 `01_optional_basics.cpp`；
- [ ] 完成 `02_optional_lookup.cpp`；
- [ ] 完成 `03_variant_basics.cpp`；
- [ ] 完成 `04_variant_access.cpp`；
- [ ] 完成 `05_monostate_state.cpp`；
- [ ] 完成 `06_error_return_comparison.cpp`；
- [ ] 完成 `07_value_or_error.cpp`；
- [ ] 完成 `08_error_propagation.cpp`；
- [ ] 完成 `09_batch_parse_summary.cpp`；
- [ ] 完成音频元数据解析器。

### 验证与输出

- [ ] 至少57项测试通过；
- [ ] 每个错误码有测试；
- [ ] 正常缺失使用 `optional`；
- [ ] 解析错误使用结构化结果；
- [ ] 批量错误全部保留；
- [ ] ASan/UBSan零错误；
- [ ] 完成实验报告；
- [ ] 完成Git提交。

---

## 53. 与就业方向的联系

### AI部署与工程化

这些设计用于：

- 模型查找可能不存在；
- 配置字段可选；
- 输入张量或音频元数据解析；
- 推理结果或错误返回；
- 后端状态的多类型表示；
- 批处理部分失败；
- 服务边界错误码；
- 日志和可观测性。

### C++工程岗位

常见面试与评审问题：

- `optional`与指针的区别；
- `value()`与 `operator*`；
- `variant`访问方式；
- `monostate`用途；
- `get_if`生命周期；
- optional/异常/错误码如何选择；
- 如何设计不易被忽略的错误返回值。

---

## 54. 今日产出

```text
9个专项练习
1个类型安全音频元数据解析器
1个测试文件
1份错误返回设计报告
1次可复现Git提交
```

---

## 55. Git提交建议

```bash
git status
git add day26
git commit -m "Learn optional variant and typed error returns"
```

提交前检查：

- 不提交二进制文件；
- 错误信息不包含敏感路径；
- 正式代码不解引用空状态；
- 测试覆盖所有错误码；
- 不通过哈希遍历顺序判断结果；
- 编译命令可复现。

---

## 56. 最终速记

```text
optional<T>：有一个T或没有值

空optional：value()抛异常，operator*不安全

value_or：只用于真实存在默认值的业务

variant<Ts...>：任一时刻保存一个候选类型

holds_alternative：检查类型
get：类型错误抛bad_variant_access
get_if：类型错误返回nullptr
visit：统一访问当前候选

monostate：人为设计的空业务候选

正常查找不存在 -> optional

成功或详细失败 -> variant<Value,Error>

错误对象：稳定错误码 + 字段/位置 + 可读消息

错误传播：不能处理就保留或增强上下文后返回，不静默伪装成功
```
