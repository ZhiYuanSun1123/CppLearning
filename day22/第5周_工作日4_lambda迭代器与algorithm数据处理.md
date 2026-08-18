# 第5周·工作日4：使用 Lambda、迭代器和 `<algorithm>`完成数据处理练习

## 0. 今天要完成什么

昨天已经接触了部分标准算法。今天不再把算法当成零散函数，而是把它们组合成一条数据处理流水线：

```text
原始数据
  ↓ 校验与筛选
有效数据
  ↓ 转换与标准化
统一数据
  ↓ 排序与去重
可用数据集
  ↓ 统计与输出
处理报告
```

今天结束后，你应该能够：

1. 独立编写有捕获和无捕获 Lambda；
2. 理解迭代器、`const_iterator`和半开区间；
3. 根据算法要求判断迭代器是否满足条件；
4. 正确组合 `copy_if`、`transform`、`sort`、`unique`、`remove_if`和 `accumulate`；
5. 避免迭代器失效、输出空间不足和 Lambda悬空捕获；
6. 完成一个音频元数据清洗流水线项目。

---

## 1. 前置知识与超纲说明

### 今天必须掌握

- Lambda基本语法；
- 空捕获、按值捕获、按引用捕获和明确捕获；
- Lambda参数、返回值和 `const`调用的直观含义；
- `begin/end`与 `[first,last)`；
- `iterator`和 `const_iterator`；
- `cbegin/cend`；
- `std::next`、`std::prev`、`std::advance`、`std::distance`；
- 不同迭代器能力存在差异；
- `for_each/find_if/copy_if/transform`；
- `sort/stable_sort/partition/stable_partition`；
- `remove_if/unique`不会自行缩小 `vector`；
- erase-remove和 sort-unique-erase；
- `accumulate`初始值类型；
- 算法执行期间修改容器结构的风险。

### 今天首次系统学习

- Lambda捕获列表；
- `mutable` Lambda；
- 泛型 Lambda；
- 迭代器类别；
- 输入范围与输出迭代器；
- `back_inserter`；
- 算法流水线；
- 分区与排序的区别；
- 相邻去重；
- 投影式思考：对结构体的某个字段处理。

### 超纲内容

以下内容只介绍用途，不要求今天掌握底层实现：

- Lambda闭包对象的真实编译器生成类型；
- 完美转发；
- `std::function`类型擦除；
- 迭代器 traits；
- 自定义迭代器；
- C++20 ranges和 views；
- 并行算法；
- 执行策略；
- SIMD与算法向量化。

### C++版本提醒

继续使用：

```bash
-std=c++17
```

今天不使用 C++20 的：

```cpp
std::ranges
std::erase_if
```

---

## 2. 今日目录结构

```text
day22/
├── include/
│   ├── audio_record.hpp
│   └── audio_dataset_processor.hpp
├── src/
│   ├── 01_lambda_basics.cpp
│   ├── 02_lambda_capture.cpp
│   ├── 03_iterator_operations.cpp
│   ├── 04_find_copy_filter.cpp
│   ├── 05_transform_normalize.cpp
│   ├── 06_sort_partition.cpp
│   ├── 07_remove_unique.cpp
│   ├── 08_algorithm_pipeline.cpp
│   ├── 09_capture_lifetime.cpp
│   ├── 10_audio_dataset_processor.cpp
│   └── audio_dataset_processor.cpp
├── tests/
│   └── lambda_algorithm_tests.cpp
├── target/
│   └── report/
│       └── audio_data_pipeline.md
└── 第5周_工作日4_lambda迭代器与algorithm数据处理.md
```

每道题下面都有明确文件名。你自己完成源码，文档不提供综合项目完整答案。

---

## 3. 建议时间

```text
60分钟：Lambda语法和捕获
60分钟：迭代器与区间
90分钟：筛选、转换、排序和删除算法
120分钟：练习1～9
90分钟：综合项目
30分钟：测试、报告与复盘
```

优先级：

```text
Lambda明确捕获
> 迭代器范围与失效
> copy_if/transform/remove_if
> sort/unique
> 综合流水线
```

---

## 4. Lambda是什么

Lambda是在使用位置定义的可调用对象。

```cpp
const auto is_positive = [](int value) {
    return value > 0;
};

std::cout << is_positive(10) << '\n';
```

基本结构：

```text
[捕获列表](参数列表) -> 返回类型 {
    函数体
}
```

多数简单情况下返回类型可以自动推导：

```cpp
[](int value) {
    return value * 2;
}
```

Lambda经常作为标准算法的谓词、转换器或比较器。

---

## 5. 无捕获 Lambda

```cpp
const auto is_even = [](int value) {
    return value % 2 == 0;
};
```

捕获列表为空：

```cpp
[]
```

意味着函数体不直接使用外部局部变量。

传给算法：

```cpp
const auto count = std::count_if(
    values.begin(),
    values.end(),
    is_even
);
```

---

## 6. 按值捕获

```cpp
const int threshold = 100;

const auto is_slow = [threshold](int latency) {
    return latency > threshold;
};
```

Lambda对象保存创建时 `threshold`的副本。

```cpp
int threshold = 100;
auto predicate = [threshold](int value) {
    return value > threshold;
};

threshold = 1000;
```

`predicate`内部仍使用捕获时的100。

按值捕获适合：

- 小型只读配置；
- Lambda可能离开当前作用域；
- 不希望外部变量后续变化影响行为。

---

## 7. 按引用捕获

```cpp
std::size_t invalid_count = 0;

std::for_each(
    records.begin(),
    records.end(),
    [&invalid_count](const AudioRecord& record) {
        if (record.duration_seconds <= 0.0) {
            ++invalid_count;
        }
    }
);
```

Lambda访问外部原对象，所以修改会反映到外部变量。

风险：

> Lambda不能活得比被引用变量更久。

错误思想：

```cpp
auto make_predicate() {
    int threshold = 100;
    return [&threshold](int value) {
        return value > threshold;
    };
}
```

函数返回后 `threshold`已销毁，Lambda中的引用悬空。

应改为按值捕获：

```cpp
return [threshold](int value) {
    return value > threshold;
};
```

---

## 8. 默认捕获 `[=]`与 `[&]`

```cpp
[=] { /* 使用到的外部局部变量按值捕获 */ }
[&] { /* 使用到的外部局部变量按引用捕获 */ }
```

它们写起来短，但会隐藏依赖。

学习和工程代码中优先明确捕获：

```cpp
[threshold, &invalid_count]
```

这样一眼可以看出：

- `threshold`是副本；
- `invalid_count`会被修改。

---

## 9. 捕获 `this`

成员函数中的 Lambda可能需要访问成员：

```cpp
class Processor {
public:
    bool accepts(double duration) const {
        return duration >= minimum_duration_;
    }

private:
    double minimum_duration_ = 1.0;
};
```

在成员函数中可能看到：

```cpp
[this](const AudioRecord& record) {
    return record.duration_seconds >= minimum_duration_;
}
```

这里捕获的是 `this`指针，不会自动延长当前对象生命周期。如果 Lambda比对象活得久，仍可能悬空。

这是今天的边界提醒，生命周期管理后续会结合回调继续学习。

---

## 10. `mutable` Lambda

按值捕获的副本默认不能在 Lambda函数体内修改：

```cpp
int count = 0;

auto next = [count]() mutable {
    return ++count;
};
```

这里修改的是 Lambda内部副本，不是外部 `count`。

```cpp
std::cout << next() << '\n'; // 1
std::cout << next() << '\n'; // 2
std::cout << count << '\n';  // 0
```

今天要会读懂，但数据统计通常更建议明确按引用捕获结果变量，或者使用算法返回值。

---

## 11. Lambda返回类型

简单情况下自动推导：

```cpp
[](double seconds) {
    return seconds * 1000.0;
}
```

需要明确时：

```cpp
[](double seconds) -> long long {
    return static_cast<long long>(
        seconds * 1000.0
    );
}
```

如果不同分支返回类型不一致，可能无法推导或发生不期望转换。优先保证所有返回路径语义一致。

---

## 12. 泛型 Lambda

C++14起可以使用 `auto`参数：

```cpp
const auto less_than = [](const auto& left,
                          const auto& right) {
    return left < right;
};
```

这相当于可处理多种匹配类型，背后涉及模板实例化。

今天只要求认识，不建议所有地方都用 `auto`隐藏业务类型。处理 `AudioRecord`时，写出明确类型通常更清晰。

---

## 13. 什么是迭代器

迭代器是用于定位和访问容器元素的对象。

```cpp
auto iterator = values.begin();
```

解引用访问元素：

```cpp
std::cout << *iterator << '\n';
```

移动到下一个位置：

```cpp
++iterator;
```

判断是否到达终点：

```cpp
iterator != values.end()
```

它的使用形式类似指针，但不要简单认为所有迭代器都是裸指针。

---

## 14. `begin/end`与半开区间

```cpp
values.begin()
values.end()
```

表示：

```text
[begin, end)
```

`end()`指向最后一个元素之后的位置，不能解引用。

空容器满足：

```cpp
values.begin() == values.end()
```

半开区间的优势：

- 空范围自然表达；
- 元素数量可由距离表示；
- 子区间容易拼接；
- 循环终止条件统一。

---

## 15. `iterator`与 `const_iterator`

普通迭代器允许修改元素：

```cpp
std::vector<int>::iterator iterator = values.begin();
*iterator = 100;
```

常量迭代器不允许通过它修改元素：

```cpp
std::vector<int>::const_iterator iterator =
    values.cbegin();
```

```cpp
*iterator = 100; // 编译错误
```

只读处理优先使用：

```cpp
cbegin()
cend()
```

注意：`const_iterator`限制的是通过迭代器修改元素，不代表容器本身一定是 `const`。

---

## 16. `auto`为什么常用于迭代器

完整类型可能很长：

```cpp
std::vector<AudioRecord>::const_iterator
```

可以写：

```cpp
const auto iterator = std::find_if(
    records.cbegin(),
    records.cend(),
    predicate
);
```

`const auto iterator`表示迭代器变量本身不能重新指向别处，不等同于 `const_iterator`。

对比：

```cpp
auto iterator = records.cbegin();
```

这里根据 `cbegin()`推导出的才是常量迭代器类型。

---

## 17. 迭代器能力分类

可以按能力从弱到强理解：

```text
输入迭代器
  ↓
前向迭代器
  ↓
双向迭代器
  ↓
随机访问迭代器
```

另外还有输出迭代器用于写入结果。

### 输入迭代器

主要向前读取一次。

### 前向迭代器

可以多次向前遍历。

### 双向迭代器

既能 `++`也能 `--`。

### 随机访问迭代器

支持：

```cpp
iterator + n
iterator - n
iterator[n]
```

`vector/array`提供随机访问迭代器；`map/set`提供双向迭代器。

所以：

```cpp
std::sort(map.begin(), map.end());
```

不能编译，因为 `sort()`要求随机访问迭代器，而且 `map`本身已经按键维护顺序。

---

## 18. 不要对所有迭代器使用 `+ n`

对 `vector`可以：

```cpp
auto iterator = values.begin() + 3;
```

对 `map`不能这样写：

```cpp
auto iterator = mapping.begin() + 3; // 错误
```

通用推进可以使用：

```cpp
std::advance(iterator, 3);
```

但复杂度取决于迭代器类型：

- `vector`随机访问通常 `O(1)`；
- `map`双向推进3步需要逐步移动。

---

## 19. `next()`、`prev()`和 `advance()`

### `std::next()`

返回移动后的新迭代器，不修改原变量：

```cpp
auto third = std::next(values.begin(), 2);
```

### `std::prev()`

向前移动：

```cpp
auto last = std::prev(values.end());
```

空容器不能这样获取最后元素。

### `std::advance()`

直接修改传入迭代器：

```cpp
auto iterator = values.begin();
std::advance(iterator, 2);
```

头文件：

```cpp
#include <iterator>
```

移动不能越过合法范围，否则可能产生未定义行为。

---

## 20. `std::distance()`

```cpp
const auto distance = std::distance(
    values.begin(),
    iterator
);
```

它计算从第一个迭代器到第二个迭代器所需的步数。

对随机访问迭代器通常是 `O(1)`；对其他迭代器可能是 `O(n)`。

两个迭代器必须来自同一个有效范围，并且第二个位置对当前迭代器类别必须可达。

---

## 21. 反向迭代器

```cpp
for (auto iterator = values.crbegin();
     iterator != values.crend();
     ++iterator) {
    std::cout << *iterator << '\n';
}
```

反向迭代器中仍然使用 `++iterator`，含义是沿反向遍历方向前进。

今天只要求会遍历，不深入 `base()`的边界对应规则。

---

## 22. 输出迭代器与 `back_inserter`

`copy_if()`需要一个输出位置。如果目标 `vector`为空，不能把 `begin()`当作可写空间：

```cpp
std::vector<int> output;

std::copy_if(
    input.begin(),
    input.end(),
    output.begin(), // 错误：没有元素空间
    predicate
);
```

可以使用：

```cpp
std::back_inserter(output)
```

完整写法：

```cpp
std::copy_if(
    input.begin(),
    input.end(),
    std::back_inserter(output),
    predicate
);
```

它会在写入时调用 `output.push_back()`。

头文件：

```cpp
#include <iterator>
```

---

## 23. 迭代器失效必须继续遵守

算法不会让容器的失效规则消失。

错误示例：

```cpp
std::for_each(
    values.begin(),
    values.end(),
    [&values](int value) {
        values.push_back(value);
    }
);
```

`push_back()`可能重新分配，并且旧 `end()`也会改变，算法正在使用的范围因此失效。

安全原则：

> 算法正在遍历一个容器时，不要在回调中改变该容器的结构。

修改元素值通常可以，但要确认算法语义允许，且不能破坏排序或关联容器键的不变量。

---

## 24. `for_each()`

```cpp
std::for_each(
    values.begin(),
    values.end(),
    [](int& value) {
        value *= 2;
    }
);
```

适合对每个元素执行有副作用的操作。

如果目标是生成另一组数据，通常 `transform()`更能表达意图；如果只是统计，优先考虑 `count_if/accumulate`等返回结果的算法。

不要为了“使用算法”把所有简单范围 `for`都替换为 `for_each()`。

---

## 25. `find_if()`和 `find_if_not()`

寻找第一个满足条件的元素：

```cpp
auto iterator = std::find_if(
    records.cbegin(),
    records.cend(),
    [](const AudioRecord& record) {
        return record.duration_seconds > 60.0;
    }
);
```

寻找第一个不满足条件的元素：

```cpp
auto invalid = std::find_if_not(
    records.cbegin(),
    records.cend(),
    is_valid
);
```

都必须在解引用前与 `end()`比较。

---

## 26. `copy()`与 `copy_if()`

复制全部：

```cpp
std::copy(
    input.cbegin(),
    input.cend(),
    std::back_inserter(output)
);
```

按条件复制：

```cpp
std::copy_if(
    input.cbegin(),
    input.cend(),
    std::back_inserter(valid_records),
    is_valid
);
```

如果输出就是新的筛选结果，`copy_if`通常比“先复制全部再删除”表达得更直接。

如果源和目标区间发生不受支持的重叠，可能导致错误；今天使用独立目标容器。

---

## 27. `transform()`

单输入转换：

```cpp
std::transform(
    records.cbegin(),
    records.cend(),
    std::back_inserter(durations_ms),
    [](const AudioRecord& record) {
        return record.duration_seconds * 1000.0;
    }
);
```

输入多少元素，通常写出多少结果。

也可以原地修改：

```cpp
std::transform(
    values.begin(),
    values.end(),
    values.begin(),
    [](int value) {
        return value * 2;
    }
);
```

原地转换时不要在 Lambda中改变容器大小。

---

## 28. `sort()`与结构体字段

```cpp
std::sort(
    records.begin(),
    records.end(),
    [](const AudioRecord& left,
       const AudioRecord& right) {
        return left.duration_seconds
             < right.duration_seconds;
    }
);
```

比较器必须形成一致的严格顺序。

错误：

```cpp
return left.duration_seconds
    <= right.duration_seconds;
```

相等对象互相比较时会返回 `true`，破坏严格性。

---

## 29. 多关键字排序

要求：采样率升序；采样率相同时路径升序。

```cpp
[](const AudioRecord& left,
   const AudioRecord& right) {
    if (left.sample_rate != right.sample_rate) {
        return left.sample_rate < right.sample_rate;
    }

    return left.path < right.path;
}
```

也可以使用 `std::tie`，但元组比较属于扩展内容。今天优先写清楚分支。

---

## 30. `stable_sort()`

如果只按采样率排序，同时希望相同采样率的记录保持原顺序：

```cpp
std::stable_sort(
    records.begin(),
    records.end(),
    [](const AudioRecord& left,
       const AudioRecord& right) {
        return left.sample_rate < right.sample_rate;
    }
);
```

稳定性是业务语义，不是“稳定排序永远更好”。它可能使用额外资源。

---

## 31. `partition()`与排序不同

```cpp
const auto boundary = std::partition(
    records.begin(),
    records.end(),
    is_valid
);
```

结果只保证：

```text
[满足条件的元素][不满足条件的元素]
                  ↑
               boundary
```

每一组内部不保证排序，也不保证原顺序。

`stable_partition()`会保持两组内部的原相对顺序。

分区适合把数据分成两类，而不是完成完整排序。

---

## 32. `remove_if()`与 `erase()`

```cpp
const auto new_end = std::remove_if(
    records.begin(),
    records.end(),
    [](const AudioRecord& record) {
        return record.duration_seconds <= 0.0;
    }
);

records.erase(new_end, records.end());
```

第一步移动保留元素并产生逻辑终点，第二步才真正缩小容器。

执行后，按 `vector`的删除规则重新判断迭代器、引用和指针有效性。

---

## 33. `unique()`只删除相邻重复

```cpp
std::vector<int> values{
    1, 2, 1, 2, 2
};
```

直接调用 `unique()`不能消除所有分散重复，因为它只处理相邻等价元素。

常见全局去重流程：

```cpp
std::sort(values.begin(), values.end());

const auto new_end = std::unique(
    values.begin(),
    values.end()
);

values.erase(new_end, values.end());
```

即：

```text
sort -> unique -> erase
```

排序会改变原顺序。如果需要保留首次出现顺序，应采用其他设计，例如辅助集合配合 `copy_if`。

---

## 34. 结构体按字段去重

先按路径排序：

```cpp
std::sort(
    records.begin(),
    records.end(),
    [](const AudioRecord& left,
       const AudioRecord& right) {
        return left.path < right.path;
    }
);
```

再定义相等条件：

```cpp
const auto new_end = std::unique(
    records.begin(),
    records.end(),
    [](const AudioRecord& left,
       const AudioRecord& right) {
        return left.path == right.path;
    }
);
```

最后 `erase()`。

必须先决定重复记录保留哪一个。如果排序后顺序无法表达保留策略，应先处理优先级再去重。

---

## 35. `accumulate()`完成聚合

```cpp
const double total_duration = std::accumulate(
    records.cbegin(),
    records.cend(),
    0.0,
    [](double total, const AudioRecord& record) {
        return total + record.duration_seconds;
    }
);
```

第四个参数定义如何把累计值和当前元素结合。

初始值 `0.0`保证累计类型为 `double`。

计算平均值前处理空容器：

```cpp
const double average = records.empty()
    ? 0.0
    : total_duration
        / static_cast<double>(records.size());
```

---

## 36. 算法返回迭代器的生命周期

```cpp
auto iterator = std::find_if(
    records.begin(),
    records.end(),
    predicate
);
```

只要后续执行使 `records`迭代器失效的操作，`iterator`就不能继续使用。

例如：

```cpp
records.push_back(new_record);
```

可能重新分配。

安全做法：

- 在修改前使用结果；
- 或保存稳定的业务键，如路径；
- 修改后重新查找；
- 不长期缓存容器迭代器。

---

## 37. 数据处理流水线应分阶段

不建议把所有操作塞入一个巨大 Lambda。

建议：

```text
阶段1：定义校验谓词
阶段2：copy_if筛选
阶段3：transform标准化
阶段4：sort建立去重相邻条件
阶段5：unique + erase
阶段6：stable_sort形成最终输出顺序
阶段7：accumulate统计
```

每个阶段保存清晰的不变量，例如：

```text
筛选后：所有记录字段合法
去重后：路径唯一
最终排序后：按时长降序且相等项保持顺序
```

---

## 38. 算法链中的性能意识

多个算法通常意味着多次遍历：

```text
copy_if  O(n)
transform O(n)
sort      O(n log n)
unique    O(n)
accumulate O(n)
```

这不意味着必须立刻合并成一个循环。

对于学习和大多数工程：

> 先保证语义清楚、测试正确，再根据性能测量决定是否融合阶段。

过早把全部逻辑合在一个循环里会增加错误风险。

---

## 39. 常见错误

### 错误1：返回引用捕获局部变量的 Lambda

局部变量销毁后捕获悬空。

### 错误2：用 `[&]`隐藏全部生命周期依赖

优先明确捕获。

### 错误3：混淆 `const auto iterator`和 `const_iterator`

前者是迭代器变量本身不可改，后者是不能通过它修改元素。

### 错误4：解引用 `end()`

算法查找失败后必须判断。

### 错误5：对 `map`迭代器使用 `+ n`

它不是随机访问迭代器。

### 错误6：推进迭代器越界

`advance/next`不会自动保护你的逻辑范围。

### 错误7：输出容器为空却传 `output.begin()`

先分配空间或使用 `back_inserter()`。

### 错误8：算法回调中改变输入容器大小

可能使算法持有的范围失效。

### 错误9：排序比较器使用 `<=`

破坏严格比较规则。

### 错误10：认为 `partition()`会排序

它只按条件分成两组。

### 错误11：只调用 `remove_if()`或 `unique()`

还需要 `erase()`缩小容器。

### 错误12：未排序就期待 `unique()`全局去重

它只消除相邻重复。

### 错误13：`accumulate()`使用错误初始类型

可能截断浮点结果或产生溢出风险。

### 错误14：算法返回迭代器后修改容器再使用

必须重新判断失效规则。

---

## 40. 练习1：Lambda基础

对应文件：`src/01_lambda_basics.cpp`

定义并调用以下 Lambda：

- 判断整数是否为偶数；
- 计算整数平方；
- 判断音频时长是否为正数；
- 把秒转换为毫秒；
- 使用明确返回类型把毫秒转成 `long long`。

要求：

- 至少一个 Lambda无捕获；
- 至少一个 Lambda有参数；
- 至少一个 Lambda显式写返回类型；
- 使用断言验证边界值0和负数；
- 不使用默认捕获。

---

## 41. 练习2：捕获方式实验

对应文件：`src/02_lambda_capture.cpp`

完成：

1. 按值捕获 `threshold`；
2. 创建 Lambda后修改外部 `threshold`；
3. 验证 Lambda仍使用旧副本；
4. 按引用捕获计数器并修改它；
5. 使用 `[threshold, &count]`混合捕获；
6. 编写一个 `mutable`计数 Lambda；
7. 证明外部计数值没有被 `mutable`副本修改；
8. 在注释中写出引用捕获的生命周期要求。

不能编写并运行故意返回悬空引用捕获的案例；只在练习9中安全分析。

---

## 42. 练习3：迭代器操作

对应文件：`src/03_iterator_operations.cpp`

对 `vector<int>`完成：

- 使用 `begin/end`正向遍历；
- 使用 `cbegin/cend`只读遍历；
- 使用 `crbegin/crend`反向遍历；
- 使用 `next()`获取第三个元素；
- 使用 `prev(end())`获取最后一个元素；
- 使用 `advance()`移动迭代器；
- 使用 `distance()`计算位置；
- 空容器时不调用 `prev(end())`；
- 不让迭代器越界。

增加一个 `map<int,std::string>`，验证其迭代器不能使用 `+ 2`，改用 `next()`。

---

## 43. 练习4：查找与筛选

对应文件：`src/04_find_copy_filter.cpp`

定义：

```cpp
struct AudioRecord {
    std::string path;
    double duration_seconds;
    int sample_rate;
};
```

至少准备8条包含合法和非法字段的数据。

完成：

- `find_if()`寻找第一个非正时长；
- `find_if_not()`寻找第一个不合法记录；
- `count_if()`统计48 kHz记录；
- `copy_if()`复制全部合法记录；
- 输出使用 `back_inserter()`；
- 验证原容器未改变；
- 查找失败时不解引用 `end()`。

合法条件至少包括：

```text
路径非空
时长大于0
采样率大于0
```

---

## 44. 练习5：转换与标准化

对应文件：`src/05_transform_normalize.cpp`

在独立目标容器中使用 `transform()`完成：

- 提取所有路径；
- 把时长从秒转为毫秒；
- 把采样率统一转换为字符串标签；
- 将路径扩展名 `.WAV`标准化为 `.wav`；
- 验证输入容器没有被意外修改。

要求分别使用：

- 预先 `resize()`的输出容器；
- 空容器配合 `back_inserter()`。

报告中比较两种写法。

---

## 45. 练习6：排序与分区

对应文件：`src/06_sort_partition.cpp`

对音频记录副本完成：

1. 按时长升序 `sort()`；
2. 按采样率升序、路径升序进行多关键字排序；
3. 按采样率 `stable_sort()`并验证相同采样率保持原顺序；
4. 使用 `partition()`把时长不低于10秒的记录放前面；
5. 使用返回边界分别遍历两组；
6. 使用 `stable_partition()`对比原顺序；
7. 所有比较器只能使用严格比较。

---

## 46. 练习7：删除与去重

对应文件：`src/07_remove_unique.cpp`

完成两组任务。

### 删除非法数据

- `remove_if()`移动有效记录；
- 记录调用前后 `size()`；
- 调用 `erase()`真正删除；
- 验证删除后全部合法。

### 根据路径去重

- 先明确重复项保留规则；
- 按路径排序；
- `unique()`使用路径相等谓词；
- `erase()`删除逻辑尾部；
- 验证所有路径唯一；
- 说明排序为何改变原顺序。

---

## 47. 练习8：完整算法流水线

对应文件：`src/08_algorithm_pipeline.cpp`

把前面算法组合为：

```text
原始记录
 -> copy_if保留合法记录
 -> transform标准化路径
 -> stable_sort按路径和质量规则排序
 -> unique按路径去重
 -> stable_sort按时长降序输出
 -> accumulate计算总时长
```

要求：

- 每个阶段使用独立、命名清楚的 Lambda；
- 每个阶段后使用断言检查不变量；
- 原始数据保持不变；
- 空输入得到空输出和0总时长；
- 不在算法 Lambda中改变正在遍历的容器结构；
- 报告每阶段输入/输出数量。

---

## 48. 练习9：捕获生命周期安全分析

对应文件：`src/09_capture_lifetime.cpp`

分析以下三类情况，但不要运行未定义行为：

```text
A. 返回按值捕获threshold的Lambda
B. 返回按引用捕获局部threshold的Lambda
C. 保存捕获this的Lambda后销毁对象
```

要求：

- A写成可运行正确案例；
- B只保留注释中的错误代码，不能执行；
- C只做生命周期说明，不制造悬空调用；
- 解释智能指针可能如何管理回调目标，但标为后续内容；
- 写出“捕获方式不等于所有权”的结论；
- 使用 ASan说明：不运行错误路径时零错误不能证明错误代码安全。

---

## 49. 练习10：音频数据集处理器综合项目

对应文件：

- 数据结构：`include/audio_record.hpp`
- 处理器声明：`include/audio_dataset_processor.hpp`
- 处理器实现：`src/audio_dataset_processor.cpp`
- 运行示例：`src/10_audio_dataset_processor.cpp`
- 单元测试：`tests/lambda_algorithm_tests.cpp`
- 实验报告：`target/report/audio_data_pipeline.md`

### 项目定位

这是面向后续 AudioLLM数据准备的内存版元数据处理器，不负责真正解码音频，也不涉及硬件设计。

### `AudioRecord`至少包含

```text
path
duration_seconds
sample_rate
channels
label
quality_score
```

### 处理配置至少包含

```text
minimum_duration
maximum_duration
allowed_sample_rates
minimum_quality_score
```

### 必须实现的能力

```text
validate
filter
normalize_paths
remove_duplicates
sort_for_output
summarize
process
```

### 业务规则

1. 路径不能为空；
2. 时长必须位于配置范围内；
3. 采样率必须在允许集合中；
4. 声道数必须大于0；
5. 质量分数必须达到阈值；
6. 路径统一扩展名大小写；
7. 相同路径只保留质量更高的记录；
8. 质量相同时保留原输入中较早记录；
9. 最终按标签升序、时长降序稳定排序；
10. 原始输入不得被修改；
11. 空输入必须安全处理；
12. 不使用拥有型裸 `new/delete`。

### 必须使用的工具

```text
明确捕获Lambda
const_iterator
find_if或find_if_not
copy_if
transform
stable_sort
unique或等价的算法组合
accumulate
all_of/any_of
```

若为实现“保留最高质量重复项”先进行多关键字稳定排序，必须在报告中写清楚为什么排序规则能保证 `unique()`留下正确记录。

### 统计结果至少包含

```text
原始记录数
有效记录数
被过滤数量
去重数量
总时长
平均时长
最长音频路径
每种标签数量
```

标签数量可以继续使用昨天学过的 `map`或 `unordered_map`。

---

## 50. 单元测试要求

文件：`tests/lambda_algorithm_tests.cpp`

至少覆盖：

```text
1. 无捕获Lambda结果正确
2. 按值捕获不受外部后续修改影响
3. 按引用捕获可修改外部计数
4. mutable修改内部副本而非外部变量
5. cbegin得到只读迭代器
6. next/prev结果正确
7. distance结果正确
8. 空容器不执行非法prev
9. find_if找到首个匹配项
10. find_if失败返回end
11. copy_if只复制合法记录
12. back_inserter写入空目标容器
13. transform转换结果正确
14. 原地transform不改变size
15. sort结果满足严格顺序
16. stable_sort保持等价项原顺序
17. partition边界两侧满足各自条件
18. stable_partition保持组内顺序
19. remove_if单独不缩小size
20. erase-remove真正删除元素
21. unique配合erase完成去重
22. 未排序时不错误声称完成全局去重
23. accumulate使用double初始值
24. 空容器平均值安全
25. Processor拒绝空路径
26. Processor拒绝非法时长
27. Processor拒绝不允许采样率
28. Processor拒绝0声道
29. Processor拒绝低质量记录
30. Processor路径标准化正确
31. Processor重复路径保留高质量项
32. 同质量重复路径保留较早项
33. 最终排序规则正确
34. 原始输入未改变
35. 空输入处理正确
36. 统计数量和总时长正确
37. 最长音频计算正确
38. 标签统计正确
39. ASan/UBSan正式测试零错误
```

可使用 GoogleTest；若测试框架配置影响进度，可先用 `<cassert>`验证核心逻辑。

---

## 51. 编译命令

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
  src/01_lambda_basics.cpp \
  -o target/01_lambda_basics
```

运行：

```bash
./target/01_lambda_basics
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
  src/10_audio_dataset_processor.cpp \
  src/audio_dataset_processor.cpp \
  -Iinclude \
  -o target/10_audio_dataset_processor
```

运行：

```bash
./target/10_audio_dataset_processor
```

---

## 52. Sanitizer验证边界

ASan/UBSan可能发现：

- 悬空引用捕获被实际调用；
- 迭代器失效后访问已释放内存；
- 输出迭代器写入非法空间；
- 越界和部分未定义行为。

但不保证发现：

- 所有迭代器逻辑失效；
- 错误的筛选规则；
- 比较器不满足严格顺序；
- `unique()`前数据未相邻；
- `remove_if()`后忘记 `erase()`；
- 浮点累计被整数初始值截断；
- 捕获了不该共享的外部状态；
- 数据流水线阶段顺序错误。

因此验收必须同时依赖：

```text
单元测试
+ Sanitizer
+ 不变量检查
+ 人工审查捕获和失效边界
```

---

## 53. 实验报告模板

文件：`target/report/audio_data_pipeline.md`

```markdown
# Lambda、迭代器与算法数据处理报告

## 环境
- 编译器：
- C++标准：C++17
- 编译参数：

## Lambda
- 无捕获：
- 按值捕获：
- 按引用捕获：
- mutable：
- 生命周期风险：

## 迭代器
- 半开区间：
- iterator与const_iterator：
- next/prev/advance/distance：
- 迭代器能力差异：
- 本项目失效边界：

## 算法流水线
- 输入数量：
- copy_if后数量：
- transform内容：
- 去重前后数量：
- 最终排序规则：

## remove与unique
- 为什么还需erase：
- 为什么unique只处理相邻项：
- 重复项保留规则：

## 统计结果
- 总时长：
- 平均时长：
- 最长音频：
- 标签数量：

## 不变量
- 筛选后：
- 去重后：
- 排序后：

## Sanitizer结果
- ASan：
- UBSan：

## 今日结论
```

---

## 54. 今日思考题

1. Lambda的完整语法由哪些部分组成？
2. `[]`表示什么？
3. 按值捕获发生在什么时候？
4. 按引用捕获最大的生命周期风险是什么？
5. `[=]`和 `[&]`为什么会隐藏依赖？
6. `mutable`修改的是外部变量还是内部副本？
7. 捕获 `this`是否延长对象生命周期？
8. 什么是泛型 Lambda？
9. `begin/end`表示什么区间？
10. 为什么 `end()`不能解引用？
11. `const auto iterator`和 `const_iterator`有什么区别？
12. `cbegin/cend`有什么用途？
13. `vector`和 `map`迭代器能力有什么差异？
14. 为什么 `map.begin()+2`不能编译？
15. `next()`与 `advance()`的区别是什么？
16. `distance()`在不同迭代器上复杂度相同吗？
17. 为什么空输出 `vector`不能直接传 `begin()`给 `copy_if`？
18. `back_inserter()`大致做什么？
19. 为什么算法回调中不能随意 `push_back()`输入容器？
20. `find_if()`失败返回什么？
21. `copy_if()`适合什么语义？
22. `transform()`原地转换时要注意什么？
23. 排序比较器为什么不能使用 `<=`？
24. `stable_sort()`保证什么？
25. `partition()`和排序有什么区别？
26. `remove_if()`为什么不改变 `size()`？
27. `unique()`为什么只处理相邻重复？
28. 全局去重常见三步是什么？
29. `accumulate()`初始值为何影响结果类型？
30. 为什么清晰的多阶段流水线可能优于一个巨大循环？

---

## 55. 今日验收清单

### 知识

- [ ] 能独立写无捕获和明确捕获 Lambda；
- [ ] 能解释值捕获和引用捕获；
- [ ] 能识别悬空捕获；
- [ ] 能区分迭代器变量为const和const_iterator；
- [ ] 能解释半开区间；
- [ ] 能根据迭代器能力选择操作；
- [ ] 能正确使用输出迭代器；
- [ ] 能组合筛选、转换、排序、删除和统计；
- [ ] 能解释算法不会绕过容器失效规则。

### 编码

- [ ] 完成 `01_lambda_basics.cpp`；
- [ ] 完成 `02_lambda_capture.cpp`；
- [ ] 完成 `03_iterator_operations.cpp`；
- [ ] 完成 `04_find_copy_filter.cpp`；
- [ ] 完成 `05_transform_normalize.cpp`；
- [ ] 完成 `06_sort_partition.cpp`；
- [ ] 完成 `07_remove_unique.cpp`；
- [ ] 完成 `08_algorithm_pipeline.cpp`；
- [ ] 完成 `09_capture_lifetime.cpp`；
- [ ] 完成音频数据集处理器。

### 验证与产出

- [ ] 至少39项测试通过；
- [ ] ASan/UBSan零错误；
- [ ] 原始输入未被意外修改；
- [ ] 每阶段不变量已验证；
- [ ] 完成实验报告；
- [ ] 完成Git提交。

---

## 56. 与就业方向的联系

### AI部署与工程化

今天的技术会直接用于：

- 模型输入元数据清洗；
- 推理请求筛选与批处理；
- 延迟和吞吐统计；
- 日志筛选；
- 配置标准化；
- 评测结果排序与汇总；
- 音频数据集预处理。

真实系统可能处理百万级数据，此时还要继续考虑：

- 内存占用；
- 流式处理；
- 并行算法；
- 数据库或文件分片；
- 错误恢复；
- 可重复性。

今天项目是内存版工程基础，不是假装生产级大数据平台。

### C++工程岗位

高频考查点：

- Lambda捕获与生命周期；
- 迭代器类别；
- 迭代器失效；
- STL算法；
- 比较器严格性；
- erase-remove；
- sort-unique-erase；
- 时间复杂度。

---

## 57. 今日产出

```text
9个独立练习源文件
1个音频数据集处理器综合项目
1个测试文件
1份数据处理实验报告
1次可复现Git提交
```

---

## 58. Git提交建议

```bash
git status
git add day22
git commit -m "Practice lambdas iterators and algorithm pipelines"
```

提交前检查：

- 不提交二进制文件；
- 提交实验报告；
- 不运行故意悬空捕获代码；
- 编译命令可复现；
- 文件名与文档一致；
- 测试不依赖未定义行为。

---

## 59. 最终速记

```text
Lambda：[捕获](参数){函数体}

值捕获：保存副本
引用捕获：依赖外部对象生命周期
捕获不等于所有权

算法范围：[first,last)，last不可解引用

const_iterator：不能通过它修改元素
const auto iterator：迭代器变量本身不可重新赋值

back_inserter：向空输出容器push_back

remove_if + erase：真正删除
sort + unique + erase：相邻化后全局去重

比较器必须严格，不能用<=代替<

accumulate初始值决定累计类型

算法执行中仍必须遵守容器迭代器失效规则
```
