# 第 2 周·工作日 2：栈、堆、对象生命周期与 `new/delete` 风险

## 1. 今日任务

今天的核心不是背诵“栈快、堆慢”，而是理解：

> 一个对象从什么时候开始存在、什么时候失效、谁负责释放它，以及错误的生命周期管理为什么会造成内存泄漏、悬空指针和程序崩溃。

完成今天的任务后，你应该能够：

- 解释自动存储期对象和动态存储期对象的生命周期；
- 理解局部变量离开作用域后为什么不能继续访问；
- 使用 `new` 创建单个对象和动态数组；
- 正确匹配 `new/delete` 与 `new[]/delete[]`；
- 识别内存泄漏、重复释放、释放后使用和悬空指针；
- 说明 `delete` 为什么不会自动把指针改成 `nullptr`；
- 为简单裸指针代码明确资源所有者；
- 理解为什么现代 C++ 更推荐 RAII、容器和智能指针。

建议用时：约 **3小时**。

今天会使用裸 `new/delete` 做实验，但需要牢记：

> 学习裸指针资源管理，是为了理解它的风险。后续真实工程应优先使用自动对象、`std::vector`、RAII和智能指针。

---

## 2. 核心知识讲解

### 2.1 作用域与生命周期不是同一个词

作用域回答：

> 在代码的哪些位置可以通过这个名字访问对象？

生命周期回答：

> 这个对象从什么时候开始存在，到什么时候结束？

例如：

```cpp
int main() {
    int outer = 10;

    {
        int inner = 20;
        std::cout << inner << '\n';
    }

    // 此处不能再使用名字 inner
    return 0;
}
```

`inner` 在进入代码块时创建，离开代码块时生命周期结束；同时它的名字也不再处于可见作用域。

### 2.2 自动存储期对象

普通局部变量通常具有自动存储期：

```cpp
void function() {
    int number = 42;
    double price = 19.9;
}
```

进入作用域时对象开始存在，离开作用域时自动销毁，不需要手动释放。

常见例子：

```cpp
int value = 10;
int values[5] = {1, 2, 3, 4, 5};
std::string message = "hello";
```

这些局部对象的资源会按照类型规则自动清理。

### 2.3 “栈对象”是一种常用但简化的说法

日常学习中，经常把自动存储期局部变量称作“栈上对象”。在常见实现中，它们确实通常放在调用栈中。

但严谨地说：

- C++语言标准定义的是存储期和生命周期；
- “栈”和“堆”包含具体实现层面的概念；
- 初学阶段可以使用“栈对象”和“堆对象”帮助理解，但要知道这是简化模型。

### 2.4 动态存储期对象

如果使用 `new` 创建对象，对象不会因为创建它的局部作用域结束而自动销毁：

```cpp
int* pointer = new int(42);
```

这里发生了两件事：

1. 动态创建一个值为 `42` 的 `int` 对象；
2. 返回这个对象的地址，并保存在 `pointer` 中。

使用结束后必须释放：

```cpp
delete pointer;
pointer = nullptr;
```

### 2.5 指针变量和动态对象是两个不同对象

```cpp
int* pointer = new int(42);
```

应区分：

```text
pointer
├── 是一个局部指针变量
├── 自己通常具有自动存储期
└── 保存动态对象的地址

new int(42)
├── 是动态创建的int对象
└── 必须由拥有者负责释放
```

如果 `pointer` 离开作用域，但动态对象没有被 `delete`，指针变量会消失，动态对象却仍然占据内存。这就是内存泄漏。

### 2.6 delete不会删除指针变量

```cpp
int* pointer = new int(42);
delete pointer;
```

`delete pointer` 销毁的是 `pointer` 所指的动态对象，不是局部指针变量本身。

释放后，`pointer` 仍然保存原来的地址，但该地址上的对象已经不存在。这时 `pointer` 是悬空指针。

因此建议：

```cpp
delete pointer;
pointer = nullptr;
```

设置为 `nullptr` 不能修复所有所有权问题，但能降低同一指针再次误用的风险。

### 2.7 单对象必须使用delete

```cpp
int* pointer = new int(42);

std::cout << *pointer << '\n';

delete pointer;
pointer = nullptr;
```

基本规则：

```text
new T(...)  必须匹配 delete
```

### 2.8 动态数组必须使用delete[]

```cpp
int size = 5;
int* values = new int[size]{};

for (int index = 0; index < size; ++index) {
    values[index] = index * 10;
}

delete[] values;
values = nullptr;
```

规则：

```text
new T[size] 必须匹配 delete[]
```

错误匹配属于未定义行为：

```cpp
int* values = new int[5];
delete values;  // 错误，应使用delete[]
```

### 2.9 初始化动态对象

下面的动态 `int` 没有显式初始化：

```cpp
int* pointer = new int;
```

对初学者更安全清晰的写法是：

```cpp
int* pointer = new int(0);
```

动态数组也建议值初始化：

```cpp
int* values = new int[size]{};
```

花括号会把基本类型元素初始化为零。

### 2.10 delete nullptr是安全的

```cpp
int* pointer = nullptr;
delete pointer;
```

删除空指针不会产生效果，是安全的。因此通常不需要写：

```cpp
if (pointer != nullptr) {
    delete pointer;
}
```

但删除后仍建议把原指针设置为 `nullptr`。

### 2.11 内存泄漏

```cpp
void leak() {
    int* pointer = new int(42);
    std::cout << *pointer << '\n';
    // 忘记delete
}
```

函数结束后，局部指针 `pointer` 消失，再也无法通过它找到那块动态内存，但动态对象仍未被释放。

循环中的泄漏会不断累积：

```cpp
for (int index = 0; index < 1000; ++index) {
    int* pointer = new int(index);
    // 每轮都泄漏
}
```

### 2.12 释放后使用

```cpp
int* pointer = new int(42);
delete pointer;

std::cout << *pointer << '\n';  // 错误：释放后使用
```

对象已经不存在，即使程序偶尔打印出 `42`，也不代表代码正确。这属于未定义行为。

### 2.13 重复释放

```cpp
int* pointer = new int(42);
delete pointer;
delete pointer;  // 错误：重复释放
```

改为：

```cpp
delete pointer;
pointer = nullptr;
delete pointer;  // 删除nullptr本身安全，但不要依赖这种写法掩盖所有权混乱
```

关键并不是“每次delete后机械置空”，而是明确某个资源只能由一个清晰的所有者负责释放一次。

### 2.14 指针别名带来的风险

```cpp
int* first = new int(42);
int* second = first;

delete first;
first = nullptr;
```

此时 `second` 仍然保存已释放对象的旧地址，仍然是悬空指针：

```cpp
std::cout << *second << '\n';  // 错误
```

把 `first` 置空不会自动修改所有保存同一地址的其他指针。

这说明裸指针共享地址时，资源所有权很容易变得不清晰。

### 2.15 返回局部变量地址是错误的

```cpp
int* wrong() {
    int value = 42;
    return &value;
}
```

函数结束时，`value` 生命周期已经结束，返回的地址立即失效。

应优先返回值：

```cpp
int correct() {
    int value = 42;
    return value;
}
```

### 2.16 返回动态对象会转移清理责任

下面的代码技术上可以创建动态对象：

```cpp
int* create_value(int value) {
    return new int(value);
}
```

但调用者必须知道自己负责 `delete`：

```cpp
int* pointer = create_value(42);
delete pointer;
pointer = nullptr;
```

如果接口没有清晰说明所有权，很容易泄漏。现代C++通常不会优先采用这种裸指针所有权接口。

### 2.17 为什么现代C++减少直接new/delete

裸 `new/delete` 的主要问题不是语法复杂，而是所有路径都必须正确释放：

```cpp
int* pointer = new int(42);

if (some_error) {
    return;  // 如果忘记delete，就泄漏
}

delete pointer;
```

真实程序还会包含异常、多个返回分支和复杂所有权关系，手动释放很难长期保持正确。

后续你会学习：

- 自动存储期对象；
- `std::vector`；
- RAII；
- `std::unique_ptr`；
- `std::shared_ptr`。

这些机制让资源释放与对象生命周期绑定，减少人为遗漏。

---

## 3. 今日安全规则

1. 每个 `new` 都必须明确对应哪一个 `delete`；
2. 每个 `new[]` 都必须对应 `delete[]`；
3. `new` 和 `new[]` 不能混用清理方式；
4. 释放后不得继续解引用；
5. 释放后将原拥有指针设为 `nullptr`；
6. 一个动态对象只能释放一次；
7. 不返回局部变量的地址或引用；
8. 不覆盖仍然拥有动态对象地址的指针；
9. 动态数组分配前先验证长度；
10. 能使用局部变量或固定数组解决的问题，不要为了练习而强行动态分配；
11. 今天的危险代码先阅读修复，不直接运行；
12. 打开全部常用编译警告。

编译命令：

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic 文件.cpp -o 程序名
```

---

## 4. 开始前自测

先凭理解回答，完成练习后再回来修正：

1. 自动存储期局部变量通常在什么时候销毁？
2. `new int(42)` 创建的对象在什么时候销毁？
3. 指针变量离开作用域是否会自动释放它指向的动态对象？
4. `delete pointer` 删除的是指针还是所指对象？
5. `delete` 后为什么还可能存在悬空指针？
6. `new int[10]` 应该和哪种释放方式匹配？
7. 内存泄漏是什么意思？
8. 什么是释放后使用？
9. 什么是重复释放？
10. 为什么把一个拥有资源的指针直接覆盖会造成泄漏？
11. 为什么不能返回局部变量的地址？
12. 为什么真实工程更倾向RAII，而不是手动管理大量 `new/delete`？

---

## 5. 生命周期观察实验

### 实验1：嵌套作用域中的局部变量

文件：`01_scope_lifetime.cpp`

完成程序：

```cpp
#include <iostream>

int main() {
    int outer = 10;
    std::cout << "outer value: " << outer << '\n';
    std::cout << "outer address: " << &outer << '\n';

    {
        int inner = 20;
        std::cout << "inner value: " << inner << '\n';
        std::cout << "inner address: " << &inner << '\n';
    }

    // 尝试在这里访问inner，观察编译器错误，然后恢复为可编译状态
    return 0;
}
```

记录：

- `outer` 和 `inner` 的可见范围；
- `inner` 的生命周期终点；
- 为什么离开代码块后不能继续使用 `inner`。

### 实验2：函数局部变量生命周期

文件：`02_function_local.cpp`

实现：

```cpp
void observe_local(int call_number);
```

函数内部创建一个局部变量，输出调用编号、数值和地址。连续调用3次。

观察：

- 每次地址是否一定不同；
- 如果地址出现重复，是否意味着旧对象仍然存在；
- 为什么不能依赖局部变量某次运行时的具体地址。

正确结论：即使存储位置被后续调用复用，也不表示前一个对象的生命周期仍在继续。

### 实验3：局部指针与动态对象

文件：`03_pointer_and_dynamic_object.cpp`

在同一作用域中输出：

- 局部指针变量自身的地址：`&pointer`；
- 动态对象地址：`pointer`；
- 动态对象的值：`*pointer`。

框架：

```cpp
int* pointer = new int(42);

// 完成观察输出

delete pointer;
pointer = nullptr;
```

用自己的话说明“指针变量”和“指针所拥有的动态对象”为什么不是同一个对象。

### 实验4：动态数组初始化

文件：`04_dynamic_array_initialization.cpp`

比较：

```cpp
int* first = new int[5];
int* second = new int[5]{};
```

注意：不要读取 `first` 中尚未初始化的元素。只为它们主动赋值后再输出。

要求：

- 给 `first` 的5个元素主动赋值；
- 观察 `second` 的5个元素；
- 分别正确执行 `delete[]`；
- 分别设置为 `nullptr`；
- 解释 `{}` 的初始化效果。

---

## 6. 必做练习

### 练习1：单个动态整数

文件：`05_dynamic_integer.cpp`

输入一个整数，动态创建一个 `int` 保存它，然后输出：

- 动态对象的值；
- 动态对象的地址；
- 局部指针变量的地址。

最后正确释放资源。

验收清单：

- 使用 `new int(value)`；
- 使用 `delete`；
- 删除后设置为 `nullptr`；
- 删除后不再解引用。

---

### 练习2：动态数组求和

文件：`06_dynamic_array_sum.cpp`

用户输入数组长度 `size`，再输入全部元素，输出总和和平均值。

要求：

- `size` 必须在 `[1, 1000]`；
- 验证合法后再分配；
- 使用 `new int[size]{}`；
- 使用 `delete[]`；
- 释放后设置为 `nullptr`；
- 平均值不得发生整数除法。

输入样例：

```text
5
80 91 76 88 95
```

输出样例：

```text
Sum: 430
Average: 86.00
```

必须测试：`size`为 `0`、负数、`1`、`1000`和超过上限。

---

### 练习3：创建并销毁动态数组

文件：`07_create_destroy_array.cpp`

实现：

```cpp
int* create_sequence(int size);
void destroy_array(int*& data);
```

规则：

- `size <= 0` 时 `create_sequence` 返回 `nullptr`；
- 合法时创建数组，并依次填入 `1, 2, ..., size`；
- `destroy_array` 使用 `delete[]`；
- 释放后通过引用把调用者指针设为 `nullptr`。

在 `main` 中验证：

1. 创建前指针为空；
2. 创建后可以安全读取；
3. 释放后指针重新为空；
4. 对空指针调用 `destroy_array` 不崩溃。

思考：为什么 `destroy_array` 的参数是 `int*&`，而不是 `int*`？

---

### 练习4：深拷贝固定数据

文件：`08_copy_to_dynamic_array.cpp`

给定：

```cpp
const int source[] = {7, 14, 21, 28, 35};
```

实现：

```cpp
int* copy_array(const int* source, int size);
```

要求：

- 空指针或非正长度返回 `nullptr`；
- 创建新的动态数组；
- 将每个元素复制到新数组；
- 修改新数组的第一个元素，验证原数组不受影响；
- 调用者负责 `delete[]`。

需要回答：为什么只写 `int* copied = source` 不能称为复制数组？

---

### 练习5：查找结果与生命周期

文件：`09_find_in_dynamic_array.cpp`

实现：

```cpp
const int* find_value(const int* data, int size, int target);
```

在一个动态数组中查找目标值，找到时返回对应元素地址，没有找到时返回 `nullptr`。

正确流程：

1. 创建动态数组；
2. 调用查找函数；
3. 在数组仍然存在时检查和使用返回指针；
4. 释放数组；
5. 不再使用之前返回的元素指针。

在注释中解释：数组释放后，查找函数返回的元素指针为什么也会变成悬空指针。

---

### 练习6：修复所有权覆盖导致的泄漏

文件：`10_pointer_overwrite.cpp`

分析下面代码：

```cpp
int* pointer = new int(10);
pointer = new int(20);
delete pointer;
```

回答：

1. 哪个对象被释放？
2. 哪个对象泄漏？
3. 为什么第一块动态内存再也无法找到？

然后写出两个修复版本：

- 版本A：在重新赋值前释放旧对象；
- 版本B：两个动态对象分别由两个不同指针管理。

---

## 7. 综合项目：动态成绩分析器

文件：`11_dynamic_score_analyzer.cpp`

用户输入学生人数和成绩，程序动态创建数组并输出：

- 全部成绩；
- 总分；
- 平均分；
- 最高分；
- 最低分；
- 及格人数；
- 第一个不及格成绩及其位置；
- 动态数组首地址。

建议函数：

```cpp
bool is_valid_size(int size);
bool is_valid_score(int score);
void print_scores(const int* scores, int size);
long long calculate_sum(const int* scores, int size);
double calculate_average(const int* scores, int size);
bool find_max(const int* scores, int size, int& output);
bool find_min(const int* scores, int size, int& output);
int count_passed(const int* scores, int size);
const int* find_first_failed(const int* scores, int size);
```

约束：

- 学生人数范围为 `[1, 1000]`；
- 每个成绩必须位于 `[0, 100]`；
- 人数合法后才允许 `new[]`；
- 输入过程中发现非法成绩时，必须先释放已创建的数组再退出；
- 所有只读函数使用 `const int*`；
- 输出完成后使用 `delete[]`；
- 释放后将拥有指针设置为 `nullptr`；
- 不允许在释放后读取成绩或之前保存的元素地址。

输入样例：

```text
5
80 91 56 88 95
```

输出样例：

```text
Scores: 80 91 56 88 95
Sum: 410
Average: 82.00
Maximum: 95
Minimum: 56
Passed: 4
First failed score: 56
First failed index: 2
Array address: 0x...
```

如果所有人都及格：

```text
No failed score
```

需要在源文件末尾回答：

1. 动态数组的所有者是谁？
2. 哪一行开始了动态数组生命周期？
3. 哪一行结束了动态数组生命周期？
4. `find_first_failed` 返回的指针可以使用到什么时候？
5. 如果输入非法后直接 `return`，为什么可能发生泄漏？

---

## 8. 风险识别与调试练习

下面代码包含多种未定义行为或资源错误。**不要直接运行原代码**，先在纸面或注释中识别并修复。

```cpp
#include <iostream>

int* make_wrong_pointer() {
    int local = 42;
    return &local;
}

int main() {
    int* leaked = new int(10);
    leaked = new int(20);

    int* value = new int(30);
    delete value;
    std::cout << *value << '\n';
    delete value;

    int* values = new int[5]{};
    delete values;

    int* wrong = make_wrong_pointer();
    std::cout << *wrong << '\n';

    return 0;
}
```

找出并修复：

1. 指针覆盖造成的内存泄漏；
2. 释放后使用；
3. 重复释放；
4. `new[]` 与 `delete` 不匹配；
5. 返回局部变量地址；
6. 哪些指针应在释放后设为 `nullptr`；
7. 每个动态对象究竟由谁负责释放。

修复完成后再编译运行。下一工作日将使用AddressSanitizer验证这些问题。

---

## 9. 选做练习

### 选做1：动态数组扩容模拟

实现：

```cpp
int* resize_array(const int* old_data, int old_size, int new_size);
```

规则：

- `new_size <= 0` 返回 `nullptr`；
- 分配新数组；
- 复制能够保留的元素；
- 新增加的位置初始化为0；
- 函数只读取旧数组，不负责释放旧数组；
- 调用者接收新数组后负责释放旧数组和新数组。

重点不是实现高效容器，而是体验手动扩容为什么容易出错。后续应优先使用 `std::vector`。

### 选做2：所有权说明练习

阅读下面的函数声明，为每个参数和返回值写出所有权说明：

```cpp
int* create_array(int size);
void print_array(const int* data, int size);
void modify_array(int* data, int size);
void destroy_array(int*& data);
const int* find_max_address(const int* data, int size);
```

说明：

- 谁创建资源；
- 谁负责释放；
- 谁只借用；
- 谁可以修改；
- 返回指针的有效期依赖哪个对象。

---

## 10. 今日笔记模板

在仓库中创建：

```text
notes/day05-stack-heap-lifetime.md
```

内容模板：

```markdown
# 栈、堆、生命周期与new/delete

## 作用域与生命周期的区别

## 自动存储期对象

## 动态存储期对象

## 指针变量和动态对象的区别

## new/delete匹配规则

## new[]/delete[]匹配规则

## 内存泄漏
- 定义：
- 示例：
- 修复方式：

## 悬空指针和释放后使用

## 重复释放

## 所有权是什么

## 为什么不返回局部变量地址

## 为什么现代C++优先RAII

## 今天发现的三个错误
1.
2.
3.

## 尚未理解的问题
```

笔记必须使用自己的代码片段解释，不能只复制定义。

---

## 11. 更新可复现仓库

建议增加：

```text
cpp-foundation-labs/
├── day05-lifetime-dynamic-memory/
│   ├── 01_scope_lifetime.cpp
│   ├── 02_function_local.cpp
│   ├── 03_pointer_and_dynamic_object.cpp
│   ├── 04_dynamic_array_initialization.cpp
│   ├── 05_dynamic_integer.cpp
│   ├── 06_dynamic_array_sum.cpp
│   ├── 07_create_destroy_array.cpp
│   ├── 08_copy_to_dynamic_array.cpp
│   ├── 09_find_in_dynamic_array.cpp
│   ├── 10_pointer_overwrite.cpp
│   └── 11_dynamic_score_analyzer.cpp
└── notes/
    └── day05-stack-heap-lifetime.md
```

README增加：

- 自动对象与动态对象的生命周期说明；
- `new/delete`配对规则；
- 综合项目编译命令；
- 输入输出样例；
- 所有权约定；
- 风险声明：本目录使用裸指针仅用于学习，后续工程优先使用RAII。

编译综合项目：

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic \
  day05-lifetime-dynamic-memory/11_dynamic_score_analyzer.cpp \
  -o dynamic_score_analyzer

./dynamic_score_analyzer
```

提交：

```bash
git status
git add README.md day05-lifetime-dynamic-memory notes/day05-stack-heap-lifetime.md
git commit -m "Add object lifetime and dynamic memory experiments"
git push
```

---

## 12. 今日验收清单

- [ ] 能解释作用域与生命周期的区别；
- [ ] 能解释自动存储期与动态存储期；
- [ ] 能区分局部指针变量和它指向的动态对象；
- [ ] 能正确匹配 `new/delete`；
- [ ] 能正确匹配 `new[]/delete[]`；
- [ ] 知道 `delete` 后指针不会自动变为 `nullptr`；
- [ ] 能识别内存泄漏；
- [ ] 能识别悬空指针和释放后使用；
- [ ] 能识别重复释放；
- [ ] 能识别指针覆盖造成的泄漏；
- [ ] 不返回局部变量地址；
- [ ] 动态数组分配前先检查长度；
- [ ] 完成4个生命周期观察实验；
- [ ] 完成6道必做题；
- [ ] 完成动态成绩分析器；
- [ ] 修复风险识别代码后才运行；
- [ ] 完成笔记并更新仓库；
- [ ] 所有正常代码在警告开启时成功编译。

## 13. 今日完成标准

达到下面状态才算完成：

> 看到一段含有 `new/delete` 的代码时，你能指出动态对象的所有者、生命周期起点和终点；能够检查每条退出路径是否释放资源；不会把已经释放的地址继续当作有效对象使用。

如果时间不足，优先级为：

1. 理解生命周期、所有权和四类风险；
2. 完成4个观察实验；
3. 完成练习1～6；
4. 完成综合项目；
5. 完成风险修复与笔记；
6. 最后再做选做题。
