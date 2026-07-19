# 第 1 周·工作日 3：函数、引用、const、参数传递与首个可复现仓库

## 1. 今日任务是什么

今天不是单纯记忆函数语法，而是建立 C++ 最重要的接口设计意识：

> 一个函数需要什么数据、是否允许修改调用者的数据、计算结果如何返回，都应当从函数声明中看出来。

完成今天的任务后，你应该能够：

- 声明、定义和调用函数；
- 区分形参与实参；
- 使用返回值表达函数的计算结果；
- 解释值传递、引用传递和 `const` 引用传递的区别；
- 使用引用修改调用者变量；
- 使用 `const` 防止意外修改；
- 避免返回局部变量的引用；
- 将昨天和今天的代码整理为一个可克隆、可编译、可运行、可验证的 GitHub 仓库。

建议用时：约 **3 小时**。今天的最低产出不是“看完知识点”，而是：

1. 完成 8 道必做题；
2. 完成综合小项目；
3. 完成知识笔记；
4. 发布第一个可复现仓库。

---

## 2. 核心知识讲解

### 2.1 函数的基本组成

一个函数通常包含返回类型、函数名、参数列表和函数体：

```cpp
int add(int left, int right) {
    return left + right;
}
```

调用：

```cpp
int result = add(3, 5);
```

其中：

- `int`：返回类型；
- `add`：函数名；
- `left`、`right`：形参；
- `3`、`5`：调用时传入的实参；
- `return`：将结果返回给调用者。

如果函数不需要返回结果，使用 `void`：

```cpp
void print_line() {
    std::cout << "----------\n";
}
```

### 2.2 函数声明与函数定义

如果函数定义写在 `main` 后面，需要先声明：

```cpp
#include <iostream>

int add(int left, int right);  // 声明

int main() {
    std::cout << add(3, 5) << '\n';
    return 0;
}

int add(int left, int right) {  // 定义
    return left + right;
}
```

函数声明告诉编译器“这个函数存在以及怎样调用”，函数定义提供真正的实现。

### 2.3 值传递

```cpp
void increase(int value) {
    ++value;
}
```

调用 `increase(number)` 时，`value` 是 `number` 的副本。函数修改的是副本，不会改变外部的 `number`。

适合：

- `int`、`double`、`char`、`bool` 等体积较小的类型；
- 函数不需要修改调用者变量；
- 函数需要一份独立副本。

### 2.4 引用传递

```cpp
void increase(int& value) {
    ++value;
}
```

`int&` 表示 `value` 是调用者变量的别名。修改 `value` 就是在修改调用者的变量。

适合：

- 函数必须修改调用者对象；
- 需要返回多个结果时，可以暂时使用输出引用参数；
- 希望避免大型对象复制，同时又需要修改对象。

注意：引用参数意味着函数具有外部副作用，不能为了“显得高级”而到处使用。

### 2.5 const 引用传递

```cpp
void print_name(const std::string& name) {
    std::cout << name << '\n';
}
```

`const std::string&` 同时表达两个信息：

1. 不复制整个字符串；
2. 函数承诺不修改字符串。

适合：

- `std::string` 等可能较大的对象；
- 函数只读取、不修改参数；
- 希望避免不必要的复制。

对于 `int`、`double` 这样的小类型，通常直接值传递即可，没有必要全部写成 `const int&`。

### 2.6 const 的三个常见用途

#### 常量变量

```cpp
const double pi = 3.141592653589793;
```

初始化后不能再次赋值。

#### const 引用

```cpp
const std::string& text
```

允许读取，不允许通过该引用修改原对象。

#### 值参数上的 const

```cpp
double calculate(const double price) {
    // price 是副本，const 防止函数内部意外修改它
    return price * 0.9;
}
```

这里的 `const` 主要约束函数内部实现。对于简单值参数，可按需要使用，不必机械添加。

### 2.7 三种参数传递方式的选择

|需求|推荐写法|调用者数据会被修改吗|
|---|---|---:|
|读取小型数据|`int value`|否|
|读取大型对象|`const std::string& text`|否|
|必须修改调用者变量|`int& value`|是|
|需要独立副本|`std::string text`|否|

判断顺序：

```text
函数需要修改调用者数据吗？
├── 是：使用 T&
└── 否：参数很小吗？
    ├── 是：使用 T
    └── 否：通常使用 const T&
```

### 2.8 今天必须避免的错误

#### 错误一：希望修改外部变量，却使用值传递

```cpp
void reset(int value) {
    value = 0;
}
```

这个函数不会改变调用者的变量。

#### 错误二：只读参数使用普通引用

```cpp
void print_text(std::string& text);
```

如果不需要修改，应优先表达为：

```cpp
void print_text(const std::string& text);
```

#### 错误三：返回局部变量的引用

```cpp
int& wrong() {
    int value = 10;
    return value;  // 错误：函数结束后局部变量已经销毁
}
```

初学阶段优先返回值：

```cpp
int correct() {
    int value = 10;
    return value;
}
```

---

## 3. 开始前自测

先凭理解回答，做完练习后再修正：

1. 函数声明和函数定义有什么区别？
2. 形参和实参有什么区别？
3. `void` 函数和具有返回值的函数分别适合什么场景？
4. 值传递为什么不会修改调用者变量？
5. `int& value` 表达了什么语义？
6. `const std::string& text` 同时解决了哪两个问题？
7. 为什么通常直接传递 `int`，而不是传递 `const int&`？
8. 为什么不能返回局部变量的引用？
9. 如果一个函数需要产生一个主要计算结果，应该优先使用返回值还是输出引用参数？
10. 函数能够编译是否就意味着它的参数设计合理？为什么？

---

## 4. 编码要求

1. 使用 C++17；
2. 每道题单独保存；
3. 所有函数都在 `main` 前声明，在 `main` 后定义；
4. `main` 只负责读取输入、调用函数和输出结果；
5. 函数内部不要直接依赖全局变量；
6. 每题至少自行补充两组测试；
7. 使用警告选项编译：

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic 文件.cpp -o 程序名
```

---

## 5. 必做练习

### 练习 1：基础计算函数

文件：`01_basic_functions.cpp`

实现以下函数：

```cpp
int square(int value);
int absolute_value(int value);
bool is_even(int value);
```

程序读取一个整数，依次输出它的平方、绝对值和奇偶性。

输入样例：

```text
-6
```

输出样例：

```text
Square: 36
Absolute: 6
Even: true
```

验收重点：

- 三个函数都只使用值传递；
- `main` 中不重复实现计算逻辑；
- 测试 `0`、正数和负数。

思考：最小的 `int` 取绝对值可能存在什么特殊问题？当前阶段只需记录，不要求解决。

---

### 练习 2：圆的面积和周长

文件：`02_circle.cpp`

实现：

```cpp
double circle_area(double radius);
double circle_circumference(double radius);
bool is_valid_radius(double radius);
```

要求：

- 使用 `const double pi`；
- 半径小于或等于 `0` 时输出 `Invalid radius`；
- 正常结果保留两位小数；
- 不在两个计算函数中重复定义不同的圆周率数值。

输入样例：

```text
2.5
```

输出样例：

```text
Area: 19.63
Circumference: 15.71
```

必须测试：`-1`、`0`、`1`、`2.5`。

---

### 练习 3：观察值传递

文件：`03_pass_by_value.cpp`

给定函数签名：

```cpp
void try_to_reset(int value);
```

完成程序：

1. 在 `main` 中创建 `number = 10`；
2. 调用前输出 `number`；
3. 在函数中将 `value` 改为 `0`；
4. 分别在函数内部和调用结束后输出数值。

预期现象：

```text
Before: 10
Inside: 0
After: 10
```

在源文件末尾用注释解释为什么 `After` 仍然是 `10`。

---

### 练习 4：引用交换

文件：`04_swap_by_reference.cpp`

不使用 `std::swap`，实现：

```cpp
void swap_values(int& left, int& right);
```

输入样例：

```text
12 35
```

输出样例：

```text
Before: 12 35
After: 35 12
```

要求：

- 使用临时变量；
- 解释为什么这里必须使用引用传递；
- 测试两个数相等的情况。

---

### 练习 5：按升序整理两个数

文件：`05_sort_two.cpp`

实现：

```cpp
void sort_two(int& first, int& second);
```

如果 `first > second`，交换两者；否则保持不变。

输入样例：

```text
9 3
```

输出样例：

```text
3 9
```

必须测试：`9 3`、`3 9`、`5 5`、负数。

验收重点：函数名和引用参数应当清楚表达“调用后原变量可能发生变化”。

---

### 练习 6：安全除法与多个输出结果

文件：`06_safe_divide.cpp`

实现：

```cpp
bool safe_divide(int dividend,
                 int divisor,
                 int& quotient,
                 int& remainder);
```

行为：

- 除数为 `0` 时返回 `false`，不得执行除法；
- 成功时设置 `quotient` 和 `remainder`，然后返回 `true`。

输入样例 1：

```text
17 5
```

输出样例 1：

```text
Quotient: 3
Remainder: 2
```

输入样例 2：

```text
17 0
```

输出样例 2：

```text
Error: divisor cannot be zero
```

验收重点：

- `dividend` 和 `divisor` 不需要被修改，使用值传递；
- `quotient` 和 `remainder` 是输出参数，使用引用；
- `bool` 返回值表示操作是否成功。

---

### 练习 7：const 引用字符串统计

文件：`07_text_statistics.cpp`

实现：

```cpp
int count_character(const std::string& text, char target);
int count_spaces(const std::string& text);
void print_text(const std::string& text);
```

输入一整行文本和一个目标字符，输出原文本、目标字符出现次数和空格数。

输入样例：

```text
hello cpp world
l
```

输出样例：

```text
Text: hello cpp world
Target count: 3
Spaces: 2
```

要求：

- 文本参数必须使用 `const std::string&`；
- 使用范围 `for` 循环遍历字符串；
- 不允许在函数中修改 `text`；
- 正确处理包含空格的整行输入。

思考：如果参数写成 `std::string text`，程序结果是否不同？运行成本可能有什么区别？

---

### 练习 8：价格折扣

文件：`08_discount.cpp`

实现：

```cpp
bool apply_discount(double& price, double discount_rate);
```

规则：

- `price` 必须大于或等于 `0`；
- `discount_rate` 必须位于 `[0, 1]`；
- 参数合法时，直接修改 `price` 并返回 `true`；
- 参数非法时，不修改 `price` 并返回 `false`。

输入样例：

```text
200 0.15
```

输出样例：

```text
Final price: 170.00
```

必须测试：

- `200 0.15`
- `200 0`
- `200 1`
- `200 -0.1`
- `200 1.1`
- `-1 0.5`

验收重点：非法输入时必须保持原价格不变。

---

## 6. 综合小项目：三数统计工具

文件：`09_number_summary.cpp`

输入三个整数，使用函数输出：

- 最大值；
- 最小值；
- 平均值；
- 三个数中偶数的数量；
- 将第一个数和第三个数交换后的结果。

至少实现：

```cpp
int max_of_three(int first, int second, int third);
int min_of_three(int first, int second, int third);
double average_of_three(int first, int second, int third);
int count_even(int first, int second, int third);
void swap_values(int& left, int& right);
void print_summary(int first, int second, int third);
```

输入样例：

```text
8 -2 5
```

输出样例：

```text
Maximum: 8
Minimum: -2
Average: 3.67
Even count: 2
After swapping first and third: 5 -2 8
```

项目要求：

- `main` 不直接计算最大值、最小值或平均值；
- 平均值不能发生整数除法；
- 输出保留两位小数；
- 函数只获取真正需要的参数；
- 测试全为正数、包含负数、全部相等和包含零的情况。

完成后回答：

1. 哪些函数使用值传递？为什么？
2. 哪个函数使用引用传递？为什么？
3. `print_summary` 是否必须使用引用？
4. 哪些函数是“有副作用”的？

---

## 7. 综合调试练习

下面的程序包含多个接口设计或生命周期问题。先打开警告编译，再逐项修复。

```cpp
#include <iostream>
#include <string>

void reset_value(int value) {
    value = 0;
}

void print_message(std::string& message) {
    std::cout << message << '\n';
}

int& create_result(int left, int right) {
    int result = left + right;
    return result;
}

double average(int left, int right) {
    return (left + right) / 2;
}

int main() {
    int number = 10;
    reset_value(number);
    std::cout << "Number: " << number << '\n';

    const std::string message = "function practice";
    print_message(message);

    std::cout << "Result: " << create_result(3, 5) << '\n';
    std::cout << "Average: " << average(3, 4) << '\n';
    return 0;
}
```

需要定位并解释：

1. 为什么 `reset_value` 没有达到函数名表达的效果？
2. 为什么 `print_message(message)` 无法通过编译？
3. `create_result` 返回的引用为什么无效？
4. `average(3, 4)` 为什么得不到 `3.5`？
5. 每个函数修复后应该使用哪种参数传递方式？

---

## 8. 选做提高题

### 选做 1：函数重载初体验

实现两个同名函数：

```cpp
int maximum(int left, int right);
double maximum(double left, double right);
```

分别调用两个版本，并观察编译器如何根据参数类型选择函数。

### 选做 2：只读与可修改接口对比

分别实现：

```cpp
void append_suffix(std::string& text, const std::string& suffix);
std::string with_suffix(const std::string& text,
                        const std::string& suffix);
```

比较：

- 第一个函数如何影响原字符串；
- 第二个函数是否修改原字符串；
- 两种接口分别适合什么场景。

---

## 9. 整理知识笔记

创建 `notes/day03-functions.md`，至少包含以下内容：

```markdown
# C++ 函数、引用和 const

## 函数声明、定义和调用

## 形参与实参

## 值传递
- 特点：
- 适用场景：
- 示例：

## 引用传递
- 特点：
- 适用场景：
- 示例：

## const 引用传递
- 特点：
- 适用场景：
- 示例：

## 参数传递选择规则

## 今天遇到的错误
1.
2.
3.

## 我仍然不理解的问题
```

不要整段复制教程。每一节至少写一个自己的例子，并说明为什么选择对应的参数传递方式。

---

## 10. 发布第一个可复现仓库

### 10.1 什么叫“可复现”

不是把若干 `.cpp` 文件上传就结束。一个陌生人克隆仓库后，应该能知道：

- 这个仓库学习什么；
- 使用什么编译器和C++标准；
- 目录中每个文件做什么；
- 怎样编译；
- 怎样运行；
- 正确结果是什么；
- 你已经完成了哪些内容。

### 10.2 推荐仓库结构

仓库名建议：`cpp-foundation-labs`

```text
cpp-foundation-labs/
├── README.md
├── .gitignore
├── day02-basics/
│   ├── 01_profile.cpp
│   ├── 02_time_convert.cpp
│   └── ...
├── day03-functions/
│   ├── 01_basic_functions.cpp
│   ├── 02_circle.cpp
│   ├── ...
│   └── 09_number_summary.cpp
└── notes/
    ├── day02-basics.md
    └── day03-functions.md
```

不要提交：

- 编译生成的可执行文件；
- 编辑器缓存；
- 系统生成文件；
- 临时日志。

`.gitignore` 至少可以包含：

```gitignore
.DS_Store
.vscode/
build/
*.out
*.o
```

### 10.3 根目录 README 必须包含

1. 项目目的；
2. 当前学习阶段；
3. 环境要求；
4. 目录说明；
5. 编译与运行命令；
6. 已完成题目；
7. 测试样例；
8. 已知限制；
9. 后续计划。

README 中的最小编译示例：

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic \
  day03-functions/09_number_summary.cpp \
  -o number_summary

./number_summary
```

### 10.4 本地验证流程

发布前必须执行：

1. 删除本地生成的可执行文件；
2. 按README中的命令重新编译；
3. 运行README中的样例输入；
4. 确认输出与文档一致；
5. 检查仓库中没有二进制文件或无关大文件；
6. 再提交和推送。

基础Git流程：

```bash
git status
git add README.md .gitignore day02-basics day03-functions notes
git status
git commit -m "Add reproducible C++ foundation labs"
git push
```

如果这是全新的本地仓库，则需要先按照GitHub新仓库页面给出的地址配置远程仓库。

### 10.5 最终复现测试

推送完成后，不要只看GitHub网页。应在另一个临时目录重新克隆：

```bash
git clone 你的仓库地址
cd cpp-foundation-labs
```

然后完全按照README编译和运行综合项目。只有这个过程成功，才能称为“可复现仓库”。

---

## 11. 今日验收清单

- [ ] 能独立写出函数声明、定义和调用；
- [ ] 能解释值传递为什么不修改调用者变量；
- [ ] 能使用引用参数修改调用者变量；
- [ ] 能使用 `const std::string&` 读取字符串而不复制、不修改；
- [ ] 知道不能返回局部变量的引用；
- [ ] 完成8道必做题；
- [ ] 完成三数统计综合项目；
- [ ] 完成综合调试练习；
- [ ] 每道题通过正常数据和边界数据测试；
- [ ] 完成 `notes/day03-functions.md`；
- [ ] README中的编译命令能够直接执行；
- [ ] 仓库不包含编译产物；
- [ ] 从新目录克隆仓库后能够复现；
- [ ] 已将仓库推送到GitHub。

## 12. 今日完成标准

达到下面的状态，才算真正完成今天的任务：

> 给你一个函数需求，你能判断应该使用值、引用还是 `const` 引用传递；写出的程序能在警告开启的情况下编译；其他人可以只阅读README就复现你的综合练习。

如果时间不足，优先级为：

1. 理解三种参数传递；
2. 完成练习1～8；
3. 完成综合项目；
4. 完成笔记和仓库复现；
5. 最后再做选做题。
