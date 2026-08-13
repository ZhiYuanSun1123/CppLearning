# 第5周·工作日3：掌握 `queue`、`stack`、`priority_queue`和常用算法

## 0. 今天要完成什么

今天的目标不是背完 `<algorithm>`，而是建立两类能力：

1. 根据数据的取出顺序选择 `queue`、`stack`或 `priority_queue`；
2. 使用标准算法完成查找、统计、判断、排序、变换和聚合，避免重复手写循环。

今天结束时，你应当能够解释：

```text
queue          -> 先进先出 FIFO
stack          -> 后进先出 LIFO
priority_queue -> 总是先取当前最高优先级元素
```

并完成一个与后续 AI 部署相关的“推理任务调度器”项目。

---

## 1. 前置知识与超纲说明

### 今天必须掌握

- `queue`的 `push/emplace/front/back/pop/empty/size`；
- `stack`的 `push/emplace/top/pop/empty/size`；
- `priority_queue`的 `push/emplace/top/pop/empty/size`；
- `pop()`不返回被删除元素；
- 三者为什么不能直接范围 `for`遍历；
- 默认 `priority_queue`是大顶堆；
- 使用 `std::greater`构造小顶堆；
- 自定义任务优先级的基本方法；
- 算法的半开区间 `[first, last)`；
- `find/find_if/count/count_if`；
- `all_of/any_of/none_of`；
- `sort/stable_sort`；
- `min_element/max_element`；
- `transform/remove_if/erase`；
- `accumulate`；
- 使用算法返回结果前先判断是否为 `end()`。

### 今天首次系统学习

- 容器适配器；
- 底层容器；
- 堆和优先级队列；
- 比较器；
- 算法与容器分离；
- 谓词；
- Lambda捕获；
- erase-remove惯用法；
- 排序稳定性。

### 超纲提示

以下内容会为理解示例而出现，但暂时不要求你掌握底层实现：

- 函数模板和算法模板的实现；
- `decltype`和复杂类型推导；
- 自定义分配器；
- 堆的数组下标证明；
- 严格弱序的形式化定义；
- C++20 ranges；
- 并行算法执行策略；
- 无锁队列和并发优先队列。

### Lambda说明

今天需要会使用简单 Lambda：

```cpp
[](int value) {
    return value % 2 == 0;
}
```

以及简单捕获：

```cpp
[threshold](int value) {
    return value >= threshold;
}
```

Lambda本质和闭包类型属于后续模板/函数对象内容，今天只要求会读、会写、理解捕获值的用途。

---

## 2. 今日目录结构

```text
day21/
├── include/
│   ├── inference_task.hpp
│   └── inference_scheduler.hpp
├── src/
│   ├── 01_queue_basics.cpp
│   ├── 02_stack_basics.cpp
│   ├── 03_priority_queue_basics.cpp
│   ├── 04_min_heap.cpp
│   ├── 05_custom_task_priority.cpp
│   ├── 06_find_count_predicates.cpp
│   ├── 07_sort_and_stability.cpp
│   ├── 08_transform_and_remove.cpp
│   ├── 09_audio_statistics.cpp
│   ├── 10_inference_scheduler.cpp
│   └── inference_scheduler.cpp
├── tests/
│   └── adaptor_algorithm_tests.cpp
├── target/
│   └── report/
│       └── adaptors_and_algorithms.md
└── 第5周_工作日3_queue_stack_priority_queue与常用算法.md
```

所有练习都在对应题目下标明文件名。源码由你完成，本文不直接提供综合项目最终答案。

---

## 3. 建议学习顺序

```text
45分钟：queue和stack
60分钟：priority_queue与比较器
75分钟：查找、判断、统计算法
60分钟：排序、变换、删除与聚合算法
120分钟：独立练习
90分钟：推理任务调度器
30分钟：测试、报告和复盘
```

如果时间不足，优先完成：

```text
三个适配器基本接口
> priority_queue比较规则
> find_if/sort/remove_if
> 综合项目
> 性能扩展
```

---

## 4. 什么是容器适配器

`queue`、`stack`和 `priority_queue`被称为容器适配器。

它们不是直接向你暴露底层容器的全部接口，而是限制访问方式，形成特定的数据结构语义。

```text
底层容器
   ↓
适配器限制接口
   ↓
queue / stack / priority_queue
```

例如 `stack`只允许访问栈顶，不能访问中间元素。这种限制不是缺点，而是在表达：

> 使用者只应按后进先出的规则操作数据。

---

## 5. `queue`：先进先出

头文件：

```cpp
#include <queue>
```

创建：

```cpp
std::queue<std::string> requests;
```

操作过程：

```text
push A  -> [A]
push B  -> [A B]
push C  -> [A B C]
pop     -> [B C]
```

最早进入的 `A`最先离开，这叫 FIFO：

```text
First In, First Out
```

典型用途：

- 普通请求排队；
- 广度优先搜索；
- 消息缓冲；
- 生产者—消费者任务队列；
- 按到达顺序处理音频片段。

---

## 6. `queue`常用接口

```cpp
std::queue<std::string> requests;

requests.push("request-A");
requests.emplace("request-B");

std::cout << requests.front() << '\n';
std::cout << requests.back() << '\n';
std::cout << requests.size() << '\n';

requests.pop();
```

含义：

| 接口 | 作用 |
|---|---|
| `push(value)` | 把已有值压入队尾 |
| `emplace(args...)` | 在队尾直接构造元素 |
| `front()` | 访问最早进入的元素 |
| `back()` | 访问最后进入的元素 |
| `pop()` | 删除队首元素 |
| `empty()` | 判断是否为空 |
| `size()` | 返回元素数量 |

---

## 7. 空队列不能调用 `front/back/pop`

下面属于错误：

```cpp
std::queue<int> values;
std::cout << values.front();
```

空容器调用 `front()`、`back()`或 `pop()`属于未定义行为。

正确做法：

```cpp
if (!values.empty()) {
    const int current = values.front();
    values.pop();
}
```

`pop()`本身不返回元素，因此要先复制或移动，再删除。

---

## 8. 为什么 `pop()`不返回值

不能写：

```cpp
int value = values.pop(); // 错误，pop返回void
```

应写：

```cpp
int value = values.front();
values.pop();
```

如果元素复制成本高且允许移动：

```cpp
Task task = std::move(tasks.front());
tasks.pop();
```

这里使用移动后，应当立即删除队首，且不要再依赖已移动对象的具体内容。

接口分离可以避免“返回值复制或移动过程抛异常时，元素是否已被删除”这样的语义困难。今天理解结论即可。

---

## 9. `queue`默认底层容器

概念上，默认类型近似为：

```cpp
std::queue<T, std::deque<T>>
```

`std::deque`会在后续系统学习。今天只需知道它支持队首删除和队尾插入。

也可以显式指定满足要求的底层容器，例如：

```cpp
std::queue<int, std::list<int>> values;
```

但初学阶段不要为了“优化”随意替换默认容器。

---

## 10. `stack`：后进先出

头文件同样是：

```cpp
#include <stack>
```

创建：

```cpp
std::stack<std::string> history;
```

操作过程：

```text
push A -> [A]
push B -> [A B]
push C -> [A B C]
pop    -> [A B]
```

最后进入的 `C`最先离开，这叫 LIFO：

```text
Last In, First Out
```

典型用途：

- 撤销操作；
- 表达式括号匹配；
- 深度优先搜索；
- 函数调用思想的模拟；
- 配置变更回滚记录。

---

## 11. `stack`常用接口

```cpp
std::stack<int> values;

values.push(10);
values.emplace(20);

std::cout << values.top() << '\n';

values.pop();
```

| 接口 | 作用 |
|---|---|
| `push(value)` | 压入栈顶 |
| `emplace(args...)` | 在栈顶构造元素 |
| `top()` | 访问栈顶元素 |
| `pop()` | 删除栈顶元素 |
| `empty()` | 判断是否为空 |
| `size()` | 返回元素数量 |

空 `stack`不能调用 `top()`或 `pop()`。

---

## 12. `stack`默认底层容器

默认近似为：

```cpp
std::stack<T, std::deque<T>>
```

也可以使用：

```cpp
std::stack<int, std::vector<int>> values;
```

因为栈只需要在同一端添加和删除。

但底层容器属于适配器类型的一部分：不同底层容器的 `stack`不是相同类型。

---

## 13. 为什么不能直接遍历 `queue`和 `stack`

它们没有公开：

```cpp
begin()
end()
```

因此不能写：

```cpp
for (const auto& value : tasks) {
}
```

这是因为适配器故意限制访问方式。

如需观察全部内容，可以复制一个临时适配器：

```cpp
auto copy = tasks;

while (!copy.empty()) {
    std::cout << copy.front() << '\n';
    copy.pop();
}
```

`stack`则使用 `top()`。

如果业务经常需要随机访问或遍历全部元素，说明适配器可能不是最适合的主存储结构。

---

## 14. `priority_queue`：按优先级取元素

创建：

```cpp
std::priority_queue<int> priorities;
```

加入：

```cpp
priorities.push(3);
priorities.push(10);
priorities.push(5);
```

`top()`得到：

```text
10
```

默认是大顶堆，即最大元素具有最高优先级。

注意它不是“整体排好序后提供任意位置访问”，只保证：

> `top()`始终是按比较规则位于最高优先级的元素。

---

## 15. 堆的直观理解

优先队列通常借助堆维护最高优先级元素。

大顶堆可以抽象为：

```text
        10
       /  \
      5    3
```

它只要求父节点不低于子节点，不要求所有元素从左到右完全排序。

因此：

- 查看最高优先级：`O(1)`；
- 插入：`O(log n)`；
- 删除最高优先级：`O(log n)`。

---

## 16. `priority_queue`常用接口

```cpp
std::priority_queue<int> priorities;

priorities.push(3);
priorities.emplace(10);

std::cout << priorities.top() << '\n';
priorities.pop();
```

| 接口 | 作用 |
|---|---|
| `push(value)` | 加入元素并维护堆 |
| `emplace(args...)` | 构造元素并维护堆 |
| `top()` | 查看最高优先级元素 |
| `pop()` | 删除最高优先级元素 |
| `empty()` | 判断是否为空 |
| `size()` | 返回数量 |

空优先队列不能调用 `top()`或 `pop()`。

---

## 17. 小顶堆

需要最小值优先时：

```cpp
#include <functional>
#include <queue>
#include <vector>

std::priority_queue<
    int,
    std::vector<int>,
    std::greater<int>
> min_heap;
```

加入：

```cpp
min_heap.push(3);
min_heap.push(10);
min_heap.push(5);
```

此时：

```cpp
min_heap.top(); // 3
```

三个模板参数分别表示：

```text
元素类型
底层容器类型
比较器类型
```

模板语法会在后续专门学习，今天先会使用标准写法。

---

## 18. 比较器最容易理解反的地方

`priority_queue`的比较器不是简单表达“谁先出来”。可以先记住标准组合：

```text
std::less<T>    -> 默认，大值在top
std::greater<T> -> 小值在top
```

自定义比较器中：

```cpp
bool operator()(const Task& left,
                const Task& right) const {
    return left.priority < right.priority;
}
```

这表示 `left`的优先级低于 `right`，所以数值更大的任务会位于 `top()`。

不要仅凭函数返回值名称猜结果，应使用3个优先级不同的任务编写测试验证。

---

## 19. 自定义任务比较器

定义任务：

```cpp
struct InferenceTask {
    int priority;
    std::size_t sequence;
    std::string request_id;
};
```

希望：

1. `priority`越大越先执行；
2. 相同优先级时，`sequence`越小越先执行。

可以声明比较器：

```cpp
struct TaskCompare {
    bool operator()(
        const InferenceTask& left,
        const InferenceTask& right
    ) const;
};
```

这里的 `operator()`叫函数调用运算符重载。

### 超纲提醒

运算符重载后续会系统讲。今天只需理解：

```text
TaskCompare的对象可以像函数一样被priority_queue调用
```

综合练习要求你根据规则补充实现，并用测试确认弹出顺序。

---

## 20. 优先队列不是稳定队列

如果两个元素按比较器判断具有相同优先级，标准不保证它们按插入顺序弹出。

如果业务要求相同优先级先进先出，需要把到达序号加入比较规则：

```text
第一关键字：priority降序
第二关键字：sequence升序
```

这也是综合项目加入 `sequence`的原因。

---

## 21. 三种适配器复杂度

| 操作 | `queue` | `stack` | `priority_queue` |
|---|---:|---:|---:|
| 查看待取元素 | 通常 `O(1)` | 通常 `O(1)` | `O(1)` |
| 添加 | 通常 `O(1)` | 通常 `O(1)` | `O(log n)` |
| 删除待取元素 | 通常 `O(1)` | 通常 `O(1)` | `O(log n)` |
| 任意位置查找 | 不提供 | 不提供 | 不提供 |

这里 `queue/stack`的复杂度还与选用的底层容器接口保证有关。默认底层容器满足所需效率。

---

## 22. 适配器与迭代器失效

适配器不公开迭代器，因此通常不直接讨论其迭代器失效。

但引用仍可能出现生命周期问题：

```cpp
const Task& current = tasks.front();
tasks.pop();
// current已经指向被删除元素，不能再使用
```

安全写法：

```cpp
Task current = tasks.front();
tasks.pop();
```

对于 `priority_queue`也一样：

```cpp
Task current = tasks.top();
tasks.pop();
```

不要让指向 `front/top`元素的引用跨过删除该元素的 `pop()`。

---

## 23. 算法与容器为什么分开

标准算法通常通过迭代器区间工作：

```cpp
std::find(values.begin(), values.end(), target);
```

算法不一定需要知道容器具体是 `vector`、`array`还是其他满足要求的序列。

这种设计可以理解为：

```text
容器负责保存数据
迭代器描述范围和位置
算法负责处理范围
```

`queue/stack/priority_queue`不公开迭代器，所以不能直接传给这些算法。

---

## 24. 半开区间 `[first, last)`

标准算法通常接收：

```text
[first, last)
```

含义：

- 包含 `first`；
- 不包含 `last`；
- `last`通常可以是 `end()`。

```cpp
std::sort(values.begin(), values.end());
```

如果只处理下标 `[1, 4)`：

```cpp
std::sort(values.begin() + 1,
          values.begin() + 4);
```

将处理下标1、2、3，不处理4。

---

## 25. 常用算法头文件

大部分算法：

```cpp
#include <algorithm>
```

数值聚合：

```cpp
#include <numeric>
```

比较器：

```cpp
#include <functional>
```

---

## 26. `find()`

```cpp
const auto iterator = std::find(
    values.begin(),
    values.end(),
    42
);
```

找到时返回对应迭代器，找不到时返回：

```cpp
values.end()
```

必须先判断：

```cpp
if (iterator != values.end()) {
    std::cout << *iterator << '\n';
}
```

在顺序容器中，`find()`通常需要线性扫描：

```text
O(n)
```

对于 `map/unordered_map`按键查找，应优先使用容器自己的 `find()`，因为它利用树或哈希结构。

---

## 27. `find_if()`

根据条件寻找第一个匹配元素：

```cpp
const auto iterator = std::find_if(
    values.begin(),
    values.end(),
    [](int value) {
        return value > 100;
    }
);
```

传入的条件被称为谓词。返回 `true`表示当前元素符合要求。

---

## 28. `count()`与 `count_if()`

统计等于指定值的元素数量：

```cpp
const auto count = std::count(
    values.begin(),
    values.end(),
    0
);
```

按条件统计：

```cpp
const auto positive_count = std::count_if(
    values.begin(),
    values.end(),
    [](int value) {
        return value > 0;
    }
);
```

返回类型通常不是 `int`，使用 `auto`或容器的差值类型更稳妥。

---

## 29. `all_of()`、`any_of()`、`none_of()`

```cpp
const bool all_valid = std::all_of(
    rates.begin(),
    rates.end(),
    [](int rate) {
        return rate > 0;
    }
);
```

```cpp
const bool has_48k = std::any_of(
    rates.begin(),
    rates.end(),
    [](int rate) {
        return rate == 48000;
    }
);
```

```cpp
const bool no_invalid = std::none_of(
    rates.begin(),
    rates.end(),
    [](int rate) {
        return rate <= 0;
    }
);
```

空范围的结果需要理解：

```text
all_of(empty)  -> true
any_of(empty)  -> false
none_of(empty) -> true
```

这来自逻辑定义，不是错误。

---

## 30. `min_element()`与 `max_element()`

```cpp
const auto minimum = std::min_element(
    latencies.begin(),
    latencies.end()
);
```

```cpp
const auto maximum = std::max_element(
    latencies.begin(),
    latencies.end()
);
```

空范围会返回 `end()`，所以解引用前必须判断。

它们返回迭代器，不直接返回值，因为迭代器还能表示位置，并避免不必要复制。

---

## 31. `sort()`

升序：

```cpp
std::sort(values.begin(), values.end());
```

降序：

```cpp
std::sort(
    values.begin(),
    values.end(),
    std::greater<int>{}
);
```

自定义规则：

```cpp
std::sort(
    tasks.begin(),
    tasks.end(),
    [](const Task& left, const Task& right) {
        return left.priority > right.priority;
    }
);
```

`sort()`平均复杂度要求通常写为：

```text
O(n log n)
```

并且要求随机访问迭代器，因此可以用于 `vector/array`，不能直接用于 `list`或关联容器。

---

## 32. 比较器必须满足一致规则

比较器应回答：

```text
left是否严格排在right之前
```

不能写：

```cpp
return left.priority >= right.priority;
```

相等时它仍返回 `true`，不满足严格性要求，可能导致未定义行为。

正确形式之一：

```cpp
return left.priority > right.priority;
```

相同优先级需要第二关键字时，明确分支：

```cpp
if (left.priority != right.priority) {
    return left.priority > right.priority;
}

return left.sequence < right.sequence;
```

---

## 33. `stable_sort()`

普通 `sort()`不保证比较结果相等的元素保持原相对顺序。

`stable_sort()`保证相等元素保留原相对顺序：

```cpp
std::stable_sort(
    tasks.begin(),
    tasks.end(),
    [](const Task& left, const Task& right) {
        return left.priority > right.priority;
    }
);
```

例如输入：

```text
A(priority=2)
B(priority=1)
C(priority=2)
```

稳定排序后同优先级的 A仍在 C之前。

是否需要稳定排序由业务语义决定，不要机械使用。

---

## 34. `is_sorted()`

验证范围是否已排序：

```cpp
const bool sorted = std::is_sorted(
    values.begin(),
    values.end()
);
```

它适合单元测试和输入前置条件检查，但不要把它当作排序操作。

---

## 35. `reverse()`

```cpp
std::reverse(values.begin(), values.end());
```

它反转现有顺序，不等于“降序排序”。

例如：

```text
3 1 2
```

反转后：

```text
2 1 3
```

不是降序。

---

## 36. `transform()`

对每个输入元素应用转换：

```cpp
std::vector<int> milliseconds(
    seconds.size()
);

std::transform(
    seconds.begin(),
    seconds.end(),
    milliseconds.begin(),
    [](int value) {
        return value * 1000;
    }
);
```

输出容器必须先有足够空间。

另一种方法是使用插入迭代器：

```cpp
#include <iterator>

std::transform(
    seconds.begin(),
    seconds.end(),
    std::back_inserter(milliseconds),
    /* 转换函数 */
);
```

`back_inserter`属于今天的扩展内容，先理解它会调用目标容器的 `push_back()`。

---

## 37. `remove_if()`为什么不真正删除容器元素

```cpp
const auto new_end = std::remove_if(
    values.begin(),
    values.end(),
    [](int value) {
        return value < 0;
    }
);
```

算法无法直接改变 `vector`的 `size()`。它会把应保留元素移动到前面，并返回新的逻辑终点。

概念上：

```text
原始：  1 -2 3 -4 5
移动后：1  3 5 ?  ?
              ↑
            new_end
```

问号区域仍属于容器实际元素范围，值处于有效但不应依赖的状态。

---

## 38. erase-remove惯用法

真正删除：

```cpp
values.erase(
    std::remove_if(
        values.begin(),
        values.end(),
        [](int value) {
            return value < 0;
        }
    ),
    values.end()
);
```

分步写更适合初学：

```cpp
const auto new_end = std::remove_if(
    values.begin(),
    values.end(),
    predicate
);

values.erase(new_end, values.end());
```

注意：`remove_if()`之后，原有迭代器的逻辑含义可能变化；`erase()`之后，被删除范围及之后的定位按 `vector`规则失效。

---

## 39. `accumulate()`

头文件：

```cpp
#include <numeric>
```

求和：

```cpp
const long long total = std::accumulate(
    values.begin(),
    values.end(),
    0LL
);
```

第三个参数不仅是初始值，也会影响累加结果类型。

错误风险：

```cpp
double sum = std::accumulate(
    durations.begin(),
    durations.end(),
    0
);
```

初始值是 `int`，累加过程可能按整数类型进行，丢失小数。

应写：

```cpp
double sum = std::accumulate(
    durations.begin(),
    durations.end(),
    0.0
);
```

---

## 40. Lambda捕获值与捕获引用

按值捕获：

```cpp
const int threshold = 100;

auto predicate = [threshold](int value) {
    return value >= threshold;
};
```

Lambda内部保存 `threshold`的副本。

按引用捕获：

```cpp
auto predicate = [&threshold](int value) {
    return value >= threshold;
};
```

Lambda依赖外部对象的生命周期。

初学阶段建议：

- 只使用需要的明确捕获；
- 只读小值优先按值捕获；
- 不让引用捕获的 Lambda活得比被引用对象更久；
- 避免无脑使用 `[&]`或 `[=]`隐藏依赖。

---

## 41. 算法复杂度速查

| 算法 | 典型复杂度 |
|---|---:|
| `find/find_if` | `O(n)` |
| `count/count_if` | `O(n)` |
| `all_of/any_of/none_of` | 最多 `O(n)`，可提前结束 |
| `min_element/max_element` | `O(n)` |
| `sort/stable_sort` | 通常关注 `O(n log n)` |
| `reverse` | `O(n)` |
| `transform` | `O(n)` |
| `remove_if` | `O(n)` |
| `accumulate` | `O(n)` |

算法复杂度之外，还要考虑谓词或比较器本身的成本。

---

## 42. 常见错误

### 错误1：从空适配器读取

调用 `front/back/top/pop`前检查 `empty()`。

### 错误2：认为 `pop()`返回被删元素

它返回 `void`，要先取值再删除。

### 错误3：长期保存 `front/top`引用后再 `pop()`

被删元素生命周期结束，引用失效。

### 错误4：认为 `priority_queue`内部全部有序

它只保证 `top()`最高优先级。

### 错误5：认为同优先级任务自动FIFO

默认不保证，应加入序号比较。

### 错误6：比较器使用 `>=`或 `<=`

可能破坏严格比较要求。

### 错误7：解引用算法返回的 `end()`

必须先判断。

### 错误8：对 `map`使用 `std::find`按键查找

应使用容器自己的 `find()`。

### 错误9：认为 `remove_if()`会改变 `vector::size()`

还必须调用 `erase()`。

### 错误10：`transform()`输出空间不足

先 `resize()`或使用 `back_inserter()`。

### 错误11：`accumulate()`初始值类型错误

整数0可能造成浮点数据精度丢失。

### 错误12：一边遍历算法区间一边修改导致失效

算法执行期间不要随意改变输入容器结构。

---

## 43. 练习1：请求FIFO队列

对应文件：`src/01_queue_basics.cpp`

使用：

```cpp
std::queue<std::string>
```

依次加入：

```text
req-001
req-002
req-003
```

要求：

- 打印 `front/back/size`；
- 按FIFO顺序取出全部任务；
- 每次先复制 `front()`再 `pop()`；
- 空队列时停止，不能继续访问；
- 验证输出顺序；
- 解释 `pop()`为什么不能直接赋值给变量。

---

## 44. 练习2：撤销操作栈

对应文件：`src/02_stack_basics.cpp`

使用 `std::stack<std::string>`保存：

```text
load model
set batch=4
enable fp16
```

要求：

- 显示当前 `top()`；
- 按相反顺序撤销；
- 每次撤销先复制再 `pop()`；
- 打印剩余数量；
- 处理空栈；
- 解释为什么撤销适合LIFO。

---

## 45. 练习3：默认大顶堆

对应文件：`src/03_priority_queue_basics.cpp`

向 `priority_queue<int>`加入：

```text
3 10 5 1 8
```

要求：

- 每次输出 `top()`并 `pop()`；
- 验证输出为降序；
- 记录每次 `size()`；
- 不尝试范围 `for`；
- 说明为什么它不是普通FIFO队列。

---

## 46. 练习4：小顶堆

对应文件：`src/04_min_heap.cpp`

使用：

```cpp
std::priority_queue<
    int,
    std::vector<int>,
    std::greater<int>
>
```

加入与练习3相同的数据，验证按升序弹出。

要求解释三个模板参数，以及为什么 `std::greater<int>`使最小值位于 `top()`。

---

## 47. 练习5：自定义推理任务优先级

对应文件：`src/05_custom_task_priority.cpp`

定义：

```cpp
struct InferenceTask {
    std::string request_id;
    int priority;
    std::size_t sequence;
};
```

规则：

```text
priority越大越先执行
priority相同时sequence越小越先执行
```

加入：

```text
A priority=2 sequence=0
B priority=5 sequence=1
C priority=5 sequence=2
D priority=1 sequence=3
```

预期顺序：

```text
B C A D
```

要求：

- 实现比较器；
- 相等时不能返回 `true`；
- 编写断言验证完整顺序；
- 解释序号为什么能实现同优先级FIFO。

---

## 48. 练习6：查找、统计和判断

对应文件：`src/06_find_count_predicates.cpp`

数据：

```cpp
std::vector<int> latencies{
    25, 40, 120, 55, 120, 10
};
```

完成：

- `find()`查找120；
- 查找不存在的999并安全处理；
- `find_if()`寻找第一个大于100的值；
- `count()`统计120；
- `count_if()`统计不超过50的值；
- `all_of()`验证全部非负；
- `any_of()`判断是否存在超过100；
- `none_of()`判断是否存在负数；
- 用断言验证结果。

---

## 49. 练习7：排序与稳定性

对应文件：`src/07_sort_and_stability.cpp`

定义任务：

```text
A priority=2
B priority=1
C priority=2
D priority=1
```

要求：

- 用 `sort()`按优先级降序；
- 用 `stable_sort()`按优先级降序；
- 比较相同优先级元素的原相对顺序；
- 使用 `is_sorted()`验证；
- 比较器不能使用 `>=`；
- 解释稳定排序适合什么业务语义。

不要依赖某次 `sort()`恰好保持顺序来证明其稳定。

---

## 50. 练习8：转换与安全删除

对应文件：`src/08_transform_and_remove.cpp`

输入秒数：

```cpp
std::vector<double> seconds{
    0.5, -1.0, 2.25, 0.0, 3.5
};
```

要求：

1. 使用 `remove_if()`和 `erase()`删除负数；
2. 使用 `transform()`转换为毫秒；
3. 保证输出容器空间充足；
4. 打印删除前后 `size()`；
5. 使用 `all_of()`验证结果非负；
6. 说明为什么只调用 `remove_if()`不够。

---

## 51. 练习9：音频统计

对应文件：`src/09_audio_statistics.cpp`

数据结构：

```cpp
struct AudioRecord {
    std::string path;
    double duration_seconds;
    int sample_rate;
};
```

至少准备5条数据，完成：

- `find_if()`按路径查找；
- `count_if()`统计48 kHz文件；
- `min_element/max_element()`寻找最短和最长音频；
- `accumulate()`计算总时长，初始值使用 `0.0`；
- 计算平均时长，处理空容器；
- `stable_sort()`按时长降序；
- 时长相同保持原顺序；
- 用测试验证结果。

---

## 52. 练习10：推理任务调度器综合项目

对应文件：

- 任务和比较器声明：`include/inference_task.hpp`
- 调度器声明：`include/inference_scheduler.hpp`
- 调度器实现：`src/inference_scheduler.cpp`
- 运行示例：`src/10_inference_scheduler.cpp`
- 单元测试：`tests/adaptor_algorithm_tests.cpp`
- 实验报告：`target/report/adaptors_and_algorithms.md`

### 项目目标

实现一个简化推理任务调度器：

```text
请求提交
   ↓
priority_queue按优先级排队
   ↓
取出最高优先级任务
   ↓
记录执行历史
```

### `InferenceTask`至少包含

```text
request_id
model_name
priority
sequence
estimated_memory_mb
```

### `InferenceScheduler`至少提供

```text
submit
empty
pending_count
peek_next
take_next
drain_all
```

### 调度规则

1. 优先级数值越大越先执行；
2. 同优先级按提交序号FIFO；
3. 空 `request_id`或 `model_name`拒绝提交；
4. 预估显存为0的任务拒绝提交；
5. `peek_next`不删除任务；
6. `take_next`取出并删除任务；
7. 空调度器调用取任务接口必须有明确错误语义；
8. 不返回指向即将 `pop()`元素的悬空引用；
9. 不使用拥有型裸 `new/delete`。

### 算法要求

`drain_all()`返回 `vector<InferenceTask>`后，至少使用标准算法完成：

- 验证所有任务显存大于0；
- 统计指定模型任务数；
- 计算预估显存总和；
- 找出显存最大的任务；
- 按模型名稳定排序一个副本用于报告。

不要为了使用算法破坏实际调度顺序。

---

## 53. 单元测试要求

文件：`tests/adaptor_algorithm_tests.cpp`

至少覆盖：

```text
1. queue按FIFO取出
2. queue的front和back正确
3. stack按LIFO取出
4. priority_queue默认大值优先
5. 小顶堆小值优先
6. 自定义任务高优先级先出
7. 相同优先级按sequence先出
8. find找到目标
9. find找不到返回end
10. find_if返回第一个匹配项
11. count和count_if结果正确
12. all_of/any_of/none_of正确
13. min_element/max_element正确
14. sort结果有序
15. stable_sort保持等价元素顺序
16. transform输出正确
17. erase-remove真正缩小size
18. accumulate浮点结果正确
19. 调度器拒绝空请求ID
20. 调度器拒绝空模型名
21. 调度器拒绝0显存任务
22. submit后pending_count正确
23. peek_next不删除任务
24. take_next删除并返回正确任务
25. 空调度器取任务错误语义正确
26. drain_all顺序正确
27. drain_all后调度器为空
28. 正式测试在ASan/UBSan下零错误
```

可以继续使用你已经学习的 GoogleTest；如果配置测试框架影响当天进度，也可以先用 `<cassert>`完成逻辑验证。

---

## 54. 编译命令

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
  src/01_queue_basics.cpp \
  -o target/01_queue_basics
```

运行：

```bash
./target/01_queue_basics
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
  src/10_inference_scheduler.cpp \
  src/inference_scheduler.cpp \
  -Iinclude \
  -o target/10_inference_scheduler
```

运行：

```bash
./target/10_inference_scheduler
```

---

## 55. ASan与UBSan验证重点

Sanitizer可能发现：

- `pop()`后继续使用旧引用；
- 越界访问算法结果；
- 解引用某些失效迭代器；
- 比较器或计算中的部分未定义行为。

Sanitizer不能保证发现：

- 从空适配器调用 `front/top/pop`的所有表现；
- 错误调度顺序；
- 比较器不满足严格规则；
- 误以为 `remove_if()`已经删除元素；
- `accumulate()`初始值类型造成的精度丢失；
- 选择错误数据结构导致的性能问题。

所以正式验收包含：

```text
Sanitizer零错误
+
单元测试通过
+
人工检查数据结构和比较规则
```

---

## 56. 实验报告模板

文件：`target/report/adaptors_and_algorithms.md`

```markdown
# 容器适配器与算法实验报告

## 环境
- 编译器：
- C++标准：C++17
- 编译参数：

## queue
- 数据取出顺序：
- 适用场景：
- 空容器保护：

## stack
- 数据取出顺序：
- 适用场景：

## priority_queue
- 默认top：
- 小顶堆写法：
- 自定义比较规则：
- 同优先级FIFO实现：

## 算法
- find/find_if：
- count/count_if：
- 判断算法：
- sort/stable_sort：
- transform：
- erase-remove：
- accumulate：

## 复杂度
- queue添加/删除：
- stack添加/删除：
- priority_queue添加/删除：
- sort：
- 线性算法：

## 推理任务调度器
- 调度规则：
- 空队列错误语义：
- 为什么take_next返回值而不是引用：
- 标准算法使用位置：

## Sanitizer结果
- ASan：
- UBSan：

## 今日结论
```

---

## 57. 今日思考题

1. 什么是容器适配器？
2. `queue`和 `stack`的顺序有什么区别？
3. 为什么 `pop()`不返回元素？
4. 从空适配器读取为什么危险？
5. 为什么三种适配器不能直接范围 `for`？
6. `priority_queue<int>`默认谁在 `top()`？
7. 如何构造整数小顶堆？
8. 堆是否意味着内部元素完全有序？
9. 优先队列插入和删除的复杂度是什么？
10. 相同优先级为什么不自动FIFO？
11. 如何通过序号实现FIFO？
12. 比较器为什么不能使用 `>=`？
13. 标准算法为什么使用迭代器范围？
14. `[first,last)`包含哪些元素？
15. `find()`失败返回什么？
16. 为什么对 `map`按键查找不优先使用 `std::find()`？
17. `find_if()`的谓词返回 `true`表示什么？
18. 空范围上的 `all_of/any_of/none_of`分别是什么？
19. `min_element()`为什么返回迭代器？
20. `sort()`和 `stable_sort()`有什么区别？
21. `reverse()`为什么不等于降序排序？
22. `transform()`对输出空间有什么要求？
23. `remove_if()`为什么不改变容器 `size()`？
24. erase-remove惯用法分哪两步？
25. `accumulate()`的初始值为什么影响结果类型？
26. Lambda按值捕获和按引用捕获有什么生命周期差别？
27. 为什么不能让 `top()`引用跨过 `pop()`？
28. 什么情况下应该直接使用 `vector`而不是适配器？
29. 推理请求调度为什么适合 `priority_queue`？
30. 普通公平请求处理为什么更适合 `queue`？

---

## 58. 今日验收清单

### 知识

- [ ] 能解释FIFO和LIFO；
- [ ] 能解释容器适配器；
- [ ] 能正确使用三个适配器；
- [ ] 能构造大顶堆和小顶堆；
- [ ] 能写双关键字任务比较器；
- [ ] 能解释半开区间；
- [ ] 能选择常用算法；
- [ ] 能解释稳定排序；
- [ ] 能解释erase-remove；
- [ ] 能避免 `accumulate()`类型陷阱。

### 编码

- [ ] 完成 `01_queue_basics.cpp`；
- [ ] 完成 `02_stack_basics.cpp`；
- [ ] 完成 `03_priority_queue_basics.cpp`；
- [ ] 完成 `04_min_heap.cpp`；
- [ ] 完成 `05_custom_task_priority.cpp`；
- [ ] 完成 `06_find_count_predicates.cpp`；
- [ ] 完成 `07_sort_and_stability.cpp`；
- [ ] 完成 `08_transform_and_remove.cpp`；
- [ ] 完成 `09_audio_statistics.cpp`；
- [ ] 完成推理任务调度器。

### 验证与产出

- [ ] 至少28项测试通过；
- [ ] ASan/UBSan零错误；
- [ ] 完成实验报告；
- [ ] 解释综合项目的比较规则；
- [ ] 完成Git提交。

---

## 59. 与后续就业技术栈的联系

### AI推理部署

这些数据结构常用于：

- 请求排队；
- 高优先级请求调度；
- 批处理候选任务；
- 超时任务选择；
- 推理结果统计；
- 延迟数据排序和聚合。

真实推理系统会进一步涉及：

- 线程安全队列；
- 条件变量；
- 动态批处理；
- 调度公平性；
- 超时和取消；
- 背压；
- 多GPU资源调度。

今天的调度器是单线程基础模型，不声称已经达到生产级。

### C++工程保底

面试常考：

- STL容器适配器；
- 大顶堆和小顶堆；
- 比较器；
- 算法复杂度；
- Lambda；
- erase-remove；
- 排序稳定性。

---

## 60. 今日产出

```text
9个独立练习源文件
1个推理任务调度器综合项目
1个测试文件
1份实验报告
1次可复现Git提交
```

---

## 61. Git提交建议

```bash
git status
git add day21
git commit -m "Learn container adaptors and standard algorithms"
```

提交前检查：

- 不提交二进制文件；
- 报告可以提交；
- 不提交故意触发未定义行为的正式测试；
- 文档写明编译命令；
- 检查所有练习文件名与文档一致。

---

## 62. 最终速记

```text
queue：FIFO，front取，back看队尾

stack：LIFO，top取

priority_queue：默认大顶堆，top为当前最高优先级

pop：只删除，不返回值

适配器：不公开begin/end，不能直接范围for

算法区间：[first,last)

find/find_if：失败返回end，解引用前必须检查

sort：不稳定；stable_sort：等价元素保持原顺序

remove_if：移动保留元素，不改变size

真正删除：remove_if + erase

accumulate：初始值决定累加类型

比较器：必须保持严格、一致，不能用>=代替>
```
