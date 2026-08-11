# 第 5 周·工作日 1：掌握 `vector`、`array`、`string`及容量与迭代器失效规则

## 0. 前置知识与超纲说明

### 今天必须掌握

- `std::vector<T>`表示动态连续数组；
- `std::array<T, N>`表示编译期固定长度数组；
- `std::string`是管理字符序列的标准库类；
- `size()`表示当前有效元素数量；
- `capacity()`表示当前已分配空间最多可容纳的元素数量；
- `empty()`只判断当前元素数量是否为0；
- `reserve()`改变容量准备，不直接改变元素数量；
- `resize()`改变元素数量，必要时也会改变容量；
- `shrink_to_fit()`只是非强制的收缩请求；
- `vector`重新分配时，原有迭代器、引用和指针全部失效；
- `vector`未重新分配时，插入位置之前的迭代器通常保持有效，插入位置及其后的失效；
- `erase()`会使被删除位置及其后的迭代器、引用和指针失效；
- `string`同样使用连续动态存储，修改内容或容量后要谨慎对待旧的 `data()`、`c_str()`、迭代器和引用；
- `array`不会动态扩容，因此不会因为 `push_back`式操作重新分配；
- 迭代器失效后继续解引用属于未定义行为；
- ASan可能发现因重新分配产生的悬空访问，但不保证发现所有迭代器失效错误。

### 今天会复习

- 栈对象与堆资源；
- RAII；
- 动态数组和裸指针；
- `std::unique_ptr<T[]>`；
- 拷贝与移动；
- 引用、指针和 `const`；
- ASan释放后使用报告；
- 范围 `for`循环；
- 类和结构体。

### 今天第一次系统理解

- 容器逻辑大小与已分配容量的区别；
- 连续存储和重新分配之间的关系；
- 迭代器、引用和元素地址为什么会一起失效；
- `reserve()`如何降低重复扩容和失效次数；
- 为什么修改容器时不能盲目保留旧迭代器；
- 为什么 `erase()`通常要接收其返回的新迭代器；
- `string::c_str()`返回地址的生命周期边界；
- 用 `struct`表示纯数据记录并存入 `vector`。

### 今天只需了解，后续再深入

- 自定义迭代器；
- 迭代器类别和 iterator traits；
- `std::span`非拥有连续视图；
- 分配器与 `std::pmr::vector`；
- Small String Optimization（SSO）；
- `vector<bool>`特化；
- 容器异常安全的完整标准保证；
- Debug Iterator和标准库调试模式；
- ranges库。

### 模板超纲提示

你还没有系统学习模板。下面的：

```cpp
std::vector<AudioMetadata>
std::array<int, 4>
```

分别表示：

```text
vector存放AudioMetadata类型元素
array存放4个int元素
```

今天只使用标准库模板，不要求自己实现模板。模板会在第6周系统学习。

### `struct`补充说明

今天会正式补充结构体的入门用法：

```cpp
struct AudioMetadata {
    std::string path;
    int sample_rate;
    double duration_seconds;
};
```

当前阶段先理解：

```text
struct成员默认public
class成员默认private
两者都可以有构造函数和成员函数
struct常用于简单数据记录
```

结构体布局、对齐、padding和标准布局会在后续相关任务中继续补充。

### 今天明确不做

- 不自己实现 `vector`；
- 不背诵具体扩容倍数；
- 不假设所有实现都按2倍扩容；
- 不把 `capacity()`当作有效元素数量；
- 不访问 `[size(), capacity())`之间尚未构造的元素；
- 不在范围 `for`中随意改变正在遍历的容器大小；
- 不依赖失效迭代器“当前看起来还能用”；
- 不用 ASan替代迭代器规则分析。

> 今天最重要的能力是：每次修改容器后，都能判断之前保存的迭代器、引用和元素地址是否仍然有效。

---

## 1. 今日目标

完成后你应该能够：

- 根据长度是否固定选择 `array`或 `vector`；
- 使用 `string`安全保存和修改文本；
- 写出 `size()`与 `capacity()`的实验；
- 解释 `reserve()`和 `resize()`的根本区别；
- 观察 `vector`扩容前后的 `data()`地址；
- 判断 `push_back`是否触发重新分配；
- 判断插入、删除、清空、调整大小后的失效范围；
- 安全使用 `begin/end/cbegin/cend`；
- 使用 `erase()`返回值继续遍历；
- 避免在范围 `for`内修改容器导致失效；
- 判断 `string::c_str()`和 `data()`返回地址何时需要重新获取；
- 使用 `struct AudioMetadata`与 `vector`完成元数据目录；
- 写出容器容量和失效规则的测试及实验报告。

建议用时：约 **3～4小时**。

```text
30分钟：vector/array/string选择与基础接口
35分钟：size、capacity、reserve、resize
40分钟：vector重新分配和失效规则
30分钟：erase、insert与安全遍历
30分钟：string地址和迭代器边界
60～90分钟：AudioMetadata目录、测试和ASan实验
```

---

## 2. 今日目录

已经建立：

```text
day19/
├── include/
├── src/
├── tests/
├── target/
│   └── report/
└── 第5周_工作日1_vector_array_string容量与迭代器失效规则.md
```

建议逐步创建：

```text
include/
├── audio_metadata.hpp
├── audio_catalog.hpp
└── test_runner.hpp

src/
├── 01_vector_basics.cpp
├── 02_array_basics.cpp
├── 03_string_basics.cpp
├── 04_size_capacity.cpp
├── 05_reserve_resize.cpp
├── 06_vector_reallocation.cpp
├── 07_insert_invalidation.cpp
├── 08_erase_iteration.cpp
├── 09_string_pointer_invalidation.cpp
├── 10_range_for_mutation.cpp
├── audio_catalog.cpp
└── test_runner.cpp

tests/
└── container_tests.cpp
```

源码由你按照练习要求创建，不直接复制最终实现。

---

## 3. 三种类型分别解决什么问题

### `std::array<T, N>`

适合长度在编译时固定：

```cpp
std::array<int, 4> supported_rates{
    16000,
    22050,
    44100,
    48000
};
```

特点：

- 元素数量固定为 `N`；
- 不能 `push_back`或 `erase`；
- 保存自己的元素；
- 支持标准容器接口和迭代器；
- 不发生动态扩容。

### `std::vector<T>`

适合运行时数量变化：

```cpp
std::vector<float> samples;

samples.push_back(0.1F);
samples.push_back(0.2F);
```

特点：

- 元素连续存储；
- 元素数量可增长或缩小；
- 自动管理动态内存；
- 扩容时可能重新分配；
- 重新分配会使旧地址和迭代器失效。

### `std::string`

适合字符串文本：

```cpp
std::string model_name = "Qwen2.5-Omni";
```

特点：

- 自动管理字符内存；
- 支持拼接、搜索和比较；
- 字符连续存储；
- 有 `size()`和 `capacity()`；
- 修改后旧字符地址和迭代器可能失效。

---

## 4. 为什么动态数组通常优先 `vector`

旧式代码：

```cpp
float* samples = new float[count]{};

// 使用samples

delete[] samples;
```

需要手工维护：

- 地址；
- 长度；
- 释放；
- 复制；
- 移动；
- 异常路径。

`vector`：

```cpp
std::vector<float> samples(
    count,
    0.0F
);
```

自动提供：

- RAII释放；
- `size()`；
- 深复制；
- 移动；
- 迭代器；
- 扩容。

所以普通动态元素集合优先 `vector`，而不是 `new[]`或 `unique_ptr<T[]>`。

---

## 5. `vector`的基本创建方式

```cpp
#include <vector>
```

空容器：

```cpp
std::vector<int> values;
```

5个值初始化为0的元素：

```cpp
std::vector<int> values(5);
```

5个值为42的元素：

```cpp
std::vector<int> values(5, 42);
```

初始化列表：

```cpp
std::vector<int> values{
    7,
    14,
    21,
    28
};
```

注意圆括号和花括号含义不同：

```cpp
std::vector<int> first(3, 5);
// 3个元素，每个值为5：{5,5,5}

std::vector<int> second{3, 5};
// 2个元素：{3,5}
```

---

## 6. 元素访问

下标：

```cpp
values[0]
```

`operator[]`不进行边界检查。越界属于未定义行为：

```cpp
values[values.size()]
```

一定越过最后一个元素，因为最后合法下标是：

```cpp
values.size() - 1
```

带检查访问：

```cpp
values.at(0)
```

越界时抛出：

```cpp
std::out_of_range
```

首尾访问：

```cpp
values.front()
values.back()
```

空容器不能调用 `front()`和 `back()`。

获取连续内存地址：

```cpp
int* pointer = values.data();
```

`data()`返回非拥有地址。容器重新分配或销毁后，旧地址可能失效。

---

## 7. `size()`与 `capacity()`

```cpp
std::cout << values.size() << '\n';
std::cout << values.capacity() << '\n';
```

### `size()`

表示当前已经存在、可以正常访问的元素数量。

```text
合法下标范围：[0, size())
```

### `capacity()`

表示不重新分配的前提下，当前存储空间最多可以容纳多少个元素。

始终满足：

```text
capacity >= size
```

但不能访问：

```cpp
values[values.size()]
```

即使：

```text
size < capacity
```

原因是容量范围中尚未成为元素的位置没有合法构造的元素。

---

## 8. 容量示意图

假设：

```text
size = 3
capacity = 8
```

概念布局：

```text
vector已分配空间

[元素0][元素1][元素2][预留][预留][预留][预留][预留]
 <------ size ------>
 <---------------- capacity ---------------->
```

只有前三个元素存在：

```cpp
values[0]
values[1]
values[2]
```

后五个位置只是存储准备，不是可以通过下标访问的对象。

---

## 9. `push_back()`与 `emplace_back()`

加入已有对象：

```cpp
AudioMetadata metadata{
    "meeting.wav",
    16000,
    12.5
};

catalog.push_back(metadata);
```

如果不再需要原对象，可以移动：

```cpp
catalog.push_back(
    std::move(metadata)
);
```

直接在容器尾部构造：

```cpp
catalog.emplace_back(
    "meeting.wav",
    16000,
    12.5
);
```

`emplace_back`不是永远更快，也不是所有调用都必须替换为它。今天重点是知道两者都会增加 `size()`，也可能触发重新分配。

---

## 10. `reserve()`只准备容量

```cpp
std::vector<int> values;

values.reserve(100);
```

执行后通常：

```text
size = 0
capacity >= 100
```

不能因此访问：

```cpp
values[0] = 10;
```

因为没有元素被创建。

正确加入元素：

```cpp
values.push_back(10);
```

`reserve()`适合你预先知道大约会加入多少元素的场景：

```cpp
std::vector<AudioMetadata> catalog;
catalog.reserve(expected_file_count);
```

这样可以减少多次扩容、移动元素和迭代器失效。

---

## 11. `resize()`改变元素数量

```cpp
std::vector<int> values;

values.resize(5);
```

执行后：

```text
size = 5
```

新增整数会值初始化为0。

指定新增值：

```cpp
values.resize(10, 42);
```

将 `size()`缩小：

```cpp
values.resize(3);
```

原来下标3及之后的元素被销毁，但容量通常不一定缩小。

---

## 12. `reserve()`和 `resize()`对比

| 操作 | 改变 `size()` | 可能改变 `capacity()` | 创建/销毁元素 |
|---|---:|---:|---:|
| `reserve(n)` | 否 | 是 | 不创建新元素 |
| `resize(n)` | 是 | 是 | 增长时创建，缩小时销毁 |

记忆：

```text
reserve：预留房间，但不入住
resize：改变实际入住人数
```

---

## 13. `clear()`和 `shrink_to_fit()`

```cpp
values.clear();
```

作用：

```text
销毁所有元素
size变为0
capacity通常保留
```

因此：

```cpp
values.clear();
std::cout << values.capacity();
```

容量可能仍然非0。

请求释放多余容量：

```cpp
values.shrink_to_fit();
```

但 `shrink_to_fit()`是非强制请求，实现可以不缩小。不能编写依赖容量一定等于 `size()`的逻辑。

如果它发生重新分配，所有旧迭代器、引用和指针都会失效。

---

## 14. 为什么 `vector`需要重新分配

假设：

```text
size = 4
capacity = 4
```

继续：

```cpp
values.push_back(50);
```

原区域放不下第5个元素。容器通常需要：

```text
申请一块更大的连续区域
        ↓
复制或移动原来的4个元素
        ↓
构造第5个元素
        ↓
释放旧区域
        ↓
更新内部地址和capacity
```

旧迭代器、引用和指针仍然指向已经释放的旧区域，因此全部失效。

---

## 15. 用 `data()`观察重新分配

```cpp
std::vector<int> values;

const int* previous = values.data();
std::size_t previous_capacity =
    values.capacity();

for (int i = 0; i < 20; ++i) {
    values.push_back(i);

    if (values.data() != previous) {
        std::cout
            << "reallocated: capacity "
            << previous_capacity
            << " -> "
            << values.capacity()
            << '\n';

        previous = values.data();
        previous_capacity = values.capacity();
    }
}
```

不要根据输出背扩容倍数。不同标准库、元素类型和容量都可能采用不同策略。

标准保证的是行为和复杂度要求，不保证固定2倍扩容。

---

## 16. 什么是迭代器

迭代器是用于定位和访问容器元素的对象，使用方式类似指针：

```cpp
auto iterator = values.begin();

std::cout << *iterator << '\n';
```

常用边界：

```cpp
values.begin() // 第一个元素
values.end()   // 最后一个元素之后的位置
```

`end()`不是有效元素，不能解引用：

```cpp
*values.end(); // 错误
```

遍历：

```cpp
for (
    auto iterator = values.begin();
    iterator != values.end();
    ++iterator
) {
    std::cout << *iterator << '\n';
}
```

只读遍历：

```cpp
for (
    auto iterator = values.cbegin();
    iterator != values.cend();
    ++iterator
) {
    std::cout << *iterator << '\n';
}
```

---

## 17. 迭代器、引用和指针为什么一起讨论

下面三者都指向 `values[0]`：

```cpp
auto iterator = values.begin();
int& reference = values[0];
int* pointer = &values[0];
```

发生重新分配后：

```text
元素被移动到新地址
旧存储被释放
```

因此：

```text
iterator失效
reference失效
pointer失效
```

“引用不能重新绑定”不代表它永远有效；它所引用对象被移动或销毁后，同样会悬空。

---

## 18. `vector`重新分配时的失效规则

如果一次操作导致 `capacity()`变化，即发生重新分配：

```text
所有迭代器失效
所有元素引用失效
所有元素指针失效
旧end()也失效
```

典型可能触发重新分配的操作：

- `push_back()`；
- `emplace_back()`；
- `insert()`；
- `emplace()`；
- 增长型 `resize()`；
- 增大型 `reserve()`；
- 实际发生收缩的 `shrink_to_fit()`。

判断最直接的实验方法：

```cpp
const auto old_capacity = values.capacity();

// 修改操作

const bool reallocated =
    values.capacity() != old_capacity;
```

更直观可以对比 `data()`，但不要在重分配后解引用旧地址。

---

## 19. `push_back()`未重新分配时

如果：

```text
size < capacity
```

尾部追加通常不需要重新分配。

此时：

- 原有元素的引用和指针保持有效；
- 原有元素的迭代器保持有效；
- 旧的 `end()`失效，因为尾后位置已经改变。

示例：

```cpp
std::vector<int> values;
values.reserve(10);
values.push_back(1);

int* first_address = &values[0];

values.push_back(2);

std::cout
    << (first_address == &values[0])
    << '\n';
```

这里容量充足时，第一元素地址通常保持相同，而且标准规则允许继续使用其引用和指针。

---

## 20. `insert()`未重新分配时

假设：

```cpp
std::vector<int> values{
    10,
    20,
    30,
    40
};

values.reserve(10);
```

在20之前插入：

```cpp
auto position = values.begin() + 1;

values.insert(position, 15);
```

即使没有重新分配，插入位置及其后面的元素会移动：

```text
插入前：10 20 30 40
插入后：10 15 20 30 40
```

规则：

```text
插入位置之前的迭代器/引用/指针保持有效
插入位置及之后的迭代器/引用/指针失效
旧end()失效
```

如果发生重新分配，则全部失效。

---

## 21. `erase()`失效规则

```cpp
std::vector<int> values{
    10,
    20,
    30,
    40
};

auto iterator = values.begin() + 1;

values.erase(iterator);
```

删除20后：

```text
10 30 40
```

30和40需要向前移动。

规则：

```text
被删除位置及其后的迭代器/引用/指针失效
删除位置之前保持有效
旧end()失效
```

`erase()`返回删除位置之后的新有效迭代器：

```cpp
iterator = values.erase(iterator);
```

因此遍历删除应使用返回值。

---

## 22. 安全的遍历删除

删除所有偶数：

```cpp
for (
    auto iterator = values.begin();
    iterator != values.end();
) {
    if (*iterator % 2 == 0) {
        iterator = values.erase(iterator);
    } else {
        ++iterator;
    }
}
```

错误写法：

```cpp
for (
    auto iterator = values.begin();
    iterator != values.end();
    ++iterator
) {
    if (*iterator % 2 == 0) {
        values.erase(iterator);
    }
}
```

原因：`erase(iterator)`后旧 `iterator`已经失效，循环结尾还继续 `++iterator`。

---

## 23. `pop_back()`、`clear()`和 `resize()`失效规则

### `pop_back()`

销毁最后一个元素：

```text
指向被删除最后元素的迭代器/引用/指针失效
旧end()失效
更早元素保持有效
```

空 `vector`不能调用 `pop_back()`。

### `clear()`

销毁全部元素：

```text
所有元素迭代器/引用/指针失效
capacity通常保留
```

容量保留不代表旧元素仍然存在。

### 缩小 `resize()`

```cpp
values.resize(smaller_size);
```

被删除元素及其后的相关定位全部失效。

### 增长 `resize()`

若重新分配，全部失效；若未重新分配，原有元素地址通常保持有效，但旧 `end()`以及与新增尾部边界相关的位置失效。

---

## 24. `reserve()`的失效规则

```cpp
values.reserve(new_capacity);
```

如果：

```text
new_capacity > 当前capacity
```

需要重新分配，全部迭代器、引用和指针失效。

如果：

```text
new_capacity <= 当前capacity
```

不会缩小容量，也不会发生重新分配，因此现有迭代器和引用保持有效。

不要在已经保存迭代器之后随意执行增大型 `reserve()`。

最好在取得迭代器或元素地址之前完成容量规划。

---

## 25. `array`为什么没有扩容失效

```cpp
std::array<int, 4> values{
    10,
    20,
    30,
    40
};
```

元素数量固定为4，没有：

```text
push_back
insert
erase
reserve
resize
```

所以不会因为扩容搬迁全部元素。

在 `array`自身生命周期内，指向其元素的迭代器、引用和指针通常保持有效。

但是：

- `array`对象销毁后全部失效；
- `array`被移动或复制不会让旧引用自动改为指向新对象；
- 对两个 `array`执行 `swap()`时，元素值会交换，已有引用和迭代器仍指向原数组中的相应位置，但看到的值可能改变。

---

## 26. `array`和原生数组的区别

原生数组：

```cpp
int values[4]{
    10,
    20,
    30,
    40
};
```

`std::array`：

```cpp
std::array<int, 4> values{
    10,
    20,
    30,
    40
};
```

`std::array`支持：

```cpp
values.size()
values.at(0)
values.begin()
values.end()
values.fill(0)
```

还能整体复制：

```cpp
auto copied = values;
```

当前阶段固定长度集合优先 `std::array`，而不是裸数组。

---

## 27. `string`基础

```cpp
#include <string>
```

创建：

```cpp
std::string path = "meeting.wav";
```

拼接：

```cpp
std::string message =
    "processing: " + path;
```

追加：

```cpp
message += " done";
message.append(" successfully");
```

访问：

```cpp
path[0]
path.at(0)
path.front()
path.back()
```

搜索：

```cpp
const auto position =
    path.find(".wav");

if (position != std::string::npos) {
    // 找到
}
```

---

## 28. `string`的 `size()`和 `capacity()`

```cpp
std::string text = "audio";

std::cout << text.size() << '\n';
std::cout << text.capacity() << '\n';
```

`size()`表示字符数量，不包含结尾空字符。

`capacity()`表示当前内部存储在重新分配前可容纳的字符数量，具体值依赖实现。

`string`保证可通过 `c_str()`得到以 `\0`结尾的 C字符串，但 `capacity()`通常讨论的是字符容量，不需要把结尾空字符手工加入业务长度。

---

## 29. `string::c_str()`和 `data()`

```cpp
const char* c_text = text.c_str();
```

用于把 C++字符串传给需要 C字符串的接口。

C++17中，非 `const string`的 `data()`可以返回可修改字符地址：

```cpp
char* data = text.data();
```

但只能在合法字符范围内修改，不能越界，也不能破坏字符串所需的结尾状态。

最重要的生命周期规则：

> `c_str()`和 `data()`返回的是字符串内部地址，不拥有数据；字符串修改、重新分配或销毁后，旧地址可能失效，应该重新获取。

---

## 30. `string`失效规则采用保守策略

不同标准版本和成员操作的精细保证较复杂。当前 C++17学习阶段采用安全、保守的工程规则：

```text
只读操作后，旧迭代器/引用/指针继续使用

任何可能改变string内容、size或capacity的操作后，
不要继续使用旧迭代器、引用、data()或c_str()地址，重新获取
```

可能引起变化的操作包括：

- `+=`；
- `append()`；
- `insert()`；
- `erase()`；
- `replace()`；
- `resize()`；
- `reserve()`；
- `shrink_to_fit()`；
- 赋值；
- `clear()`。

这条保守规则可能比某些具体实现保证更严格，但能避免依赖容易记错的细节。

---

## 31. `string`旧地址失效实验

```cpp
std::string text = "audio";

const char* old_address = text.c_str();
const auto old_capacity = text.capacity();

text.append(
    1000,
    'x'
);

std::cout
    << "capacity changed: "
    << (text.capacity() != old_capacity)
    << '\n';

std::cout
    << "address changed: "
    << (text.c_str() != old_address)
    << '\n';
```

只能比较旧地址值，不要在可能重新分配后解引用 `old_address`。

如果需要继续传给 C接口，应重新获取：

```cpp
const char* current = text.c_str();
```

---

## 32. Small String Optimization只需了解

许多 `string`实现会把较短字符串直接放在 `string`对象内部，减少堆分配，这通常称为 SSO。

但是：

- 标准不要求固定的 SSO容量；
- 不同标准库实现可能不同；
- 不能根据某次实验把具体阈值写进业务逻辑；
- 从短字符串增长为长字符串时，内部地址仍可能变化。

因此不管是否使用 SSO，修改字符串后重新获取 `c_str()`仍是安全习惯。

---

## 33. 范围 `for`中的引用

复制每个元素：

```cpp
for (auto metadata : catalog) {
    // metadata是副本
}
```

修改原元素：

```cpp
for (auto& metadata : catalog) {
    metadata.sample_rate = 48000;
}
```

只读且避免复制：

```cpp
for (const auto& metadata : catalog) {
    std::cout << metadata.path << '\n';
}
```

通常读取大型对象时使用：

```cpp
const auto&
```

---

## 34. 为什么不能在范围 `for`中 `push_back`

危险：

```cpp
for (const auto& item : values) {
    if (item > 0) {
        values.push_back(item);
    }
}
```

范围 `for`内部依赖迭代器和结束位置。`push_back`可能重新分配，使当前迭代器和引用全部失效；即使没有重新分配，旧 `end()`也已经变化。

结果属于未定义行为。

更合理做法：

- 先收集要添加的元素到另一个容器；
- 遍历结束后统一追加；
- 或根据固定的原始 `size()`使用索引并保证逻辑清楚。

例如：

```cpp
const std::size_t original_size =
    values.size();

for (
    std::size_t i = 0;
    i < original_size;
    ++i
) {
    if (values[i] > 0) {
        values.push_back(values[i]);
    }
}
```

这里不要保存元素引用跨越 `push_back`。

---

## 35. ASan是否一定发现迭代器失效

### 重新分配后的旧地址

旧存储通常被释放：

```cpp
int* pointer = &values[0];

// 触发重新分配
values.push_back(...);

std::cout << *pointer;
```

ASan很可能报告：

```text
heap-use-after-free
```

### `erase()`后的旧迭代器

`erase()`通常不会释放整个容量，后续元素只是向前移动。旧迭代器可能仍指向一块已分配内存，甚至读出看似正常的值。

这种未定义行为不一定被 ASan发现。

所以：

```text
ASan能够补充检测
迭代器规则才是主要依据
```

---

## 36. `struct AudioMetadata`

文件：`include/audio_metadata.hpp`

```cpp
#pragma once

#include <string>

struct AudioMetadata {
    std::string path;
    int sample_rate;
    double duration_seconds;
};
```

初始化：

```cpp
AudioMetadata metadata{
    "meeting.wav",
    16000,
    12.5
};
```

访问：

```cpp
std::cout << metadata.path << '\n';
```

当前 `struct`只是数据记录，不需要 getter/setter。后续若需要维持复杂不变量，可以改成封装类。

---

## 37. 音频元数据目录设计

```cpp
class AudioCatalog {
public:
    void reserve(std::size_t count);

    void add(AudioMetadata metadata);

    bool remove_by_path(
        const std::string& path
    );

    const AudioMetadata* find_by_path(
        const std::string& path
    ) const noexcept;

    std::size_t size() const noexcept;
    std::size_t capacity() const noexcept;

private:
    std::vector<AudioMetadata> entries_;
};
```

这里返回：

```cpp
const AudioMetadata*
```

表示：

```text
允许找不到并返回nullptr
调用者不拥有元素
调用者不能通过该指针修改元素
```

但必须记录：只要 `entries_`发生可能失效的修改，之前返回的元素指针就可能失效，调用者需要重新查找。

---

## 38. `remove_by_path()`的安全实现思路

```cpp
bool AudioCatalog::remove_by_path(
    const std::string& path
) {
    for (
        auto iterator = entries_.begin();
        iterator != entries_.end();
        ++iterator
    ) {
        if (iterator->path == path) {
            entries_.erase(iterator);
            return true;
        }
    }

    return false;
}
```

这里删除后立即返回，因此不会继续使用失效的 `iterator`。

如果删除后还要继续遍历，就必须：

```cpp
iterator = entries_.erase(iterator);
```

---

## 39. `vector`失效规则总结表

| 操作 | 若发生重新分配 | 若未重新分配 |
|---|---|---|
| `push_back/emplace_back` | 全部失效 | 原元素定位有效，旧 `end()`失效 |
| `insert/emplace` | 全部失效 | 插入位置及之后失效 |
| `reserve`增大容量 | 全部失效 | 不增大时不失效 |
| `resize`增长 | 全部失效 | 原元素定位通常有效，旧 `end()`失效 |
| `resize`缩小 | 通常不重分配 | 被删除元素及之后失效 |
| `erase` | 通常不重分配 | 删除位置及之后失效 |
| `pop_back` | 通常不重分配 | 最后元素与旧 `end()`失效 |
| `clear` | 通常保留容量 | 所有元素定位失效 |
| `shrink_to_fit` | 若实际重分配则全部失效 | 未重分配则不因该操作失效 |

“定位”包括：

```text
迭代器
元素引用
元素裸指针
data()返回地址
```

---

## 40. 常见错误

### 错误1：把 `capacity()`当作合法元素数

```cpp
values.reserve(100);
values[0] = 1;
```

此时 `size()`仍为0。

### 错误2：扩容后使用旧指针

```cpp
int* pointer = values.data();
values.push_back(42);
std::cout << *pointer;
```

如果发生重新分配，指针悬空。

### 错误3：`erase()`后继续递增旧迭代器

必须使用返回值。

### 错误4：解引用 `end()`

`end()`是尾后位置，不是最后一个元素。

### 错误5：空容器调用 `front/back/pop_back`

先检查 `empty()`。

### 错误6：范围 `for`中改变容器大小

可能使内部迭代器和结束位置失效。

### 错误7：修改 `string`后继续使用旧 `c_str()`

重新获取地址。

### 错误8：认为 `reserve()`会创建元素

它只准备容量。

### 错误9：认为 `shrink_to_fit()`一定释放内存

它只是非强制请求。

### 错误10：依赖固定扩容倍数

标准不保证固定倍率。

---

## 41. 练习1：`vector`基础

对应文件：`src/01_vector_basics.cpp`

创建：

```cpp
std::vector<int> values{
    7,
    14,
    21,
    28
};
```

完成：

- 打印 `size/capacity/empty`；
- 使用 `[]`、`at()`、`front()`和 `back()`；
- 追加两个元素；
- 捕获一次 `at()`越界异常；
- 解释合法下标范围。

---

## 42. 练习2：`array`固定采样率

对应文件：`src/02_array_sample_rates.cpp`

使用：

```cpp
std::array<int, 4>
```

保存：

```text
16000 22050 44100 48000
```

要求：

- 使用范围 `for`只读遍历；
- 使用 `at()`访问；
- 使用 `size()`；
- 使用 `front/back`；
- 复制整个 `array`；
- 说明为什么它不需要容量和扩容接口。

---

## 43. 练习3：`string`路径处理

对应文件：`src/03_string_path.cpp`

输入：

```text
/data/audio/meeting.wav
```

完成：

- 判断是否包含 `.wav`；
- 提取文件名；
- 拼接处理状态；
- 打印 `size/capacity`；
- 获取一次 `c_str()`；
- 修改字符串后重新获取 `c_str()`；
- 不解引用旧地址。

---

## 44. 练习4：容量增长日志

对应文件：`src/04_vector_capacity.cpp`

从空 `vector<int>`开始添加100个元素。仅在容量变化时输出：

```text
old_size
new_size
old_capacity
new_capacity
old_data_address
new_data_address
```

要求：

- 不背扩容倍率；
- 记录发生了多少次重新分配；
- 再调用 `reserve(100)`后重做实验；
- 比较重新分配次数。

---

## 45. 练习5：`reserve`与 `resize`

对应文件：`src/05_reserve_resize.cpp`

分别完成：

```cpp
std::vector<int> first;
first.reserve(5);

std::vector<int> second;
second.resize(5);
```

打印并比较：

```text
size
capacity
能否访问下标0
当前实际元素值
```

故意对 `first.at(0)`进行访问并捕获异常，不要使用 `first[0]`制造未定义行为。

---

## 46. 练习6：重新分配导致地址失效

对应文件：

- 安全观察：`src/06_reallocation_observe.cpp`
- ASan负向案例：`src/06_reallocation_invalid.cpp`

安全观察版：

```cpp
std::vector<int> values;
values.push_back(10);

const int* old_address = values.data();
const auto old_capacity = values.capacity();

while (values.capacity() == old_capacity) {
    values.push_back(20);
}

std::cout
    << "old: "
    << static_cast<const void*>(old_address)
    << '\n';

std::cout
    << "new: "
    << static_cast<const void*>(values.data())
    << '\n';
```

只比较和打印旧地址，不解引用。

可选负向 ASan文件中解引用旧地址，观察 `heap-use-after-free`；该文件必须与最终测试分开。

---

## 47. 练习7：插入位置失效分析

对应文件：`src/07_insert_invalidation.cpp`

预留足够容量，保存：

```text
插入位置之前元素的地址
插入位置元素的地址
插入位置之后元素的地址
```

执行中间插入后：

- 不解引用已失效地址；
- 重新获取新地址；
- 解释为什么插入位置之前保持有效；
- 解释为什么插入位置及之后失效；
- 验证 `data()`没有变化，说明没有整体重分配。

---

## 48. 练习8：安全遍历删除

对应文件：`src/08_safe_erase.cpp`

从：

```cpp
std::vector<int> values{
    1,2,3,4,5,6,7,8,9,10
};
```

删除所有偶数。

要求：

- 使用 `iterator = erase(iterator)`；
- 不能在失效迭代器上 `++`；
- 最终结果为 `{1,3,5,7,9}`；
- 写测试验证 `size()`和每个元素。

---

## 49. 练习9：范围 `for`修改错误分析

对应文件：`src/09_range_for_invalidation.cpp`

分析：

```cpp
for (const int& value : values) {
    values.push_back(value);
}
```

要求回答：

1. 范围 `for`内部依赖什么？
2. `push_back`重分配后哪些对象失效？
3. 即使没有重新分配，旧 `end()`是否仍然代表新结束位置？
4. 怎样用原始 `size()`和索引改写？
5. 为什么不能保存 `const int& value`跨过 `push_back`？

---

## 50. 练习10：AudioCatalog综合项目

对应文件：

- 数据结构声明：`include/audio_metadata.hpp`
- 类声明：`include/audio_catalog.hpp`
- 类实现：`src/audio_catalog.cpp`
- 运行示例：`src/10_audio_catalog.cpp`
- 单元测试：`tests/container_tests.cpp`
- 实验报告：`target/report/capacity_and_invalidation.md`

实现：

```text
AudioMetadata结构体
AudioCatalog类
reserve
add
find_by_path
remove_by_path
size
capacity
```

至少加入：

```text
meeting.wav 16000 12.5
music.wav   44100 180.0
noise.wav   48000 5.2
```

要求：

- 使用 `vector<AudioMetadata>`；
- `add`按值接收并移动到容器；
- 查找不存在时返回 `nullptr`；
- 删除使用安全迭代器逻辑；
- 在文档中说明返回元素指针的失效边界；
- 预留容量前后比较重新分配次数；
- 不出现裸拥有型 `new/delete`。

---

## 51. 单元测试要求

文件：`tests/container_tests.cpp`

至少覆盖：

```text
1. vector初始化元素正确
2. reserve不改变size
3. resize改变size并创建元素
4. at越界抛out_of_range
5. 预留容量内push_back保持旧元素地址
6. 重新分配后data地址变化
7. erase返回下一个有效迭代器
8. 安全删除所有偶数
9. array固定size为4
10. string查找后缀成功
11. AudioCatalog添加元数据
12. AudioCatalog查找存在项
13. AudioCatalog查找不存在项返回nullptr
14. AudioCatalog删除后size减少
15. 容器正式测试ASan零错误
```

不要在正式测试中解引用失效迭代器。失效错误只放进独立负向案例。

---

## 52. 编译命令

单文件：

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
  src/04_size_capacity.cpp \
  -I include \
  -o target/04_size_capacity
```

测试：

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
  src/audio_catalog.cpp \
  src/test_runner.cpp \
  tests/container_tests.cpp \
  -I include \
  -o target/container_tests
```

运行并保存：

```bash
./target/container_tests \
  > target/report/container_tests.txt \
  2>&1

exit_code=$?

cat target/report/container_tests.txt
echo "exit_code=$exit_code"
```

---

## 53. 今日报告

创建：

```text
target/report/capacity_and_invalidation.md
```

至少包含：

```markdown
# 容量与迭代器失效实验

## vector扩容记录
- 未reserve时扩容次数
- reserve后扩容次数
- data地址变化

## reserve与resize
- size变化
- capacity变化
- 元素是否存在

## 失效规则
- push_back
- insert
- erase
- clear
- resize
- reserve

## string内部地址
- 修改前c_str地址
- 修改后c_str地址
- 重新获取原则

## ASan边界
- 哪个失效案例被检测
- 哪个失效案例可能未被检测

## AudioCatalog接口约定
- 所有权
- 返回指针的有效期
- 哪些操作后必须重新查找
```

---

## 54. 今日必须回答的问题

1. `vector`、`array`和 `string`分别适合什么场景？
2. `size()`与 `capacity()`有什么区别？
3. 为什么不能访问 `[size(), capacity())`？
4. `reserve()`是否创建元素？
5. `resize()`增长和缩小时分别做什么？
6. `clear()`是否一定释放容量？
7. `shrink_to_fit()`是否强制收缩？
8. 为什么 `vector`扩容需要移动或复制元素？
9. 重新分配后哪些定位对象会失效？
10. 未重新分配的尾部追加会使什么失效？
11. 中间插入未重新分配时，哪些位置失效？
12. `erase()`后为什么应使用返回迭代器？
13. `end()`能否解引用？
14. 为什么范围 `for`中 `push_back`危险？
15. `array`为什么没有扩容失效？
16. 修改 `string`后为什么要重新获取 `c_str()`？
17. `string::size()`是否包含结尾 `\0`？
18. SSO为什么不能写进可移植业务逻辑？
19. ASan为什么可能发现扩容后的旧指针？
20. ASan为什么可能发现不了 `erase()`后的旧迭代器？
21. `struct`与 `class`的默认访问权限有什么区别？
22. `AudioCatalog::find_by_path()`返回裸指针是否拥有元素？
23. 哪些目录修改后，之前返回的元素指针可能失效？
24. 为什么预知数量时应尽早 `reserve()`？
25. 为什么不能背固定扩容倍率？

---

## 55. 今日验收清单

### 概念

- [ ] 能区分 `vector/array/string`；
- [ ] 能区分 `size/capacity`；
- [ ] 能区分 `reserve/resize`；
- [ ] 知道 `shrink_to_fit`非强制；
- [ ] 能解释连续存储与重新分配；
- [ ] 能判断主要修改操作的失效范围；
- [ ] 知道 `end()`是尾后位置；
- [ ] 知道 `string`旧内部地址的风险；
- [ ] 能说明 ASan对失效错误的检测边界。

### 编码

- [ ] 完成三种容器基础实验；
- [ ] 完成容量增长日志；
- [ ] 完成 `reserve/resize`对比；
- [ ] 完成重新分配地址实验；
- [ ] 完成安全遍历删除；
- [ ] 定义 `AudioMetadata`结构体；
- [ ] 实现 `AudioCatalog`；
- [ ] 正式源码没有裸拥有型 `new/delete`。

### 测试与输出

- [ ] 所有单元测试通过；
- [ ] ASan没有报告正式测试错误；
- [ ] 退出码为0；
- [ ] 保存 `container_tests.txt`；
- [ ] 完成 `capacity_and_invalidation.md`；
- [ ] 报告记录实际扩容次数而非假设倍率；
- [ ] 报告包含返回元素指针的失效约定。

---

## 56. 与 AI部署工程的联系

后续项目中会大量使用：

```text
vector<float>          音频样本
vector<int64_t>        Token ID或张量形状
vector<Request>        请求批次
array<int, N>          固定维度或固定配置
string                 模型路径、设备名、请求文本
vector<AudioMetadata>  数据集元信息
```

如果不理解容量和失效规则，可能产生：

- 保存 `data()`后继续扩容，向推理框架传入悬空地址；
- 批处理队列追加请求后，旧引用失效；
- 删除失败请求时继续使用失效迭代器；
- 修改模型路径后继续使用旧 `c_str()`；
- 把 `capacity()`误当成有效张量元素数量。

真实部署框架经常接收：

```cpp
samples.data()
shape.data()
model_path.c_str()
```

所以必须保证：

```text
框架使用地址期间，容器仍然存活
容器没有发生导致重新分配的修改
传入的元素数量使用size而不是capacity
```

今天的容量和失效规则是后续 ONNX Runtime、TensorRT和网络请求批处理的直接基础。

---

## 57. 今日项目产出

今天应得到：

```text
1. vector/array/string基础实验
2. 容量增长与地址变化日志
3. 迭代器失效规则说明
4. AudioMetadata结构体
5. AudioCatalog小项目
6. 容器单元测试
7. capacity_and_invalidation.md实验报告
```

`AudioCatalog`将继续为后面的元数据索引、算法、文件处理和模型输入管理提供基础。

---

## 58. 今日 Git提交建议

```bash
git status
git add day19
git commit -m "Learn container capacity and iterator invalidation"
```

README建议记录：

```text
1. vector的size是有效元素数，capacity只是存储准备
2. reserve不创建元素，resize改变元素数量
3. vector重新分配会使所有旧迭代器、引用和指针失效
4. erase后使用返回迭代器继续遍历
5. 修改string后重新获取data或c_str地址
6. array用于编译期固定长度集合
7. ASan不能替代迭代器失效规则
8. AudioCatalog返回的元素指针是非拥有且可能失效
```

---

## 59. 今日一句话总结

```text
vector、array和string不仅要会增删查改，更要知道size与capacity的边界，并在每次可能搬迁或移动元素的操作后重新判断迭代器、引用和指针是否仍然有效。
```
