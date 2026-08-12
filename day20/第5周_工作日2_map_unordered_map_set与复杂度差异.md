# 第5周·工作日2：掌握 `map`、`unordered_map`、`set`及复杂度差异

## 0. 今天到底要完成什么

今天不要求你背诵 STL 的全部接口，而是完成下面四件事：

1. 能根据业务需求在 `map`、`unordered_map`和 `set`之间做选择；
2. 正确完成插入、查找、修改、删除和遍历；
3. 理解平均复杂度、最坏复杂度、哈希冲突和有序性之间的关系；
4. 完成一个可测试的模型注册表与音频标签索引。

今天的核心不是：

```text
unordered_map一定比map快
```

而是：

```text
根据是否需要顺序、范围查询、稳定复杂度和键唯一性选择容器
```

---

## 1. 前置知识与超纲说明

### 今天必须掌握

- 键、值、键值对的概念；
- `map<Key, Value>`；
- `unordered_map<Key, Value>`；
- `set<Key>`；
- `insert()`、`emplace()`、`find()`、`at()`、`erase()`；
- `map/unordered_map`的 `operator[]`行为；
- `size()`、`empty()`、`clear()`；
- `map`与 `set`的有序遍历；
- 平均复杂度和最坏复杂度；
- 迭代器、引用和指针的基本失效规则；
- `unordered_map`的 `reserve()`、桶和重新哈希概念。

### 今天首次系统接触

- 关联容器；
- 哈希表；
- 哈希冲突；
- 桶 `bucket`；
- 负载因子 `load_factor`；
- 重新哈希 `rehash`；
- 范围查询 `lower_bound/upper_bound`；
- `std::pair`和结构化绑定。

### 只要求认识，暂不要求独立实现

- 自定义比较器；
- 自定义哈希函数；
- 自定义键的相等判断；
- 透明比较器；
- 节点提取 `extract()`；
- `multimap`和 `multiset`；
- 并发哈希表。

如果示例使用这些内容，会在示例前再次提醒。

### C++版本提醒

今天继续使用：

```bash
-std=c++17
```

`contains()`是 C++20 接口。今天使用 C++17 时应写：

```cpp
container.find(key) != container.end()
```

不要直接使用：

```cpp
container.contains(key);
```

---

## 2. 今日目录结构

```text
day20/
├── include/
│   ├── model_info.hpp
│   └── model_registry.hpp
├── src/
│   ├── 01_map_basics.cpp
│   ├── 02_unordered_word_frequency.cpp
│   ├── 03_set_audio_tags.cpp
│   ├── 04_lookup_and_subscript.cpp
│   ├── 05_ordered_range_query.cpp
│   ├── 06_hash_bucket_observe.cpp
│   ├── 07_iterator_stability.cpp
│   ├── 08_lookup_benchmark.cpp
│   ├── 09_audio_metadata_index.cpp
│   ├── 10_model_registry.cpp
│   └── model_registry.cpp
├── tests/
│   └── associative_container_tests.cpp
├── target/
│   └── report/
│       └── associative_containers.md
└── 第5周_工作日2_map_unordered_map_set与复杂度差异.md
```

源码由你按照练习要求创建。本文不会直接给出综合项目的完整答案。

---

## 3. 建议学习时间

```text
45分钟：理解关联容器、键和值
60分钟：map与set
60分钟：unordered_map、哈希与桶
45分钟：复杂度和失效规则
120分钟：练习1～8
90分钟：综合项目和测试
30分钟：实验报告与复盘
```

如果当天时间不足，优先顺序为：

```text
map/unordered_map/set基本操作
    > operator[]陷阱
    > 复杂度差异
    > 失效规则
    > 桶实验
    > 性能测试
```

---

## 4. 什么是关联容器

昨天使用的 `vector`属于顺序容器：元素主要按位置组织。

```cpp
values[0];
values[1];
```

关联容器主要按“键”组织数据。

例如模型注册表：

```text
键                  值
qwen2.5-omni   ->   模型信息
whisper-large  ->   模型信息
```

你不需要记住某个模型在第几个位置，只需要通过名字查找：

```cpp
registry.find("qwen2.5-omni");
```

---

## 5. 三个容器分别解决什么问题

### `std::map<Key, Value>`

保存唯一键对应的值，并按照键的比较顺序排列。

```cpp
std::map<std::string, int> request_counts;
```

适合：

- 需要按键排序遍历；
- 需要范围查询；
- 需要稳定的对数复杂度；
- 不希望依赖哈希函数。

### `std::unordered_map<Key, Value>`

保存唯一键对应的值，通过哈希表组织，不保证遍历顺序。

```cpp
std::unordered_map<std::string, int> request_counts;
```

适合：

- 主要进行精确键查找；
- 不需要排序；
- 希望通常获得较快查找；
- 键具有合适的哈希函数。

### `std::set<Key>`

只保存唯一键，不额外保存映射值，并按照键排序。

```cpp
std::set<std::string> supported_formats;
```

适合：

- 去重；
- 判断某个值是否存在；
- 希望按顺序遍历唯一值；
- 需要范围查询。

---

## 6. 头文件

```cpp
#include <map>
#include <set>
#include <unordered_map>
```

常见配套头文件：

```cpp
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
```

`std::pair`和 `std::move`位于 `<utility>`。

---

## 7. `map`的基本结构

```cpp
std::map<std::string, int> sample_rates;
```

含义：

```text
Key   = std::string
Value = int
```

加入数据：

```cpp
sample_rates.insert({"meeting.wav", 16000});
sample_rates.emplace("music.wav", 44100);
```

概念上保存的是：

```cpp
std::pair<const std::string, int>
```

注意键具有 `const`性质，不能通过迭代器直接修改键。

---

## 8. 为什么 `map`的键不能直接修改

容器的组织结构依赖键的顺序。

如果允许把：

```text
meeting.wav
```

直接修改成：

```text
z.wav
```

原来的内部排序关系就可能被破坏。

因此不能写：

```cpp
iterator->first = "new.wav"; // 编译错误
```

但映射值可以修改：

```cpp
iterator->second = 48000;
```

需要改变键时，初学阶段使用：

```text
删除旧键
插入新键
```

---

## 9. `map`通常如何实现

`map`通常由平衡搜索树实现，常见实现是红黑树。

但要注意：

> C++标准规定的是行为和复杂度要求，并不强制实现必须使用红黑树。

可以用下面的抽象帮助理解：

```text
             music.wav
            /         \
   meeting.wav       noise.wav
```

树会维护键的比较顺序，因此：

- 查找通常沿树高进行；
- 插入后仍保持顺序；
- 遍历自然按键排序。

---

## 10. `map`的有序遍历

```cpp
std::map<std::string, int> sample_rates{
    {"noise.wav", 48000},
    {"meeting.wav", 16000},
    {"music.wav", 44100}
};
```

遍历时通常按照字符串比较顺序输出：

```text
meeting.wav
music.wav
noise.wav
```

遍历代码：

```cpp
for (const auto& entry : sample_rates) {
    std::cout
        << entry.first
        << ": "
        << entry.second
        << '\n';
}
```

这里的 `auto`会自动推导复杂的键值对类型。

---

## 11. 结构化绑定

这是 C++17语法：

```cpp
for (const auto& [path, sample_rate] : sample_rates) {
    std::cout << path << ' ' << sample_rate << '\n';
}
```

它把键值对拆分成两个名字：

```text
path        -> first
sample_rate -> second
```

当前只要求会读、会用于遍历，不要求研究其底层展开规则。

---

## 12. 插入及返回结果

### `insert()`

```cpp
const auto result =
    sample_rates.insert({"meeting.wav", 16000});
```

返回结果可以理解为：

```text
result.first  -> 指向对应元素的迭代器
result.second -> 是否真的插入成功
```

如果键已经存在，普通 `insert()`不会覆盖原值：

```cpp
sample_rates.insert({"meeting.wav", 48000});
```

原来的 `16000`仍然保留。

### `emplace()`

```cpp
sample_rates.emplace("music.wav", 44100);
```

它尝试直接在容器中构造元素。

对于今天的简单类型，`insert`和 `emplace`性能差别通常不是学习重点。先保证语义正确。

### `insert_or_assign()`

```cpp
sample_rates.insert_or_assign(
    "meeting.wav",
    48000
);
```

键不存在时插入，存在时覆盖对应值。

### `try_emplace()`

```cpp
registry.try_emplace(
    "qwen",
    /* 构造ModelInfo所需参数 */
);
```

键已经存在时，不会无意义地构造映射值。综合项目可以尝试，今天不要求深入模板细节。

---

## 13. `find()`查找

```cpp
const auto iterator =
    sample_rates.find("meeting.wav");

if (iterator != sample_rates.end()) {
    std::cout << iterator->second << '\n';
}
```

找不到时返回：

```cpp
sample_rates.end()
```

不能在未判断时直接解引用：

```cpp
auto iterator = sample_rates.find("missing.wav");
std::cout << iterator->second; // 找不到时属于错误
```

---

## 14. `operator[]`最重要的陷阱

```cpp
std::map<std::string, int> request_counts;

request_counts["qwen"];
```

如果键不存在，`operator[]`会插入该键，并对映射值进行默认初始化。

对于 `int`，通常得到：

```text
qwen -> 0
```

所以：

```cpp
++request_counts["qwen"];
```

很适合频次统计。

但是下面这种“只想查询”的写法会意外修改容器：

```cpp
if (request_counts["missing"] == 0) {
    // 容器中已经被插入missing
}
```

只查找、不插入时使用：

```cpp
find()
```

或者：

```cpp
at()
```

---

## 15. `at()`和 `operator[]`

```cpp
sample_rates.at("meeting.wav");
```

键不存在时抛出：

```cpp
std::out_of_range
```

对比：

| 接口 | 键存在 | 键不存在 | 是否可能修改容器 |
|---|---|---|---|
| `operator[]` | 返回值引用 | 插入默认值 | 是 |
| `at()` | 返回值引用 | 抛异常 | 否 |
| `find()` | 返回迭代器 | 返回 `end()` | 否 |

另外，`const map`不能调用 `operator[]`，因为它可能插入元素。

---

## 16. `erase()`删除

按键删除：

```cpp
const std::size_t removed =
    sample_rates.erase("meeting.wav");
```

返回被删除元素数量。对于唯一键容器，只会是：

```text
0 或 1
```

按迭代器删除并继续遍历：

```cpp
auto iterator = sample_rates.begin();

while (iterator != sample_rates.end()) {
    if (iterator->second < 16000) {
        iterator = sample_rates.erase(iterator);
    } else {
        ++iterator;
    }
}
```

`erase(iterator)`返回下一个有效迭代器。

---

## 17. `set`只保存唯一值

```cpp
std::set<std::string> tags;

tags.insert("speech");
tags.insert("music");
tags.insert("speech");
```

最后只有两个元素：

```text
music
speech
```

`set`自动去重，并按照比较顺序排列。

---

## 18. 为什么不能通过 `set`迭代器修改元素

`set`的元素本身就是键。

```cpp
auto iterator = tags.find("speech");
```

不能写：

```cpp
*iterator = "voice"; // 编译错误
```

因为修改后可能破坏容器内部顺序。

需要修改时使用：

```text
删除speech
插入voice
```

---

## 19. `set`常用操作

```cpp
std::set<std::string> tags;

tags.insert("speech");
tags.emplace("music");

const auto iterator = tags.find("speech");

if (iterator != tags.end()) {
    std::cout << "found\n";
}

tags.erase("speech");
```

判断存在还可以使用：

```cpp
if (tags.count("music") != 0) {
    // 存在
}
```

唯一键容器的 `count()`只可能返回0或1。

---

## 20. 有序容器的范围查询

这是 `map/set`相较于 `unordered_map`的重要优势。

```cpp
std::map<int, std::string> models{
    {1, "tiny"},
    {4, "small"},
    {8, "medium"},
    {16, "large"}
};
```

### `lower_bound(key)`

返回第一个“不小于 key”的元素。

```cpp
auto iterator = models.lower_bound(5);
```

得到键8。

### `upper_bound(key)`

返回第一个“严格大于 key”的元素。

```cpp
auto iterator = models.upper_bound(8);
```

得到键16。

查找 `[low, high]`可以写：

```cpp
auto begin = models.lower_bound(low);
auto end = models.upper_bound(high);
```

然后遍历 `[begin, end)`。

`unordered_map`没有这种按键顺序的范围查询能力。

---

## 21. `unordered_map`的基本使用

```cpp
std::unordered_map<std::string, int>
    request_counts;

++request_counts["qwen"];
++request_counts["whisper"];
++request_counts["qwen"];
```

结果：

```text
qwen    -> 2
whisper -> 1
```

查找：

```cpp
const auto iterator = request_counts.find("qwen");

if (iterator != request_counts.end()) {
    std::cout << iterator->second << '\n';
}
```

基本接口与 `map`相似，但内部组织方式不同。

---

## 22. 哈希函数是什么

哈希函数把键映射成一个哈希值：

```text
"qwen"    -> 某个整数
"whisper" -> 某个整数
```

哈希表再根据哈希值选择一个桶：

```text
bucket 0: ...
bucket 1: qwen
bucket 2: ...
bucket 3: whisper
```

理想情况下，可以快速定位到较小的桶中，而不需要按顺序检查所有元素。

标准库已经为很多基本类型提供 `std::hash`，包括：

- 整数；
- 指针；
- `std::string`；
- 一些其他标准类型。

所以字符串可以直接作为 `unordered_map`的键。

---

## 23. 哈希冲突

不同的键可能进入同一个桶：

```text
bucket 3:
    qwen
    whisper
    salmonn
```

这叫哈希冲突。

冲突不等于程序错误，哈希容器必须能够区分这些键。

查找通常分为：

1. 根据哈希函数定位桶；
2. 在桶内比较键是否相等。

如果大量键集中在少数桶，性能会下降。

因此 `unordered_map`只能保证平均情况下常数级查找，最坏情况下可能退化为线性复杂度。

---

## 24. 桶数量与负载因子

查看桶数量：

```cpp
table.bucket_count();
```

查看负载因子：

```cpp
table.load_factor();
```

可以近似理解为：

```text
元素数量 / 桶数量
```

查看允许的最大负载因子：

```cpp
table.max_load_factor();
```

当元素越来越多时，容器可能增加桶数量并重新组织元素，这叫重新哈希。

---

## 25. `unordered_map::reserve()`

如果提前知道大约会插入多少元素：

```cpp
request_counts.reserve(1000);
```

它会为预期元素数量准备足够的桶，以减少插入过程中的重新哈希。

注意它与 `vector::reserve()`概念相似但并不完全相同：

```text
vector::reserve      -> 预留连续元素存储容量
unordered_map::reserve -> 为预期元素数量调整桶结构
```

二者都可能让当前迭代器失效，所以不要机械地认为 `reserve()`永远安全。

---

## 26. `rehash()`

```cpp
table.rehash(128);
```

它请求容器调整为至少满足要求的桶数量。

初学阶段：

- 通常优先根据预期元素数量使用 `reserve()`；
- 使用桶实验理解 `rehash()`；
- 不要为了“优化”随意修改最大负载因子。

---

## 27. `unordered_map`不保证遍历顺序

```cpp
std::unordered_map<std::string, int> values{
    {"a", 1},
    {"b", 2},
    {"c", 3}
};
```

不能假设输出一定是：

```text
a b c
```

也不能假设每次运行、每个平台、每个标准库版本都得到相同顺序。

因此不要写依赖其遍历顺序的业务逻辑或测试。

如果输出必须稳定排序，可以：

- 使用 `map`；
- 或把键复制到 `vector`后排序；
- 或在测试中比较集合内容而不是遍历顺序。

---

## 28. 三个容器的复杂度

设容器中有 `n`个元素。

| 操作 | `map` | `unordered_map` | `set` |
|---|---:|---:|---:|
| 精确查找 | `O(log n)` | 平均 `O(1)`，最坏 `O(n)` | `O(log n)` |
| 插入 | `O(log n)` | 平均 `O(1)`，最坏 `O(n)` | `O(log n)` |
| 按键删除 | `O(log n)`附近 | 平均 `O(1)`，最坏 `O(n)` | `O(log n)`附近 |
| 有序遍历 | 支持 | 不保证顺序 | 支持 |
| 范围查询 | 支持 | 不适合 | 支持 |
| 去重 | 键唯一 | 键唯一 | 元素唯一 |

复杂度用于描述输入规模增长时的趋势，不直接等于某次运行的真实耗时。

---

## 29. 为什么 `O(1)`不代表永远更快

`unordered_map`平均查找为 `O(1)`，但仍然有成本：

- 计算哈希值；
- 访问桶；
- 处理冲突；
- 可能重新哈希；
- 桶数组占用额外内存；
- 缓存局部性可能不理想。

数据量非常小时，常数成本可能比复杂度差异更重要。

所以不要得出：

```text
unordered_map在所有场景都比map快
```

正确结论是：

```text
unordered_map通常适合大量精确查找且不要求顺序的场景
map适合有序遍历、范围查询和稳定对数复杂度的场景
```

---

## 30. 复杂度中的平均和最坏情况

### `map`

查找通常保证：

```text
O(log n)
```

这是较稳定的上界。

### `unordered_map`

平均：

```text
O(1)
```

最坏：

```text
O(n)
```

如果哈希分布很差，多个键集中到同一个桶，就需要逐个比较。

在涉及不可信输入的系统中，还需要注意恶意构造哈希冲突导致的性能问题。今天只理解风险，不要求处理安全哈希。

---

## 31. 内存开销差异

`map/set`的元素通常以独立节点组织，每个节点除了元素外还需要保存树结构相关信息。

`unordered_map`通常需要：

- 桶数组；
- 节点或链式结构；
- 哈希相关管理信息。

因此两者都不像 `vector`那样把所有元素紧密排列在一段连续内存中。

这意味着：

- 元素地址稳定性通常比 `vector`好；
- 内存开销通常比 `vector`大；
- 遍历的缓存局部性通常不如 `vector`。

---

## 32. `map/set`迭代器失效规则

对于 `map`和 `set`：

### 插入

```text
不会使已有元素的迭代器、引用和指针失效
```

### 删除

```text
只使指向被删除元素的迭代器、引用和指针失效
其他元素保持有效
```

### `clear()`

```text
全部元素被删除，全部元素定位失效
```

这与 `vector`插入可能导致整体重新分配有明显区别。

---

## 33. `unordered_map`迭代器失效规则

### 插入且没有重新哈希

```text
已有迭代器通常保持有效
```

### 插入导致重新哈希

```text
所有迭代器失效
```

### `reserve()`或 `rehash()`实际改变桶结构

```text
所有迭代器失效
```

### 删除

```text
只使被删除元素的迭代器失效
```

### 重要细节

重新哈希会使迭代器失效，但标准关联哈希容器中，未被删除元素的引用和指针不会仅因重新哈希而失效。

这比 `vector`的重新分配规则不同，必须分开记忆。

但是为了降低初学阶段误用风险，在可能重新哈希后，建议重新获取迭代器。

---

## 34. 失效规则对比

| 容器与操作 | 迭代器 | 元素引用/指针 |
|---|---|---|
| `map/set`插入 | 保持有效 | 保持有效 |
| `map/set`删除 | 仅被删元素失效 | 仅被删元素失效 |
| `unordered_map`插入，无 rehash | 保持有效 | 保持有效 |
| `unordered_map`发生 rehash | 全部迭代器失效 | 未删除元素仍有效 |
| `unordered_map`删除 | 被删元素失效 | 被删元素失效 |
| 任意容器 `clear()` | 全部失效 | 全部失效 |

不要把昨天的 `vector`失效规则直接套用到节点型关联容器。

---

## 35. 遍历时安全删除

适用于三个容器的基本写法：

```cpp
auto iterator = container.begin();

while (iterator != container.end()) {
    if (should_remove(*iterator)) {
        iterator = container.erase(iterator);
    } else {
        ++iterator;
    }
}
```

不要这样写：

```cpp
for (auto iterator = container.begin();
     iterator != container.end();
     ++iterator) {
    if (condition) {
        container.erase(iterator);
    }
}
```

删除后旧迭代器已经失效，循环尾部继续 `++iterator`属于错误。

---

## 36. 选择容器的决策流程

```text
是否只需要保存唯一值？
├── 是
│   ├── 需要有序/范围查询 -> set
│   └── 不需要有序       -> unordered_set（后续补充）
└── 否，需要键映射到值
    ├── 需要有序遍历或范围查询 -> map
    └── 主要精确查找且不要求顺序 -> unordered_map
```

今天重点学习 `set`，`unordered_set`与 `unordered_map`的哈希思想相似，后续可以很快补齐。

---

## 37. AudioLLM工程中的典型用法

### 模型名称到模型信息

```cpp
std::unordered_map<std::string, ModelInfo>
    model_registry;
```

主要按唯一模型名精确查找，不要求遍历排序。

### 延迟阈值到策略名称

```cpp
std::map<int, std::string> latency_policies;
```

需要根据延迟范围选择策略，可以使用 `lower_bound()`。

### 支持的音频标签

```cpp
std::set<std::string> supported_tags;
```

用于去重并稳定输出。

### 请求频次统计

```cpp
std::unordered_map<std::string, std::size_t>
    request_counts;
```

```cpp
++request_counts[model_name];
```

---

## 38. 常见错误

### 错误1：使用 `operator[]`只做查询

它可能插入新键。

### 错误2：认为 `insert()`一定覆盖旧值

唯一键已存在时，普通 `insert()`不会覆盖。

### 错误3：解引用 `find()`返回的 `end()`

必须先判断。

### 错误4：依赖 `unordered_map`遍历顺序

顺序不受保证。

### 错误5：认为平均 `O(1)`等于永远一步完成

哈希、冲突和重新哈希都有成本。

### 错误6：重新哈希后继续使用旧迭代器

旧迭代器全部失效。

### 错误7：直接修改 `map/set`的键

键关系决定内部结构，不能直接修改。

### 错误8：删除迭代器后继续递增旧迭代器

应接收 `erase()`返回值。

### 错误9：把复杂度当作绝对运行时间

复杂度只描述随规模增长的趋势。

### 错误10：使用浮点数作为哈希键却期待“近似相等”

哈希键使用精确相等语义，初学阶段避免这样设计。

---

## 39. 练习1：`map`基础操作

对应文件：`src/01_map_basics.cpp`

创建：

```cpp
std::map<std::string, int> sample_rates;
```

完成：

- 插入 `meeting.wav -> 16000`；
- 插入 `music.wav -> 44100`；
- 插入 `noise.wav -> 48000`；
- 按键顺序遍历；
- 使用 `find()`查找存在项；
- 使用 `find()`查找不存在项；
- 修改 `meeting.wav`的值；
- 删除 `noise.wav`；
- 打印 `size()`。

必须解释：

```text
为什么遍历顺序不是插入顺序
```

---

## 40. 练习2：词频统计

对应文件：`src/02_unordered_word_frequency.cpp`

输入字符串序列：

```text
audio model audio inference model audio
```

使用：

```cpp
std::unordered_map<std::string, std::size_t>
```

统计频次。

要求：

- 使用 `operator[]`完成计数；
- 使用 `find()`完成查询；
- 不依赖遍历顺序；
- 验证 `audio == 3`；
- 验证 `model == 2`；
- 验证 `inference == 1`；
- 说明这里为什么适合 `unordered_map`。

---

## 41. 练习3：音频标签去重

对应文件：`src/03_set_audio_tags.cpp`

输入：

```text
speech music speech noise music environment
```

使用：

```cpp
std::set<std::string>
```

要求：

- 插入全部标签；
- 输出去重后数量；
- 按顺序遍历；
- 查找 `speech`；
- 查找 `unknown`；
- 删除 `noise`；
- 解释为何不能通过迭代器把 `music`改成 `voice`。

---

## 42. 练习4：`operator[]`查询陷阱

对应文件：`src/04_lookup_and_subscript.cpp`

创建：

```cpp
std::map<std::string, int> counts{
    {"qwen", 3}
};
```

完成并观察：

1. 记录初始 `size()`；
2. 执行 `counts["missing"]`；
3. 再次打印 `size()`；
4. 使用 `find("another_missing")`；
5. 再次打印 `size()`；
6. 使用 `at("missing")`；
7. 对不存在键调用 `at()`并捕获异常。

最终写出三种接口的语义差异。

---

## 43. 练习5：有序范围查询

对应文件：`src/05_ordered_range_query.cpp`

创建显存需求到模型名的映射：

```text
2  -> tiny
4  -> small
8  -> medium
16 -> large
24 -> xlarge
```

使用：

```cpp
std::map<int, std::string>
```

要求：

- 查找第一个显存需求不低于6 GB的模型；
- 输出显存需求在 `[4, 16]`范围内的全部模型；
- 分别使用 `lower_bound()`和 `upper_bound()`；
- 处理返回 `end()`的情况；
- 解释为什么 `unordered_map`不适合这个需求。

---

## 44. 练习6：观察哈希桶

对应文件：`src/06_hash_bucket_observe.cpp`

向空 `unordered_map<int, std::string>`插入至少50个元素。

只在桶数量变化时记录：

```text
size
bucket_count
load_factor
max_load_factor
```

然后：

1. 清空并重新创建容器；
2. 调用 `reserve(50)`；
3. 再插入50个元素；
4. 比较桶数量变化次数；
5. 不依赖某个固定桶增长倍率。

报告中解释：

```text
reserve为什么可能减少rehash
```

---

## 45. 练习7：迭代器稳定性实验

对应文件：`src/07_iterator_stability.cpp`

分别完成两组实验。

### A. `map`插入

- 保存一个已有元素的迭代器、引用和地址；
- 插入100个新元素；
- 验证旧元素定位仍有效；
- 不删除该元素。

### B. `unordered_map`重新哈希

- 保存一个元素迭代器；
- 保存该元素映射值的指针；
- 记录 `bucket_count()`；
- 插入元素直到桶数量变化；
- 重新哈希后禁止使用旧迭代器；
- 重新通过 `find()`获取迭代器；
- 验证未删除元素的指针仍指向对应值；
- 在报告中解释“迭代器失效但引用/指针仍有效”的差异。

注意：不要通过运行结果证明未定义行为，不要解引用失效迭代器。

---

## 46. 练习8：查找性能实验

对应文件：`src/08_lookup_benchmark.cpp`

分别构造：

```cpp
std::map<int, int>
std::unordered_map<int, int>
```

插入相同的至少10万个整数键值对，并完成相同次数的查找。

要求：

- 使用 `<chrono>`计时；
- 使用相同输入；
- 分开统计构建时间和查找时间；
- 使用查找结果，避免测试代码被无意义优化；
- 至少运行3次；
- 记录编译选项；
- 不能根据一次实验断言某容器永远更快；
- 报告复杂度与实测时间的区别。

`<chrono>`计时属于今天的辅助内容，不要求深入时钟实现。

---

## 47. 练习9：音频元数据索引

对应文件：`src/09_audio_metadata_index.cpp`

定义简单结构体：

```cpp
struct AudioMetadata {
    std::string path;
    int sample_rate;
    double duration_seconds;
};
```

构造：

```cpp
std::unordered_map<std::string, AudioMetadata>
```

以音频路径作为键。

要求：

- 插入3条元数据；
- 拒绝或明确处理重复路径；
- 根据路径查询；
- 查询不存在路径时不插入新元素；
- 删除指定路径；
- 统计各采样率文件数量；
- 使用 `set<std::string>`保存去重后的扩展名；
- 说明为什么主索引使用 `unordered_map`。

---

## 48. 练习10：模型注册表综合项目

对应文件：

- 数据结构：`include/model_info.hpp`
- 类声明：`include/model_registry.hpp`
- 类实现：`src/model_registry.cpp`
- 运行示例：`src/10_model_registry.cpp`
- 单元测试：`tests/associative_container_tests.cpp`
- 实验报告：`target/report/associative_containers.md`

### `ModelInfo`至少保存

```text
模型名称
后端类型
版本
是否支持音频
最大并发数
```

### `ModelRegistry`内部至少包含

```cpp
std::unordered_map<std::string, ModelInfo>
```

模型名称作为唯一键。

另外使用：

```cpp
std::set<std::string>
```

保存全部后端类型。

### 必须实现的接口

```text
reserve
register_model
find_model
remove_model
contains_model
size
backend_types
```

### 行为要求

- 空模型名注册失败；
- 重复模型名注册失败，不静默覆盖；
- `find_model`找不到时返回 `nullptr`；
- 查询操作不能意外插入元素；
- 删除存在项返回成功；
- 删除不存在项返回失败；
- 注册与删除后后端类型集合保持业务一致；
- 不依赖 `unordered_map`遍历顺序；
- 不使用拥有型裸 `new/delete`；
- 对外返回指针时，在说明文档中写明删除对应元素后指针失效。

### 建议注册数据

```text
qwen2.5-omni  qwen-runtime     v1  true  4
whisper-large whisper-runtime v3  true  8
text-embedder onnx-runtime    v2  false 16
```

---

## 49. 单元测试要求

文件：`tests/associative_container_tests.cpp`

至少覆盖：

```text
1. map插入后size正确
2. map按键有序遍历
3. 普通insert不覆盖重复键
4. insert_or_assign可以更新值
5. find不存在项返回end
6. operator[]访问不存在项会增加size
7. at不存在项抛out_of_range
8. set自动去重
9. set遍历有序
10. unordered_map频次统计正确
11. unordered_map测试不依赖遍历顺序
12. reserve后可插入预期数量数据
13. map插入不破坏旧元素定位
14. erase返回下一有效迭代器
15. ModelRegistry成功注册模型
16. ModelRegistry拒绝空名称
17. ModelRegistry拒绝重复名称
18. ModelRegistry可查询存在模型
19. ModelRegistry查询不存在项不改变size
20. ModelRegistry删除行为正确
21. backend_types去重正确
22. 正式测试在ASan/UBSan下零错误
```

如果你当天不想引入 GoogleTest，可以先使用：

```cpp
#include <cassert>
```

完成基础断言。你已经学习过单元测试后，也可以继续使用 GoogleTest，但不要把测试框架配置问题和今天的容器知识混在一起。

---

## 50. 编译命令

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
  src/01_map_basics.cpp \
  -o target/01_map_basics
```

运行：

```bash
./target/01_map_basics
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
  src/10_model_registry.cpp \
  src/model_registry.cpp \
  -Iinclude \
  -o target/10_model_registry
```

运行：

```bash
./target/10_model_registry
```

注意每个编译选项必须作为独立参数，不能写成：

```text
"-Wall -Wextra -Wpedantic"
```

---

## 51. 性能测试编译说明

理解代码行为时使用：

```bash
-O0
```

进行性能对比时还应再编译一个优化版本：

```bash
clang++ \
  -std=c++17 \
  -O2 \
  -DNDEBUG \
  -Wall \
  -Wextra \
  -Wpedantic \
  src/08_lookup_benchmark.cpp \
  -o target/08_lookup_benchmark_release
```

不要使用开启 Sanitizer 的程序做正式性能结论，因为 Sanitizer会增加运行成本。

因此分开：

```text
正确性验证 -> ASan/UBSan + -O0
性能观察   -> 不开Sanitizer + -O2
```

---

## 52. ASan能发现什么、不能发现什么

ASan可能发现：

- 删除元素后继续使用其指针；
- 访问已释放对象；
- 越界访问；
- 部分悬空引用错误。

ASan不保证发现：

- 所有失效迭代器操作；
- 依赖 `unordered_map`顺序的逻辑错误；
- 错误选择容器导致的性能问题；
- `operator[]`意外插入造成的业务错误；
- 平均复杂度退化问题。

所以：

```text
Sanitizer零错误
不等于
容器逻辑和复杂度选择正确
```

---

## 53. 实验报告模板

文件：`target/report/associative_containers.md`

建议内容：

```markdown
# 关联容器实验报告

## 环境

- 编译器：
- C++标准：C++17
- 操作系统：
- 正确性编译参数：
- 性能编译参数：

## map、unordered_map和set选择

- map适用场景：
- unordered_map适用场景：
- set适用场景：

## 复杂度

- map查找：
- unordered_map平均查找：
- unordered_map最坏查找：
- set查找：

## operator[]实验

- 操作前size：
- 操作后size：
- 原因：

## 桶与rehash实验

- 未reserve时桶变化次数：
- reserve后桶变化次数：
- load_factor观察：

## 迭代器失效

- map插入：
- map删除：
- unordered_map rehash：
- unordered_map删除：

## 性能实验

- 数据量：
- map构建/查找时间：
- unordered_map构建/查找时间：
- 为什么本结果不能推广到所有场景：

## ModelRegistry设计

- 为什么使用unordered_map：
- 为什么后端类型使用set：
- 查找失败语义：
- 返回指针失效边界：

## Sanitizer结果

- ASan：
- UBSan：

## 今日结论
```

---

## 54. 今日思考题

1. `map`与 `unordered_map`都能保存键值对，它们最核心的区别是什么？
2. 为什么 `map`遍历有序？
3. C++标准是否强制 `map`使用红黑树？
4. 为什么 `unordered_map`遍历顺序不能依赖？
5. `operator[]`查找不存在键时发生什么？
6. 为什么 `const map`不能调用 `operator[]`？
7. `find()`失败时返回什么？
8. 为什么不能直接解引用 `end()`？
9. 普通 `insert()`遇到重复键会覆盖吗？
10. `insert_or_assign()`有什么不同？
11. `set`为什么会自动去重？
12. 为什么不能通过 `set`迭代器修改元素？
13. `lower_bound()`和 `upper_bound()`分别返回什么？
14. 哪个容器适合范围查询？
15. `unordered_map`平均 `O(1)`为什么不是最坏 `O(1)`？
16. 什么是哈希冲突？
17. 什么是桶？
18. 什么是负载因子？
19. `unordered_map::reserve()`主要准备什么？
20. 重新哈希会使哪些对象失效？
21. 为什么重新哈希后元素指针仍可能有效，而迭代器失效？
22. `map`插入会使旧迭代器失效吗？
23. `map`删除会使哪些定位失效？
24. 遍历删除时为什么要接收 `erase()`返回值？
25. 为什么不能只根据一次基准测试选择容器？
26. 为什么大O复杂度不等于实际毫秒数？
27. 模型注册表为什么通常适合 `unordered_map`？
28. 支持标签为什么可以用 `set`？
29. 延迟阈值策略为什么可能适合 `map`？
30. ASan为什么无法发现 `operator[]`意外插入？

---

## 55. 今日验收清单

### 知识

- [ ] 能解释键和值；
- [ ] 能说出三个容器的主要用途；
- [ ] 能解释有序与无序；
- [ ] 能说出核心复杂度；
- [ ] 能解释平均和最坏情况；
- [ ] 能解释哈希、桶、冲突和 rehash；
- [ ] 能解释 `operator[]`的插入行为；
- [ ] 能解释三个容器的失效规则。

### 编码

- [ ] 完成 `01_map_basics.cpp`；
- [ ] 完成 `02_unordered_word_frequency.cpp`；
- [ ] 完成 `03_set_audio_tags.cpp`；
- [ ] 完成 `04_lookup_and_subscript.cpp`；
- [ ] 完成 `05_ordered_range_query.cpp`；
- [ ] 完成 `06_hash_bucket_observe.cpp`；
- [ ] 完成 `07_iterator_stability.cpp`；
- [ ] 完成 `08_lookup_benchmark.cpp`；
- [ ] 完成 `09_audio_metadata_index.cpp`；
- [ ] 完成模型注册表项目。

### 测试与输出

- [ ] 完成至少22项测试；
- [ ] 正式测试ASan/UBSan零错误；
- [ ] 性能测试与Sanitizer测试分开；
- [ ] 完成实验报告；
- [ ] 提交Git仓库。

---

## 56. 今天结束后应该能回答的面试问题

```text
map和unordered_map有什么区别？
为什么unordered_map平均O(1)但最坏O(n)？
map为什么适合范围查询？
operator[]与find/at有什么区别？
unordered_map什么时候发生rehash？
rehash会使哪些对象失效？
set如何实现去重？
如何在遍历关联容器时安全删除元素？
如何选择map、unordered_map和set？
```

回答时不要只背复杂度，还应该说明：

- 是否需要有序；
- 是否需要范围查询；
- 哈希质量；
- 最坏情况要求；
- 内存开销；
- 迭代器稳定性；
- 真实数据规模。

---

## 57. 与就业方向的联系

### AI部署与推理服务

关联容器常用于：

- 模型注册表；
- 请求ID索引；
- 会话状态；
- 配置项；
- 统计计数；
- 后端能力集合；
- 缓存元数据。

### C++后端保底方向

面试常问：

- STL容器选择；
- 红黑树和哈希表的抽象差异；
- 复杂度；
- 迭代器失效；
- 哈希冲突；
- rehash；
- `operator[]`副作用。

今天的内容同时服务于 AI工程化主线和 C++工程后路。

---

## 58. 今日产出

至少应形成：

```text
9个独立练习源文件
1个ModelRegistry综合项目
1个测试文件
1份关联容器实验报告
1次可复现Git提交
```

综合项目可以在后续阶段继续扩展为：

```text
模型名称
   ↓
ModelRegistry
   ↓
推理后端对象
   ↓
推理服务调度
```

---

## 59. Git提交建议

```bash
git status
git add day20
git commit -m "Learn associative containers and build model registry"
```

提交前检查：

- 不提交 `target`中的二进制文件；
- 实验报告可以提交；
- 不提交系统临时文件；
- README或学习笔记写明编译方法；
- 正式测试不能包含故意触发未定义行为的代码。

---

## 60. 最终速记

```text
map：唯一键值对，有序，核心操作O(log n)，支持范围查询

unordered_map：唯一键值对，无序，平均O(1)，最坏O(n)

set：唯一值，有序，核心操作O(log n)

operator[]：不存在时会插入默认值

find：只查找，失败返回end

map/set插入：旧定位保持有效

unordered_map rehash：旧迭代器全部失效，但未删除元素引用/指针保持有效

遍历删除：iterator = container.erase(iterator)

选择容器：先看语义和查询需求，再看复杂度
```
