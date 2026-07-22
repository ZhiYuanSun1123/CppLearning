# 第 2 周·工作日 3：使用 AddressSanitizer 定位内存错误

## 1. 今日任务

今天的目标是使用 AddressSanitizer（简称 ASan）定位三类典型内存错误：

- 数组越界；
- 悬空指针和释放后使用；
- 重复释放。

完成后你应该能够：

- 使用ASan编译并运行C++程序；
- 区分编译错误、普通运行错误和ASan内存错误；
- 从报告中识别错误类型；
- 找到非法访问发生的位置；
- 找到动态内存最初分配的位置；
- 找到动态内存第一次释放的位置；
- 根据报告修复数组边界和对象生命周期；
- 修复后重新运行并确认ASan不再报告错误；
- 将错误代码、修复代码和分析记录整理进可复现仓库。

建议用时：约 **3小时**。

今天需要主动运行包含内存错误的教学程序。所有危险代码必须：

1. 单独保存在练习目录中；
2. 使用ASan编译；
3. 不放入真实项目；
4. 不处理真实数据；
5. 每次只观察一种主要错误；
6. 修复后重新验证。

---

## 2. AddressSanitizer是什么

AddressSanitizer是一种运行时内存错误检测工具。编译器会在程序中插入额外检查逻辑，在程序执行非法内存操作时输出报告。

它常用于检测：

- 栈数组越界；
- 堆数组越界；
- 释放后使用；
- 重复释放；
- 错误的释放方式；
- 部分作用域结束后使用；
- 部分内存泄漏问题，具体支持取决于平台。

ASan不是静态代码检查器。程序必须真正执行到错误路径，ASan才有机会发现问题。

例如：

```cpp
int values[3] = {10, 20, 30};
std::cout << values[10] << '\n';
```

如果这行代码没有被执行，ASan不会凭空报告它。

---

## 3. 编译与运行方式

### 3.1 推荐编译命令

macOS通常使用Apple Clang：

```bash
clang++ -std=c++17 -g -O1 \
  -Wall -Wextra -Wpedantic \
  -fsanitize=address \
  -fno-omit-frame-pointer \
  example.cpp -o example_asan
```

如果你的 `g++` 实际指向Apple Clang，也可以使用：

```bash
g++ -std=c++17 -g -O1 \
  -Wall -Wextra -Wpedantic \
  -fsanitize=address \
  -fno-omit-frame-pointer \
  example.cpp -o example_asan
```

参数含义：

|参数|作用|
|---|---|
|`-std=c++17`|使用C++17|
|`-g`|保留调试信息，使报告显示源码位置|
|`-O1`|使用较低优化级别，兼顾检测与可读性|
|`-Wall -Wextra -Wpedantic`|打开常用编译警告|
|`-fsanitize=address`|启用AddressSanitizer|
|`-fno-omit-frame-pointer`|保留调用栈信息|

运行：

```bash
./example_asan
```

### 3.2 调试阶段不要先使用高优化

初学时不要使用：

```bash
-O3
```

高优化可能改变代码结构，使源码行和调用栈更难理解。今天统一使用 `-O1`。

### 3.3 保存ASan报告

ASan通常将报告输出到标准错误。可以保存：

```bash
./example_asan 2> asan-report.txt
```

如果希望屏幕显示的同时保存：

```bash
./example_asan 2>&1 | tee asan-report.txt
```

报告中包含每次运行不同的地址，因此不要求地址数值完全一致。

### 3.4 退出码

检测到严重内存错误时，程序通常非正常退出。可以在运行后查看：

```bash
echo $?
```

正常程序通常返回 `0`。ASan终止的程序通常返回非零值，具体数字可能因平台而异。

---

## 4. 如何阅读ASan报告

一份ASan报告通常需要关注四类信息。

### 4.1 错误类型

常见关键词：

```text
stack-buffer-overflow
heap-buffer-overflow
heap-use-after-free
attempting double-free
alloc-dealloc-mismatch
```

看到报告后，先确定属于哪一类错误，不要先盯着十六进制地址。

### 4.2 非法操作类型

报告通常包含类似：

```text
READ of size 4
```

或者：

```text
WRITE of size 4
```

含义：

- `READ`：程序从非法地址读取；
- `WRITE`：程序向非法地址写入；
- `size 4`：本次访问4字节，常见于一个 `int`。

### 4.3 错误发生位置

报告会提供调用栈，例如：

```text
#0 ... in main example.cpp:8
```

优先找到自己代码中的第一个源码位置。系统库内部的调用栈通常不是第一修复目标。

### 4.4 分配与释放位置

对于动态内存，报告还可能说明：

```text
previously allocated by thread T0 here:
```

以及：

```text
freed by thread T0 here:
```

这三组位置共同回答：

```text
对象在哪里创建？
对象在哪里第一次释放？
错误访问或再次释放发生在哪里？
```

### 4.5 地址与边界说明

ASan可能写出：

```text
0 bytes after 20-byte region
```

这通常说明访问位置正好位于合法区域末尾之后。

例如5个 `int`，每个4字节，总区域通常为20字节。访问下标5时，位置就在20字节区域之后。

---

## 5. 标准排错流程

每个练习都使用以下流程：

```text
1. 阅读代码并预测错误
2. 使用ASan编译
3. 运行并触发错误
4. 记录错误类型
5. 找到源码行
6. 找到分配和释放位置
7. 解释根因
8. 修改代码
9. 重新编译
10. 重新运行，确认零ASan错误
```

不要只完成“看见红色报告”。真正的验收是修复后重新运行成功。

---

## 6. 开始前自测

1. 为什么只使用 `-Wall` 不一定能发现运行时数组越界？
2. ASan是在编译时、运行时还是两者配合工作？
3. 为什么编译时需要加入 `-g`？
4. `READ of size 4` 表示什么？
5. `heap-buffer-overflow` 与 `stack-buffer-overflow` 有什么区别？
6. `heap-use-after-free` 表示什么？
7. `attempting double-free` 表示什么？
8. 为什么一个程序可能同时有多个错误，但第一次只看到一个报告？
9. 修复后为什么必须重新编译？
10. ASan没有报错是否能证明程序在所有输入下绝对安全？

---

## 7. 环境验证实验

### 实验1：确认正常程序不会报错

文件：`01_asan_clean_baseline.cpp`

```cpp
#include <iostream>

int main() {
    int values[3] = {10, 20, 30};

    for (int index = 0; index < 3; ++index) {
        std::cout << values[index] << '\n';
    }

    return 0;
}
```

使用ASan编译运行：

```bash
clang++ -std=c++17 -g -O1 \
  -Wall -Wextra -Wpedantic \
  -fsanitize=address -fno-omit-frame-pointer \
  01_asan_clean_baseline.cpp -o 01_asan_clean_baseline

./01_asan_clean_baseline
```

验收：

- 正常输出3个数；
- 没有ASan错误；
- 退出码为0。

如果正常程序都无法运行，应先解决编译器或环境问题，再进行危险实验。

---

## 8. 必做实验

### 实验2：栈数组越界

文件：`02_stack_buffer_overflow.cpp`

错误版本：

```cpp
#include <iostream>

int main() {
    int values[3] = {10, 20, 30};

    for (int index = 0; index <= 3; ++index) {
        std::cout << values[index] << '\n';
    }

    return 0;
}
```

任务：

1. 运行前预测哪个下标越界；
2. 使用ASan编译运行；
3. 找到 `stack-buffer-overflow`；
4. 判断是 `READ` 还是 `WRITE`；
5. 记录报错源码行；
6. 将循环条件修复为合法边界；
7. 重新编译运行并确认无报错。

需要回答：为什么 `index <= 3` 错，而 `index < 3` 正确？

---

### 实验3：堆数组写越界

文件：`03_heap_buffer_overflow.cpp`

错误版本：

```cpp
#include <iostream>

int main() {
    const int size = 5;
    int* values = new int[size]{};

    for (int index = 0; index <= size; ++index) {
        values[index] = index * 10;
    }

    delete[] values;
    values = nullptr;
    return 0;
}
```

任务：

1. 预测非法写入发生在哪个下标；
2. 找到 `heap-buffer-overflow`；
3. 找到 `WRITE of size 4`；
4. 找到动态数组分配位置；
5. 根据报告判断合法区域大小；
6. 修复循环边界；
7. 重新验证。

需要记录：5个 `int` 在你的环境中占多少字节，ASan报告是否与之对应。

---

### 实验4：堆数组读越界

文件：`04_heap_read_overflow.cpp`

错误版本：

```cpp
#include <iostream>

int main() {
    const int size = 4;
    int* values = new int[size]{7, 14, 21, 28};

    std::cout << values[size] << '\n';

    delete[] values;
    values = nullptr;
    return 0;
}
```

任务：

- 找到 `heap-buffer-overflow`；
- 确认本次是非法读取；
- 解释为什么 `values[size]` 不合法；
- 改为访问最后一个合法元素；
- 重新验证零报错。

最后一个合法下标应通过表达式表示，不要直接写神秘数字。

---

### 实验5：释放后使用

文件：`05_heap_use_after_free.cpp`

错误版本：

```cpp
#include <iostream>

int main() {
    int* pointer = new int(42);
    delete pointer;

    std::cout << *pointer << '\n';

    return 0;
}
```

任务：

1. 找到 `heap-use-after-free`；
2. 找到非法读取位置；
3. 找到第一次释放位置；
4. 找到最初分配位置；
5. 解释为什么偶尔还能看到旧值也不代表安全；
6. 修改代码，使对象在使用完成后才释放；
7. 释放后设置为 `nullptr`；
8. 重新验证。

注意：把指针设置为 `nullptr` 后再解引用，仍然是错误。正确修复是调整生命周期，不是把一种非法访问换成另一种非法访问。

---

### 实验6：别名导致的悬空指针

文件：`06_alias_use_after_free.cpp`

错误版本：

```cpp
#include <iostream>

int main() {
    int* owner = new int(99);
    int* observer = owner;

    delete owner;
    owner = nullptr;

    std::cout << *observer << '\n';
    return 0;
}
```

任务：

- 观察 `heap-use-after-free`；
- 找到访问、释放和分配的三处位置；
- 解释为什么把 `owner` 设置为 `nullptr` 没有自动修改 `observer`；
- 在对象仍然存活时完成读取；
- 释放后不再使用 `observer`，并将它设为 `nullptr`；
- 重新验证。

重点：置空一个指针不能自动修复所有指向同一对象的其他指针。

---

### 实验7：重复释放

文件：`07_double_free.cpp`

错误版本：

```cpp
#include <iostream>

int main() {
    int* pointer = new int(42);

    delete pointer;
    delete pointer;

    return 0;
}
```

任务：

1. 找到 `attempting double-free`；
2. 找到第二次释放位置；
3. 找到第一次释放位置；
4. 找到分配位置；
5. 删除重复释放；
6. 第一次释放后将指针设为 `nullptr`；
7. 重新验证。

需要说明：真正的修复目标是确保所有权清晰、资源只释放一次，而不只是让报告暂时消失。

---

### 实验8：new[]和delete不匹配

文件：`08_alloc_dealloc_mismatch.cpp`

错误版本：

```cpp
#include <iostream>

int main() {
    int* values = new int[5]{};
    values[0] = 10;

    delete values;
    return 0;
}
```

任务：

- 观察平台是否报告 `alloc-dealloc-mismatch`；
- 找到分配和释放位置；
- 将释放方式修复为 `delete[]`；
- 设置为 `nullptr`；
- 重新验证。

不同编译器的报告文字可能略有区别，但代码本身必须严格遵守：

```text
new     ↔ delete
new[]   ↔ delete[]
```

---

## 9. 报告分析练习

为实验2～8分别填写以下模板：

```markdown
### 实验名称

- 预测错误：
- ASan错误类型：
- READ还是WRITE：
- 非法访问/释放源码位置：
- 分配位置：
- 第一次释放位置：
- 根本原因：
- 修复方式：
- 修复后退出码：
- 修复后是否还有ASan报告：
```

不是每种报告都有全部字段。例如栈数组越界没有动态分配和 `delete` 位置，此时写“不适用”。

---

## 10. 综合练习：逐个修复动态成绩程序

文件：`09_buggy_score_analyzer.cpp`

以下代码故意包含多个错误。ASan通常会在遇到第一个严重错误后终止，因此你需要采用“发现一个、修复一个、重新编译、继续运行”的方式。

```cpp
#include <iostream>

long long calculate_sum(const int* scores, int size) {
    long long sum = 0;

    for (int index = 0; index <= size; ++index) {
        sum += scores[index];
    }

    return sum;
}

int main() {
    const int size = 5;
    int* scores = new int[size]{80, 91, 76, 88, 95};

    std::cout << "Sum: " << calculate_sum(scores, size) << '\n';

    int* first_score = scores;

    delete[] scores;
    scores = nullptr;

    std::cout << "First score: " << *first_score << '\n';

    delete[] first_score;
    return 0;
}
```

程序中至少包含：

- 堆数组越界读取；
- 释放后使用；
- 对同一动态数组的重复释放风险。

修复流程：

1. 保存原始错误版本；
2. 运行并保存第一次报告；
3. 只修复第一个错误；
4. 重新编译运行；
5. 保存第二次报告；
6. 继续修复下一个错误；
7. 最终得到干净版本；
8. 最终版本输出总分和第一个成绩；
9. 最终版本ASan零报错；
10. 在笔记中记录每次错误出现的先后顺序。

预期正确结果：

```text
Sum: 430
First score: 80
```

注意：不要通过删除全部功能来“修复”程序。最终版本仍需完成原有正确功能。

---

## 11. 修复昨天的代码

从昨天的动态内存练习中至少选择两个程序重新验证：

- `07_create_destroy_array.cpp`；
- `11_dynamic_score_analyzer.cpp`。

使用ASan重新编译：

```bash
clang++ -std=c++17 -g -O1 \
  -Wall -Wextra -Wpedantic \
  -fsanitize=address -fno-omit-frame-pointer \
  07_create_destroy_array.cpp \
  -o 07_create_destroy_array_asan

./07_create_destroy_array_asan
```

测试：

- 正常长度；
- 长度为0；
- 负数长度；
- 正常输入；
- 非法输入路径；
- 空指针销毁路径。

验收：所有测试均无ASan报告，并且程序输出符合预期。

---

## 12. 可选：内存泄漏检测说明

部分平台可以通过ASan或LeakSanitizer检测泄漏，但macOS上的具体支持可能随编译器版本和运行环境变化。

可以尝试：

```bash
ASAN_OPTIONS=detect_leaks=1 ./program_asan
```

如果当前Apple Clang环境不支持泄漏检测，不要把环境限制误认为程序一定没有泄漏。今天的必做验收聚焦：

- 越界；
- 释放后使用；
- 重复释放；
- 分配释放不匹配。

泄漏检测工具会在后续工程阶段继续补充。

---

## 13. 今日笔记模板

创建：

```text
notes/day06-address-sanitizer.md
```

内容：

```markdown
# AddressSanitizer内存错误定位

## ASan解决什么问题

## 编译参数
- -g：
- -O1：
- -fsanitize=address：
- -fno-omit-frame-pointer：

## 报告阅读顺序
1.
2.
3.
4.

## stack-buffer-overflow
- 触发代码：
- 报告关键内容：
- 修复方式：

## heap-buffer-overflow
- 触发代码：
- 报告关键内容：
- 修复方式：

## heap-use-after-free
- 触发代码：
- 分配位置：
- 释放位置：
- 非法访问位置：
- 修复方式：

## double-free
- 第一次释放位置：
- 第二次释放位置：
- 修复方式：

## alloc-dealloc-mismatch

## ASan的局限

## 今天修复的三个主要错误
1.
2.
3.

## 仍然不理解的问题
```

---

## 14. 更新可复现仓库

建议目录：

```text
cpp-foundation-labs/
├── day06-address-sanitizer/
│   ├── README.md
│   ├── buggy/
│   │   ├── 02_stack_buffer_overflow.cpp
│   │   ├── 03_heap_buffer_overflow.cpp
│   │   ├── 04_heap_read_overflow.cpp
│   │   ├── 05_heap_use_after_free.cpp
│   │   ├── 06_alias_use_after_free.cpp
│   │   ├── 07_double_free.cpp
│   │   ├── 08_alloc_dealloc_mismatch.cpp
│   │   └── 09_buggy_score_analyzer.cpp
│   ├── fixed/
│   │   ├── 02_stack_buffer_overflow_fixed.cpp
│   │   ├── 03_heap_buffer_overflow_fixed.cpp
│   │   ├── 04_heap_read_overflow_fixed.cpp
│   │   ├── 05_heap_use_after_free_fixed.cpp
│   │   ├── 06_alias_use_after_free_fixed.cpp
│   │   ├── 07_double_free_fixed.cpp
│   │   ├── 08_alloc_dealloc_mismatch_fixed.cpp
│   │   └── 09_score_analyzer_fixed.cpp
│   └── reports/
│       └── README.md
└── notes/
    └── day06-address-sanitizer.md
```

不要提交：

- 编译后的可执行文件；
- 含有本机绝对路径的大量原始报告；
- core dump；
- 无法解释的截图。

报告目录可以保存经过整理和脱敏的关键片段，重点保留：

- 错误类型；
- 源码行；
- 分配位置；
- 释放位置；
- 根因和修复。

README必须明确：

> `buggy/`中的文件故意包含未定义行为，只用于ASan教学，不应复制到生产代码。

提交示例：

```bash
git status
git add README.md day06-address-sanitizer notes/day06-address-sanitizer.md
git commit -m "Add AddressSanitizer memory error labs"
git push
```

---

## 15. 今日验收清单

- [ ] 能使用ASan参数编译程序；
- [ ] 正常基线程序零报错且退出码为0；
- [ ] 能识别 `stack-buffer-overflow`；
- [ ] 能识别 `heap-buffer-overflow`；
- [ ] 能区分非法读取和非法写入；
- [ ] 能识别 `heap-use-after-free`；
- [ ] 能识别 `attempting double-free`；
- [ ] 能识别 `new[]/delete` 不匹配；
- [ ] 能从报告找到自己代码的源码行；
- [ ] 能找到动态内存的分配位置；
- [ ] 能找到第一次释放位置；
- [ ] 每个错误版本都已产生预期报告；
- [ ] 每个修复版本都重新编译和运行；
- [ ] 修复版本全部无ASan错误；
- [ ] 完成综合程序的逐轮修复；
- [ ] 使用ASan重新验证至少两个昨天的程序；
- [ ] 完成ASan笔记；
- [ ] 仓库明确区分 `buggy` 和 `fixed`；
- [ ] 没有把可执行文件提交到Git仓库。

## 16. 今日完成标准

达到下面的状态才算完成：

> 看到ASan报告后，你能够先识别错误类型，再找到非法操作、分配和释放位置，解释对象生命周期为什么失效，并通过重新运行证明修复有效。

如果时间不足，优先级为：

1. 完成环境验证；
2. 完成越界、释放后使用和重复释放三个核心实验；
3. 完成综合程序逐轮修复；
4. 用ASan复查昨天的代码；
5. 完成笔记和仓库整理；
6. 最后尝试泄漏检测。
