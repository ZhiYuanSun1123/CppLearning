# 第 4 周·工作日 1：理解拷贝构造、拷贝赋值和 Rule of Five

## 0. 前置知识与超纲说明

### 今天必须掌握

- “用已有对象创建新对象”和“给已有对象重新赋值”的区别；
- 拷贝构造函数的声明与调用时机；
- 拷贝赋值运算符的声明与调用时机；
- 默认拷贝为什么可能产生浅拷贝；
- 拥有裸指针资源的类为什么必须考虑深拷贝；
- 自赋值是什么，以及为什么要处理；
- Rule of Three（五法则之前的基础）；
- 移动构造和移动赋值的基本目的；
- Rule of Five 的五个特殊成员函数；
- `= delete`如何明确禁止复制；
- 优先使用 Rule of Zero 的工程思想；
- 使用 ASan 检查重复释放、释放后使用和内存泄漏。

### 今天会复习

- 构造函数、析构函数和成员初始化列表；
- 栈、堆与对象生命周期；
- `new[]`和`delete[]`必须配对；
- `const`引用参数；
- RAII；
- 类的声明与类外实现；
- 异常路径；
- 自定义测试运行器；
- 编译警告和 AddressSanitizer。

### 今天第一次系统学习

- `operator=`：拷贝赋值运算符；
- `this`指针；
- `if (this == &other)`自赋值判断；
- 右值引用`T&&`；
- `std::move`；
- 移动后对象仍然有效但状态可能改变；
- 为移动操作添加`noexcept`。

### 今天只需初步理解，后续还会继续使用

- 右值、左值和值类别的完整规则；
- 编译器何时隐式生成或删除特殊成员函数；
- 复制省略（copy elision）；
- 强异常安全保证；
- 容器为什么更偏爱`noexcept`移动构造；
- copy-and-swap写法；
- `std::unique_ptr`和`std::vector`如何帮助实现 Rule of Zero。

### 今天明确不做

- 不深入模板；
- 不实现通用容器；
- 不使用`memcpy`复制未知对象；
- 不研究完整的值类别体系；
- 不追求手写内存管理比标准容器更高效；
- 不把裸指针版本直接当成生产代码。

> 今天手写资源管理类是为了理解拷贝和移动的底层规则。工程实践中通常应优先让`std::string`、`std::vector`和智能指针管理资源，从而尽量使用 Rule of Zero。

---

## 1. 今日目标

完成今天的内容后，你应该能够：

- 判断一行代码调用的是普通构造、拷贝构造还是拷贝赋值；
- 解释默认浅拷贝为什么可能导致两个对象拥有同一块堆内存；
- 为拥有动态数组的类实现深拷贝；
- 正确返回`*this`以支持连续赋值；
- 安全处理`object = object`；
- 解释移动操作为什么可以减少资源复制；
- 实现一个最小可用的移动构造和移动赋值；
- 说出 Rule of Five 的五个函数；
- 解释什么时候应该禁止复制；
- 解释为什么 Rule of Zero 比手写 Rule of Five 更适合大多数业务类；
- 使用 ASan 验证最终实现没有内存错误。

建议用时：约 **3～4小时**。

```text
35分钟：区分拷贝构造与拷贝赋值
40分钟：复现默认浅拷贝错误
60分钟：实现深拷贝和自赋值保护
45分钟：理解移动构造与移动赋值
45～70分钟：完成AudioBuffer练习与ASan验证
```

---

## 2. 今日目录建议

```text
day14/
├── include/
│   └── audio_buffer.hpp
├── src/
│   ├── 01_copy_or_assign.cpp
│   ├── 02_shallow_copy_bug.cpp
│   ├── 03_deep_copy.cpp
│   ├── 04_copy_assignment.cpp
│   ├── 05_self_assignment.cpp
│   ├── 06_move_semantics.cpp
│   └── 07_audio_buffer.cpp
├── tests/
│   └── audio_buffer_tests.cpp
├── target/
│   └── report/
└── 第4周_工作日1_拷贝构造_拷贝赋值与Rule_of_Five.md
```

创建目录：

```bash
mkdir -p include src tests target/report
```

---

## 3. 最关键的区别：创建还是赋值

先看两行非常相似的代码：

```cpp
AudioBuffer first(4);

AudioBuffer second = first;
```

第二行正在创建`second`，因此调用的是：

```cpp
AudioBuffer(const AudioBuffer& other);
```

也就是拷贝构造函数。

再看：

```cpp
AudioBuffer first(4);
AudioBuffer second(2);

second = first;
```

`second`早已存在，现在只是让它接收`first`的内容，因此调用：

```cpp
AudioBuffer& operator=(const AudioBuffer& other);
```

也就是拷贝赋值运算符。

记忆方法：

```text
创建新对象 → 构造函数
修改旧对象 → 赋值运算符
```

### 四种常见写法

```cpp
AudioBuffer a(4);       // 普通构造
AudioBuffer b(a);       // 拷贝构造
AudioBuffer c = a;      // 仍然是拷贝构造
c = b;                  // 拷贝赋值
```

注意：

```cpp
AudioBuffer c = a;
```

虽然出现了`=`，但`c`是在这一行才被创建，因此不是赋值，而是拷贝构造。

---

## 4. 拷贝构造函数

标准形式：

```cpp
class AudioBuffer {
public:
    AudioBuffer(
        const AudioBuffer& other
    );
};
```

逐部分解释：

```text
AudioBuffer                 函数名与类名相同，说明它是构造函数
const AudioBuffer& other    接收另一个同类型对象的const引用
```

为什么参数必须是引用？

错误写法：

```cpp
AudioBuffer(AudioBuffer other);
```

为了把实参按值传给`other`，编译器必须先复制实参；复制又需要调用拷贝构造函数；调用拷贝构造函数又需要按值复制参数，于是形成无限递归。因此拷贝构造函数通常接收：

```cpp
const AudioBuffer& other
```

为什么加`const`？

- 拷贝源不应该被修改；
- 可以复制`const`对象；
- 可以接受更多合法实参。

---

## 5. 拷贝赋值运算符

标准形式：

```cpp
class AudioBuffer {
public:
    AudioBuffer& operator=(
        const AudioBuffer& other
    );
};
```

这是你第一次系统接触运算符重载。

```cpp
left = right;
```

对类对象来说，可以理解为：

```cpp
left.operator=(right);
```

为什么返回`AudioBuffer&`？

为了支持：

```cpp
a = b = c;
```

执行顺序近似为：

```cpp
b.operator=(c);
a.operator=(b);
```

因此拷贝赋值最后通常返回当前对象：

```cpp
return *this;
```

其中：

```text
this    指向当前对象的指针
*this   当前对象本身
```

---

## 6. 编译器默认拷贝做了什么

如果你没有编写拷贝构造和拷贝赋值，编译器通常会尝试生成默认版本。默认行为是：

> 逐个成员复制。

例如：

```cpp
class AudioBuffer {
private:
    int size_;
    double* data_;
};
```

默认拷贝大致相当于：

```cpp
new_object.size_ = old_object.size_;
new_object.data_ = old_object.data_;
```

`size_`复制数值没有问题，但`data_`复制的只是地址。

结果是：

```text
first.data_ ──┐
              ├──> 同一块堆内存
second.data_ ─┘
```

两个对象析构时都会执行：

```cpp
delete[] data_;
```

于是同一地址被释放两次，产生重复释放或释放后使用。这就是拥有型裸指针发生默认浅拷贝的典型危险。

---

## 7. 浅拷贝与深拷贝

### 浅拷贝

只复制指针保存的地址：

```cpp
data_ = other.data_;
```

两个对象共享同一块内存，但双方都认为自己负责释放它。

### 深拷贝

为新对象申请一块独立内存，再复制每个元素：

```cpp
data_ = new double[other.size_];

for (int i = 0; i < other.size_; ++i) {
    data_[i] = other.data_[i];
}
```

结果：

```text
first.data_  ───> 第一块堆内存

second.data_ ───> 第二块堆内存
```

两个对象内容相同，但资源相互独立。

---

## 8. 错误示例：默认浅拷贝

文件：`src/02_shallow_copy_bug.cpp`

```cpp
#include <cstddef>
#include <iostream>

class UnsafeBuffer {
public:
    explicit UnsafeBuffer(std::size_t size)
        : size_(size),
          data_(new double[size]{}) {
    }

    ~UnsafeBuffer() {
        delete[] data_;
    }

    void set(
        std::size_t index,
        double value
    ) {
        data_[index] = value;
    }

    double get(std::size_t index) const {
        return data_[index];
    }

private:
    std::size_t size_;
    double* data_;
};

int main() {
    UnsafeBuffer first(3);
    first.set(0, 0.25);

    UnsafeBuffer second = first;
    second.set(0, 0.75);

    std::cout
        << "first[0] = "
        << first.get(0)
        << '\n';

    return 0;
}
```

这里没有自定义拷贝构造，因此：

```cpp
UnsafeBuffer second = first;
```

只复制了`data_`中的地址。你可能观察到：

- 修改`second`也改变了`first`看到的内容；
- 程序结束时 ASan 报告重复释放；
- 即使某次运行“看起来正常”，行为仍然是错误的。

编译并运行：

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
  src/02_shallow_copy_bug.cpp \
  -o target/02_shallow_copy_bug

./target/02_shallow_copy_bug
```

> 这是故意制造错误的实验。出现 ASan 报告正是实验目标，不要把该实现用于后续项目。

---

## 9. 正确实现深拷贝构造

```cpp
AudioBuffer::AudioBuffer(
    const AudioBuffer& other
)
    : size_(other.size_),
      data_(nullptr) {
    if (size_ == 0) {
        return;
    }

    data_ = new double[size_];

    for (std::size_t i = 0; i < size_; ++i) {
        data_[i] = other.data_[i];
    }
}
```

执行过程：

```text
1. 复制元素数量
2. 为新对象申请独立数组
3. 逐个复制元素
4. 新旧对象分别拥有自己的数组
```

为什么先写：

```cpp
data_(nullptr)
```

因为空缓冲区不需要申请资源，同时让指针始终处于明确状态。

---

## 10. 正确实现拷贝赋值

赋值比拷贝构造更复杂，因为左侧对象已经拥有资源。

```cpp
AudioBuffer& AudioBuffer::operator=(
    const AudioBuffer& other
) {
    if (this == &other) {
        return *this;
    }

    double* new_data = nullptr;

    if (other.size_ > 0) {
        new_data = new double[other.size_];

        for (
            std::size_t i = 0;
            i < other.size_;
            ++i
        ) {
            new_data[i] = other.data_[i];
        }
    }

    delete[] data_;

    data_ = new_data;
    size_ = other.size_;

    return *this;
}
```

### 为什么先申请新内存，再释放旧内存

错误顺序：

```cpp
delete[] data_;
data_ = new double[other.size_];
```

如果`new`抛出异常，原对象的数据已经丢失。

较安全的顺序：

```text
先申请并复制新资源
        ↓
成功后释放旧资源
        ↓
提交新状态
```

这样如果申请失败，当前对象仍然保留原来的有效数据。

---

## 11. 自赋值是什么

```cpp
AudioBuffer buffer(4);
buffer = buffer;
```

此时：

```text
this   指向buffer
&other 也指向buffer
```

所以：

```cpp
this == &other
```

结果为`true`。

标准保护写法：

```cpp
if (this == &other) {
    return *this;
}
```

如果先释放当前资源，而`other`恰好就是当前对象，那么随后复制时读取的已经是被释放的内存。

---

## 12. Rule of Three

在移动语义加入C++之前，常用规则是 Rule of Three：

如果类需要手写以下任何一个函数，通常应该同时检查另外两个：

```cpp
~AudioBuffer();

AudioBuffer(
    const AudioBuffer& other
);

AudioBuffer& operator=(
    const AudioBuffer& other
);
```

分别是：

```text
析构函数
拷贝构造函数
拷贝赋值运算符
```

原因是它们共同管理同一种资源所有权。

---

## 13. 为什么需要移动

假设函数创建了一个很大的音频缓冲区：

```cpp
AudioBuffer make_buffer() {
    AudioBuffer result(1'000'000);
    return result;
}
```

如果只能复制，就可能需要重新申请一百万个元素并逐个复制。但函数内部的临时对象马上就要结束生命周期，它的资源完全可以直接交给新对象。

移动的核心思想：

```text
复制：重新申请资源并复制内容
移动：接管原对象的资源
```

---

## 14. 移动构造函数

标准形式：

```cpp
AudioBuffer(
    AudioBuffer&& other
) noexcept;
```

实现：

```cpp
AudioBuffer::AudioBuffer(
    AudioBuffer&& other
) noexcept
    : size_(other.size_),
      data_(other.data_) {
    other.size_ = 0;
    other.data_ = nullptr;
}
```

这里没有复制数组，只复制了：

- 数量；
- 指针地址。

但是与错误的浅拷贝不同，移动之后会把源对象清空：

```cpp
other.size_ = 0;
other.data_ = nullptr;
```

因此只有新对象拥有原来的内存。

移动后的`other`：

- 必须仍然可以安全析构；
- 可以重新赋值；
- 不应该再假设它保留移动前的内容。

---

## 15. 移动赋值运算符

```cpp
AudioBuffer& AudioBuffer::operator=(
    AudioBuffer&& other
) noexcept {
    if (this == &other) {
        return *this;
    }

    delete[] data_;

    data_ = other.data_;
    size_ = other.size_;

    other.data_ = nullptr;
    other.size_ = 0;

    return *this;
}
```

执行步骤：

```text
1. 防止自己移动给自己
2. 释放目标对象原有资源
3. 接管源对象资源
4. 将源对象置为空状态
5. 返回当前对象
```

---

## 16. `std::move`到底做什么

使用：

```cpp
AudioBuffer first(4);
AudioBuffer second(
    std::move(first)
);
```

需要：

```cpp
#include <utility>
```

`std::move(first)`本身并不会搬运内存。它只是把`first`转换成一种允许调用移动操作的表达式。

真正接管资源的是：

```cpp
AudioBuffer(AudioBuffer&& other)
```

记忆：

```text
std::move只是发出“可以移动”的信号
移动构造或移动赋值才真正接管资源
```

调用`std::move(first)`以后，如果资源确实被移动，就不要继续依赖`first`原来的内容。

---

## 17. 为什么移动函数经常写`noexcept`

```cpp
AudioBuffer(
    AudioBuffer&& other
) noexcept;
```

表示这个移动构造承诺不向外抛异常。

当前移动操作只是在：

- 复制数量；
- 复制指针；
- 清空源对象。

它没有申请新内存，因此可以合理地声明为`noexcept`。

以后学习`std::vector`时会看到：某些标准容器在重新分配元素时，只有确认移动构造不会抛异常，才更愿意使用移动而不是复制。

---

## 18. Rule of Five

Rule of Five包含五个特殊成员函数：

```cpp
~AudioBuffer();

AudioBuffer(
    const AudioBuffer& other
);

AudioBuffer& operator=(
    const AudioBuffer& other
);

AudioBuffer(
    AudioBuffer&& other
) noexcept;

AudioBuffer& operator=(
    AudioBuffer&& other
) noexcept;
```

对应：

```text
1. 析构函数
2. 拷贝构造函数
3. 拷贝赋值运算符
4. 移动构造函数
5. 移动赋值运算符
```

这不是说所有类都必须手写五个，而是说：

> 一旦类开始手动管理资源并需要自定义其中某些函数，就必须系统检查复制、移动和销毁是否保持一致。

---

## 19. 完整教学示例：`AudioBuffer`

文件：`include/audio_buffer.hpp`

```cpp
#pragma once

#include <cstddef>

class AudioBuffer {
public:
    explicit AudioBuffer(std::size_t size);

    ~AudioBuffer();

    AudioBuffer(
        const AudioBuffer& other
    );

    AudioBuffer& operator=(
        const AudioBuffer& other
    );

    AudioBuffer(
        AudioBuffer&& other
    ) noexcept;

    AudioBuffer& operator=(
        AudioBuffer&& other
    ) noexcept;

    std::size_t size() const noexcept;

    void set(
        std::size_t index,
        double value
    );

    double get(std::size_t index) const;

    const double* data() const noexcept;

private:
    std::size_t size_;
    double* data_;
};
```

文件：`src/audio_buffer.cpp`

```cpp
#include "audio_buffer.hpp"

#include <stdexcept>

AudioBuffer::AudioBuffer(std::size_t size)
    : size_(size),
      data_(size == 0
                ? nullptr
                : new double[size]{}) {
}

AudioBuffer::~AudioBuffer() {
    delete[] data_;
}

AudioBuffer::AudioBuffer(
    const AudioBuffer& other
)
    : size_(other.size_),
      data_(nullptr) {
    if (size_ == 0) {
        return;
    }

    data_ = new double[size_];

    for (std::size_t i = 0; i < size_; ++i) {
        data_[i] = other.data_[i];
    }
}

AudioBuffer& AudioBuffer::operator=(
    const AudioBuffer& other
) {
    if (this == &other) {
        return *this;
    }

    double* new_data = nullptr;

    if (other.size_ > 0) {
        new_data = new double[other.size_];

        for (
            std::size_t i = 0;
            i < other.size_;
            ++i
        ) {
            new_data[i] = other.data_[i];
        }
    }

    delete[] data_;

    data_ = new_data;
    size_ = other.size_;

    return *this;
}

AudioBuffer::AudioBuffer(
    AudioBuffer&& other
) noexcept
    : size_(other.size_),
      data_(other.data_) {
    other.size_ = 0;
    other.data_ = nullptr;
}

AudioBuffer& AudioBuffer::operator=(
    AudioBuffer&& other
) noexcept {
    if (this == &other) {
        return *this;
    }

    delete[] data_;

    data_ = other.data_;
    size_ = other.size_;

    other.data_ = nullptr;
    other.size_ = 0;

    return *this;
}

std::size_t AudioBuffer::size() const noexcept {
    return size_;
}

void AudioBuffer::set(
    std::size_t index,
    double value
) {
    if (index >= size_) {
        throw std::out_of_range(
            "AudioBuffer索引越界"
        );
    }

    data_[index] = value;
}

double AudioBuffer::get(std::size_t index) const {
    if (index >= size_) {
        throw std::out_of_range(
            "AudioBuffer索引越界"
        );
    }

    return data_[index];
}

const double* AudioBuffer::data() const noexcept {
    return data_;
}
```

文件：`src/07_audio_buffer.cpp`

```cpp
#include "audio_buffer.hpp"

#include <iostream>
#include <utility>

int main() {
    AudioBuffer original(3);
    original.set(0, 0.1);
    original.set(1, 0.2);
    original.set(2, 0.3);

    AudioBuffer copied(original);
    copied.set(0, 9.9);

    std::cout
        << "original[0] = "
        << original.get(0)
        << '\n';

    std::cout
        << "copied[0] = "
        << copied.get(0)
        << '\n';

    std::cout
        << "different storage = "
        << (original.data() != copied.data())
        << '\n';

    AudioBuffer assigned(1);
    assigned = original;

    std::cout
        << "assigned size = "
        << assigned.size()
        << '\n';

    AudioBuffer moved(
        std::move(original)
    );

    std::cout
        << "moved size = "
        << moved.size()
        << '\n';

    std::cout
        << "original size after move = "
        << original.size()
        << '\n';

    return 0;
}
```

预期关键输出：

```text
original[0] = 0.1
copied[0] = 9.9
different storage = 1
assigned size = 3
moved size = 3
original size after move = 0
```

---

## 20. 禁止复制：`= delete`

并非所有资源都应该复制。例如你之前实现的`FileHandle`：两个对象若同时管理同一个`FILE*`，就可能重复关闭文件。

因此可以明确禁止复制：

```cpp
class FileHandle {
public:
    FileHandle(
        const FileHandle&
    ) = delete;

    FileHandle& operator=(
        const FileHandle&
    ) = delete;
};
```

含义：

```text
FileHandle不能拷贝构造
FileHandle不能拷贝赋值
```

尝试复制会产生编译期错误。这不是运行时崩溃，而是编译器直接阻止危险操作。

什么时候选择深拷贝，什么时候禁止复制？

```text
资源可以合理复制，而且每份对象应独立拥有副本 → 深拷贝
资源不能或不应该复制                         → = delete
资源只需转移所有权                           → 允许移动
```

---

## 21. Rule of Zero：工程中更推荐的方向

如果类的成员自己就能正确管理资源，类通常不需要手写析构、拷贝和移动函数。

例如下面只是预览，`std::vector`会在后续课程正式学习：

```cpp
#include <vector>

class AudioBuffer {
public:
    explicit AudioBuffer(std::size_t size)
        : samples_(size, 0.0) {
    }

private:
    std::vector<double> samples_;
};
```

`std::vector`已经正确实现：

- 析构；
- 深拷贝；
- 拷贝赋值；
- 移动构造；
- 移动赋值。

因此外层类可以让编译器自动生成相应操作。这就是 Rule of Zero：

> 业务类尽量不直接管理裸资源，把所有权交给标准资源管理类型。

今天手写`AudioBuffer`是学习底层规则；以后写项目时优先考虑 Rule of Zero。

---

## 22. 常见错误清单

### 错误1：拷贝构造参数按值传递

```cpp
AudioBuffer(AudioBuffer other);
```

问题：为了传参需要再次拷贝，形成递归需求。

正确：

```cpp
AudioBuffer(
    const AudioBuffer& other
);
```

### 错误2：拷贝赋值忘记返回

```cpp
AudioBuffer& operator=(
    const AudioBuffer& other
) {
    // 复制资源
}
```

正确结尾：

```cpp
return *this;
```

### 错误3：把拷贝赋值误认为构造函数

```cpp
AudioBuffer second(3);
second = first;
```

`second`已经存在，因此这是拷贝赋值。

### 错误4：只复制裸指针

```cpp
data_ = other.data_;
```

如果双方都负责释放，这会造成重复释放。

### 错误5：忘记自赋值

```cpp
buffer = buffer;
```

需要保证操作后对象仍然有效。

### 错误6：移动后不清空源对象

```cpp
data_ = other.data_;
```

如果不再写：

```cpp
other.data_ = nullptr;
```

两个析构函数仍可能释放同一地址。

### 错误7：认为`std::move`自动移动资源

`std::move`只改变表达式类别，真正移动由移动构造或移动赋值完成。

### 错误8：对所有类机械手写五个函数

如果成员已经能够安全管理资源，优先使用 Rule of Zero。

---

## 23. 练习1：判断调用了哪个函数

不要先运行。写出每一行调用的函数类型。

```cpp
AudioBuffer a(4);
AudioBuffer b(a);
AudioBuffer c = a;
AudioBuffer d(2);
d = a;
AudioBuffer e(std::move(a));
d = std::move(b);
```

需要回答：

1. 哪些是普通构造？
2. 哪些是拷贝构造？
3. 哪些是拷贝赋值？
4. 哪些是移动构造？
5. 哪些是移动赋值？
6. 哪些对象在移动后不能再假设保留原内容？

---

## 24. 练习2：观察编译器默认拷贝

实现只包含以下成员的类：

```cpp
class ModelConfig {
private:
    std::string model_name_;
    int device_id_;
};
```

要求：

- 不手写析构函数；
- 不手写任何拷贝或移动函数；
- 创建一个对象并复制；
- 修改副本；
- 验证原对象保持不变；
- 思考为什么这个类的默认拷贝是安全的。

提示：`std::string`自己负责内部资源，因此这个类适合 Rule of Zero。

---

## 25. 练习3：用ASan观察浅拷贝错误

运行第8节的`UnsafeBuffer`。

需要记录：

- 两个对象中的`data_`是否保存相同地址；
- 修改副本后原对象读到的数值；
- ASan错误类型；
- 第一次释放的位置；
- 第二次释放或错误访问的位置；
- 为什么程序输出正确也不能说明内存安全。

报告文件建议保存为：

```text
target/report/03_shallow_copy_asan.txt
```

运行并保存：

```bash
./target/02_shallow_copy_bug \
  > target/report/03_shallow_copy_asan.txt \
  2>&1
```

---

## 26. 练习4：只实现深拷贝构造

为`AudioBuffer`实现：

```cpp
AudioBuffer(
    const AudioBuffer& other
);
```

验收要求：

- 副本尺寸与原对象一致；
- 每个元素内容一致；
- `original.data() != copied.data()`；
- 修改副本不会改变原对象；
- ASan无错误。

暂时不要实现拷贝赋值，以便集中观察拷贝构造。

---

## 27. 练习5：实现拷贝赋值

实现：

```cpp
AudioBuffer& operator=(
    const AudioBuffer& other
);
```

测试以下场景：

```cpp
AudioBuffer small(2);
AudioBuffer large(5);

small = large;
large = small;
```

验收要求：

- 不同大小之间能够赋值；
- 旧资源被正确释放；
- 新资源与源对象独立；
- 返回`*this`；
- ASan无错误。

---

## 28. 练习6：验证自赋值

```cpp
AudioBuffer buffer(3);
buffer.set(0, 1.5);

buffer = buffer;
```

验收要求：

- 不崩溃；
- `buffer.size()`不变；
- `buffer.get(0)`仍为`1.5`；
- ASan无释放后使用；
- 能解释`this == &other`两边分别是什么。

---

## 29. 练习7：实现移动构造

实现：

```cpp
AudioBuffer(
    AudioBuffer&& other
) noexcept;
```

测试：

```cpp
AudioBuffer source(4);
const double* old_address = source.data();

AudioBuffer target(
    std::move(source)
);
```

验收要求：

- `target.data() == old_address`；
- `target.size() == 4`；
- `source.data() == nullptr`；
- `source.size() == 0`；
- 程序结束时资源只释放一次。

---

## 30. 练习8：实现移动赋值

```cpp
AudioBuffer source(5);
AudioBuffer target(2);

target = std::move(source);
```

验收要求：

- `target`原来的两元素资源被释放；
- `target`接管`source`的五元素资源；
- `source`变为空状态；
- 没有内存泄漏；
- 没有重复释放。

---

## 31. 练习9：禁止复制的资源类

为之前的`FileHandle`补充：

```cpp
FileHandle(
    const FileHandle&
) = delete;

FileHandle& operator=(
    const FileHandle&
) = delete;
```

故意尝试：

```cpp
FileHandle first(
    "data.txt",
    "r"
);

FileHandle second = first;
```

记录编译器错误，并回答：

1. 这是编译期错误还是运行时错误？
2. 为什么文件句柄不应该默认浅拷贝？
3. 深拷贝一个文件句柄具体意味着什么，语义是否明确？
4. 为什么禁止复制可能比勉强实现深拷贝更合理？

---

## 32. 练习10：AudioBuffer综合测试

文件：`tests/audio_buffer_tests.cpp`

至少完成以下测试：

```text
1. 普通构造后尺寸正确
2. set和get正常
3. 越界访问抛出异常
4. 拷贝构造后内容相同
5. 拷贝构造后存储地址不同
6. 修改副本不影响原对象
7. 拷贝赋值支持不同尺寸
8. 自赋值后数据保持不变
9. 移动构造接管原地址
10. 移动构造后源对象为空
11. 移动赋值接管原地址
12. 整个测试程序ASan零错误
```

今天可以继续使用昨天的`TestRunner`。浮点数示例只使用能够精确表示或直接保存读取的简单值，例如：

```cpp
0.5
1.0
1.5
```

暂时不扩展浮点近似比较框架。

---

## 33. 编译命令

综合示例：

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
  src/audio_buffer.cpp \
  src/07_audio_buffer.cpp \
  -I include \
  -o target/07_audio_buffer
```

运行：

```bash
./target/07_audio_buffer
```

查看退出码：

```bash
echo $?
```

测试程序示例：

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
  src/audio_buffer.cpp \
  src/testRunner.cpp \
  tests/audio_buffer_tests.cpp \
  -I include \
  -o target/audio_buffer_tests
```

如果你的`TestRunner`源文件在其他目录，请按实际路径修改，不要机械复制路径。

---

## 34. ASan最终验证

最终正确版本需要满足：

```text
没有heap-use-after-free
没有attempting double-free
没有heap-buffer-overflow
没有内存泄漏报告
进程退出码为0
```

保存结果：

```bash
./target/audio_buffer_tests \
  > target/report/audio_buffer_tests.txt \
  2>&1

echo $?
```

“ASan零错误”指的是：

- 程序使用`-fsanitize=address`重新编译；
- 执行覆盖复制、赋值、移动和销毁路径；
- 运行期间没有出现ASan错误报告；
- 测试断言全部通过。

它不代表程序绝对没有任何逻辑错误，只表示本次执行覆盖到的路径没有被ASan检测出相关内存错误。

---

## 35. 今日必须回答的问题

1. `AudioBuffer b = a;`为什么是拷贝构造而不是拷贝赋值？
2. 拷贝构造函数为什么不能接收同类型的值参数？
3. 为什么拷贝源通常使用`const`引用？
4. 默认成员复制对普通整数为什么安全，对拥有型裸指针为什么危险？
5. 浅拷贝与深拷贝的区别是什么？
6. 拷贝赋值为什么必须处理目标对象原有资源？
7. `this`保存的是什么？
8. `*this`表示什么？
9. `this == &other`在判断什么？
10. 拷贝赋值为什么返回`T&`？
11. 为什么建议先成功申请新资源，再释放旧资源？
12. Rule of Three包含哪三个函数？
13. Rule of Five比Rule of Three增加了哪两个函数？
14. 移动构造与浅拷贝的关键区别是什么？
15. `std::move`是否亲自搬运资源？
16. 移动后的源对象需要满足什么最低要求？
17. 为什么移动构造常写`noexcept`？
18. `= delete`适用于什么场景？
19. Rule of Zero表达了什么工程思想？
20. 为什么今天仍然要手写一个生产中不推荐的裸指针资源类？

---

## 36. 今日验收清单

### 概念

- [ ] 能区分普通构造、拷贝构造和拷贝赋值；
- [ ] 能解释默认浅拷贝的危险；
- [ ] 能解释深拷贝；
- [ ] 能说出Rule of Three；
- [ ] 能说出Rule of Five；
- [ ] 能解释Rule of Zero；
- [ ] 能解释移动与复制的区别；
- [ ] 能解释`std::move`的真实作用。

### 编码

- [ ] 完成`AudioBuffer`析构函数；
- [ ] 完成深拷贝构造；
- [ ] 完成拷贝赋值；
- [ ] 正确处理自赋值；
- [ ] 完成移动构造；
- [ ] 完成移动赋值；
- [ ] 移动后将源对象置为安全状态；
- [ ] 为不可复制资源使用`= delete`。

### 测试

- [ ] 观察一次故意制造的浅拷贝ASan错误；
- [ ] 验证副本与原对象地址不同；
- [ ] 验证修改副本不影响原对象；
- [ ] 验证自赋值安全；
- [ ] 验证移动操作接管原地址；
- [ ] 验证移动后的源对象可以安全析构；
- [ ] 最终测试全部通过；
- [ ] 最终ASan零错误；
- [ ] 保存测试报告。

---

## 37. 与AI部署方向的联系

这些内容不是纯语法练习。在AI部署工程中经常会遇到：

```text
音频缓冲区
张量内存
模型运行时句柄
CUDA资源
推理请求对象
网络连接
文件和模型映射
```

你必须明确：

```text
资源能否复制？
复制后是否应该独立？
资源是否只能转移？
谁负责最终释放？
异常发生时资源是否仍安全？
```

例如：

- 模型配置通常可以安全复制；
- 音频样本容器通常可以深拷贝或移动；
- 文件句柄和某些运行时句柄通常不应随意复制；
- 大型张量更倾向移动或共享明确的所有权，而不是无意义地完整复制。

今天的项目产出不是一个独立求职项目，但它会成为后续C++推理服务、音频流水线和资源管理代码的基础。

---

## 38. 今日Git提交建议

检查状态：

```bash
git status
```

添加本日代码：

```bash
git add day14
```

提交：

```bash
git commit -m "Learn copy and move semantics with AudioBuffer"
```

建议在README或学习笔记中记录：

```text
1. 拷贝构造与拷贝赋值的区别
2. 浅拷贝导致重复释放的ASan证据
3. 深拷贝如何建立独立所有权
4. 移动操作如何转移所有权
5. Rule of Five与Rule of Zero的适用场景
```

---

## 39. 今日一句话总结

```text
拷贝构造负责用旧对象创建新对象，拷贝赋值负责更新已有对象；拥有资源的类必须明确复制、移动和销毁规则，而工程代码应尽量通过标准资源管理类型实现Rule of Zero。
```
