# 第 4 周·工作日 2：掌握右值引用、移动构造和 `std::move` 的使用边界

## 0. 前置知识与超纲说明

### 今天必须掌握

- 左值和右值的直观区别；
- 左值引用`T&`与右值引用`T&&`的基本含义；
- 右值引用为什么可以绑定临时对象；
- `std::move`本身不搬运资源；
- `std::move`只是允许对象参与移动重载选择；
- 移动构造与移动赋值分别在什么情况下调用；
- 具名右值引用在表达式中仍然是左值；
- 移动后对象仍然有效，但值通常不再确定；
- `const`对象通常无法真正转移资源；
- 类没有移动操作时，`std::move`可能退回复制；
- 返回局部对象时通常不要写`std::move`；
- 只有确定不再需要原值时，才对具名对象使用`std::move`；
- 用`noexcept`表达移动操作不抛异常。

### 今天会复习

- 拷贝构造和拷贝赋值；
- Rule of Five；
- `this`与自赋值判断；
- 裸指针资源所有权；
- `new[]`与`delete[]`；
- 构造函数、析构函数和成员初始化列表；
- `const`引用；
- ASan和测试断言。

### 今天第一次系统理解

- 表达式具有值类别，而变量本身有类型；
- 一个变量的声明类型可以是`T&&`，但只要它有名字，表达式就是左值；
- 重载选择如何区分`const T&`和`T&&`；
- moved-from（被移动后）状态；
- 复制省略与返回值优化的基本直觉；
- `std::move`使用边界。

### 今天只需了解，后续再深入

- lvalue、xvalue、prvalue的完整标准分类；
- 引用折叠；
- 转发引用；
- `std::forward`与完美转发；
- 移动构造的隐式生成和隐式删除规则；
- `std::move_if_noexcept`；
- 标准容器重新分配时的移动策略；
- 复制省略的完整标准规则；
- API所有权设计。

### 今天明确不做

- 不实现模板版转发函数；
- 不使用`std::forward`；
- 不背完整值类别标准定义；
- 不把所有对象都机械地包上`std::move`；
- 不认为移动永远比复制快；
- 不依赖被移动对象保存原值；
- 不用`std::move`替代正常的返回值优化。

> 今天的重点不是“到处使用`std::move`”，而是判断什么时候可以明确放弃一个对象的旧值，并验证最终到底调用了复制还是移动。

---

## 1. 今日目标

完成后你应该能够：

- 根据表达式判断它通常按左值还是右值参与重载；
- 写出接收左值引用和右值引用的重载；
- 解释为什么具名`T&&`仍需再次使用`std::move`；
- 为资源类正确实现移动构造和移动赋值；
- 证明移动操作接管的是原资源地址；
- 解释`std::move(const_object)`为什么经常仍然复制；
- 识别没有移动构造时的复制回退；
- 避免`return std::move(local)`；
- 避免在对象最后一次使用之前过早移动；
- 为移动后对象设定并测试安全状态；
- 使用编译日志、计数器和地址验证真实行为；
- 使用ASan验证移动路径没有泄漏和重复释放。

建议用时：约 **3～4小时**。

```text
35分钟：左值、右值与引用绑定
35分钟：std::move与重载选择
45分钟：移动构造和移动赋值
35分钟：const、复制回退和具名右值引用
35分钟：返回值优化与错误边界
50～80分钟：TraceBuffer实验、测试和ASan验证
```

---

## 2. 今日目录

已经建立：

```text
day15/
├── include/
│   ├── trace_buffer.hpp
│   └── test_runner.hpp
├── src/
│   ├── 01_value_categories.cpp
│   ├── 02_reference_overload.cpp
│   ├── 03_named_rvalue_reference.cpp
│   ├── 04_move_constructor.cpp
│   ├── 05_move_assignment.cpp
│   ├── 06_const_move.cpp
│   ├── 07_move_fallback.cpp
│   ├── 08_return_boundary.cpp
│   ├── trace_buffer.cpp
│   └── test_runner.cpp
├── tests/
│   └── trace_buffer_tests.cpp
├── target/
│   └── report/
└── 第4周_工作日2_右值引用_移动构造与std_move使用边界.md
```

源码文件由你按照练习顺序创建，避免直接复制最终答案而跳过实验。

---

## 3. 先区分对象、变量和表达式

看下面的变量：

```cpp
AudioBuffer first(4);
```

`first`：

- 类型是`AudioBuffer`；
- 有名字；
- 可以在后续代码中再次定位到它；
- 表达式`first`是左值。

再看：

```cpp
AudioBuffer(4)
```

这是一个没有名字的临时对象。它通常很快结束生命周期，因此可以把内部资源交给其他对象。

当前阶段可以用下面的直觉：

```text
有名字、可反复定位的对象表达式 → 通常是左值
临时结果、即将消失的对象表达式 → 通常是右值
```

这只是入门直觉，不是完整标准定义。

---

## 4. 左值引用`T&`

```cpp
AudioBuffer first(4);
AudioBuffer& reference = first;
```

`reference`引用已经存在的`first`。

普通非常量左值引用通常不能绑定临时对象：

```cpp
AudioBuffer& reference = AudioBuffer(4);
```

这通常会编译失败，因为临时对象很快消失，而非常量左值引用通常用于修改一个持续存在的对象。

`const`左值引用可以绑定左值，也可以绑定临时对象：

```cpp
const AudioBuffer& first_ref = first;
const AudioBuffer& temporary_ref = AudioBuffer(4);
```

这也是为什么拷贝构造通常接收：

```cpp
const AudioBuffer& other
```

---

## 5. 右值引用`T&&`

右值引用使用两个`&`：

```cpp
AudioBuffer&& reference = AudioBuffer(4);
```

这里的`reference`绑定一个临时`AudioBuffer`。

右值引用表达的核心意图是：

> 这个对象通常即将结束原有用途，调用者允许我们考虑接管它的资源。

右值引用不代表自动移动。例如：

```cpp
void consume(AudioBuffer&& buffer) {
    // 仅仅接收到了一个右值引用
    // 目前还没有执行资源转移
}
```

资源是否转移，取决于函数内部是否调用了移动构造、移动赋值或其他转移操作。

---

## 6. 用重载观察左值和右值

文件：`src/02_reference_overload.cpp`

```cpp
#include <iostream>
#include <utility>

class Token {
};

void inspect(const Token&) {
    std::cout << "const lvalue reference\n";
}

void inspect(Token&&) {
    std::cout << "rvalue reference\n";
}

int main() {
    Token token;

    inspect(token);
    inspect(Token{});
    inspect(std::move(token));

    return 0;
}
```

预期：

```text
const lvalue reference
rvalue reference
rvalue reference
```

解释：

```text
token              有名字的左值
Token{}            临时对象
std::move(token)   把token转换成允许匹配右值引用的表达式
```

这里仍然没有真实资源，所以观察到的只是重载选择。

---

## 7. `std::move`到底是什么

需要包含：

```cpp
#include <utility>
```

使用：

```cpp
AudioBuffer second(
    std::move(first)
);
```

`std::move(first)`不会：

- 申请内存；
- 释放内存；
- 复制元素；
- 修改`first`；
- 自动把指针设为空。

它做的是：

> 把表达式转换为可以优先匹配右值引用重载的形式。

之后，如果类具有：

```cpp
AudioBuffer(
    AudioBuffer&& other
) noexcept;
```

编译器才可以选择这个移动构造函数。真正接管资源的代码位于移动构造函数内部。

记忆：

```text
std::move：给出“允许移动”的信号
移动函数：真正执行资源接管
```

---

## 8. 具名右值引用仍然是左值

这是今天最容易出错的地方。

```cpp
void forward_to_storage(
    AudioBuffer&& buffer
) {
    AudioBuffer stored(buffer);
}
```

参数声明类型是：

```cpp
AudioBuffer&&
```

但函数体中的表达式：

```cpp
buffer
```

有名字，因此它是左值。所以下面通常调用拷贝构造：

```cpp
AudioBuffer stored(buffer);
```

如果这个函数明确要接管参数资源，需要写：

```cpp
AudioBuffer stored(
    std::move(buffer)
);
```

文件：`src/03_named_rvalue_reference.cpp`

```cpp
#include <iostream>
#include <utility>

class Token {
};

void inspect(const Token&) {
    std::cout << "lvalue overload\n";
}

void inspect(Token&&) {
    std::cout << "rvalue overload\n";
}

void receive(Token&& token) {
    inspect(token);
    inspect(std::move(token));
}

int main() {
    receive(Token{});
    return 0;
}
```

预期：

```text
lvalue overload
rvalue overload
```

---

## 9. 移动构造发生在创建新对象时

```cpp
AudioBuffer first(4);

AudioBuffer second(
    std::move(first)
);
```

因为`second`在这一行才被创建，所以调用的是移动构造：

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

执行前：

```text
first.data_ ──> 内存A
first.size_ = 4
```

执行后：

```text
second.data_ ──> 内存A
second.size_ = 4

first.data_ = nullptr
first.size_ = 0
```

没有创建第二份数组。

---

## 10. 移动赋值发生在更新已有对象时

```cpp
AudioBuffer first(4);
AudioBuffer second(5);

second = std::move(first);
```

`second`已经存在，所以调用移动赋值：

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

这里必须先释放`second`原来的资源，再覆盖它的指针。移动赋值不需要`new`，因为它接管`first`已经存在的资源。

---

## 11. 被移动后的对象能做什么

C++通常要求被移动对象仍处于：

```text
有效但状态未特别指定
valid but unspecified state
```

“有效”表示至少可以：

- 安全析构；
- 被重新赋值；
- 调用文档明确允许的操作。

对我们自己实现的`AudioBuffer`，移动后明确设置为：

```cpp
other.data_ = nullptr;
other.size_ = 0;
```

因此可以：

```cpp
AudioBuffer source(4);
AudioBuffer target(std::move(source));

std::cout << source.size(); // 我们的实现明确返回0

source = AudioBuffer(8);    // 可以重新赋值
```

不能继续假设：

```cpp
source.get(0); // 已经没有原来的元素
```

对于标准库类型，不要普遍假设移动后一定为空，除非对应类型的文档给出这种保证。

---

## 12. 为什么`const`对象通常不能真正移动

```cpp
const AudioBuffer source(4);

AudioBuffer target(
    std::move(source)
);
```

`std::move(source)`得到的表达式保留`const`属性，可以理解为：

```cpp
const AudioBuffer&&
```

而移动构造通常接收：

```cpp
AudioBuffer&& other
```

移动构造需要修改源对象：

```cpp
other.data_ = nullptr;
other.size_ = 0;
```

但`const`对象不能被修改，所以普通移动构造无法接收它。若存在拷贝构造：

```cpp
AudioBuffer(
    const AudioBuffer& other
);
```

那么代码通常会退回复制。

结论：

```text
std::move(const对象)通常不能转移其资源
```

不要为了“优化”给本来要移动的局部对象加`const`。

---

## 13. 没有移动函数时会怎样

```cpp
class CopyOnly {
public:
    CopyOnly() = default;

    CopyOnly(const CopyOnly&) {
        std::cout << "copy\n";
    }
};
```

调用：

```cpp
CopyOnly first;
CopyOnly second(std::move(first));
```

类没有：

```cpp
CopyOnly(CopyOnly&&);
```

但`const CopyOnly&`可以绑定这个表达式，因此可能输出：

```text
copy
```

所以：

```text
看到std::move不等于确认发生了移动
```

必须结合类的可用构造函数和运行日志判断。

---

## 14. 临时对象不一定需要手写`std::move`

```cpp
AudioBuffer target(
    AudioBuffer(4)
);
```

`AudioBuffer(4)`本身就是临时对象，可以直接参与移动构造，或者被编译器直接在`target`的位置构造。

下面通常是多余的：

```cpp
AudioBuffer target(
    std::move(AudioBuffer(4))
);
```

不要把`std::move`理解成所有临时对象的必需标记。

---

## 15. 返回局部对象时通常不要写`std::move`

正确：

```cpp
AudioBuffer make_buffer() {
    AudioBuffer result(1024);
    return result;
}
```

编译器通常可以直接把`result`构造在调用者需要的位置，这叫复制省略或返回值优化。

不推荐机械写成：

```cpp
AudioBuffer make_buffer() {
    AudioBuffer result(1024);
    return std::move(result);
}
```

显式`std::move`可能妨碍某些复制省略机会。即使不写，编译器在无法省略时也可能对局部返回对象进行隐式移动。

当前规则：

```text
按值返回当前函数的局部对象 → 直接return对象名
```

---

## 16. 不要过早移动

错误思路：

```cpp
AudioBuffer buffer(4);

storage = std::move(buffer);

std::cout << buffer.get(0);
```

`buffer`的资源已经交给`storage`，后面又依赖原内容，逻辑错误。

更合理：

```cpp
AudioBuffer buffer(4);

// 先完成所有对buffer原值的使用
std::cout << buffer.get(0);

// 最后一次使用时再转移
storage = std::move(buffer);
```

判断是否使用`std::move`时先问：

```text
我是否明确不再需要这个对象当前保存的值？
```

如果答案不确定，就不要移动。

---

## 17. 不要把移动误当成复制

复制后通常可以继续使用两份内容：

```cpp
AudioBuffer copied = original;

// original和copied都保留数据
```

移动后资源通常只有一份：

```cpp
AudioBuffer moved = std::move(original);

// moved接管数据
// original不再保留原数据
```

如果业务需要原对象继续保留原内容，就应该复制，而不是为了“性能”强行移动。

---

## 18. 移动并不总是更快

对于拥有大块堆内存的`AudioBuffer`：

```text
复制：申请新数组 + 逐个复制样本
移动：复制指针和长度 + 清空源对象
```

移动通常明显更便宜。

但对普通整数：

```cpp
int a = 10;
int b = std::move(a);
```

没有需要转移的资源，效果基本仍是复制整数值。`std::move`不会让内置类型神奇加速。

对于只包含很少数据的类，移动与复制成本也可能接近。

---

## 19. `noexcept`的使用边界

我们的移动构造：

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

只做：

- 数字赋值；
- 指针赋值；
- 指针置空。

这些操作不会抛异常，因此可以承诺`noexcept`。

但不要机械地给所有移动函数写`noexcept`。如果移动函数内部调用了可能抛异常的操作，而你仍声明`noexcept`，异常逃出时程序会调用`std::terminate`。

当前判断方法：

```text
只接管裸指针等不抛异常资源 → 通常可写noexcept
内部可能申请资源或调用会抛异常的操作 → 不能随意承诺
```

---

## 20. 完整观察类：`TraceBuffer`

这个类用静态计数器记录复制和移动次数，帮助验证真实调用。

文件：`include/trace_buffer.hpp`

```cpp
#pragma once

#include <cstddef>

class TraceBuffer {
public:
    explicit TraceBuffer(std::size_t size);

    ~TraceBuffer();

    TraceBuffer(
        const TraceBuffer& other
    );

    TraceBuffer& operator=(
        const TraceBuffer& other
    );

    TraceBuffer(
        TraceBuffer&& other
    ) noexcept;

    TraceBuffer& operator=(
        TraceBuffer&& other
    ) noexcept;

    std::size_t size() const noexcept;
    const double* data() const noexcept;

    void set(
        std::size_t index,
        double value
    );

    double get(std::size_t index) const;

    static int copy_constructor_count() noexcept;
    static int copy_assignment_count() noexcept;
    static int move_constructor_count() noexcept;
    static int move_assignment_count() noexcept;
    static void reset_counts() noexcept;

private:
    std::size_t size_;
    double* data_;

    inline static int copy_constructor_count_ = 0;
    inline static int copy_assignment_count_ = 0;
    inline static int move_constructor_count_ = 0;
    inline static int move_assignment_count_ = 0;
};
```

文件：`src/trace_buffer.cpp`

```cpp
#include "trace_buffer.hpp"

#include <stdexcept>

TraceBuffer::TraceBuffer(std::size_t size)
    : size_(size),
      data_(size == 0
                ? nullptr
                : new double[size]{}) {
}

TraceBuffer::~TraceBuffer() {
    delete[] data_;
}

TraceBuffer::TraceBuffer(
    const TraceBuffer& other
)
    : size_(other.size_),
      data_(nullptr) {
    ++copy_constructor_count_;

    if (size_ == 0) {
        return;
    }

    data_ = new double[size_];

    for (std::size_t i = 0; i < size_; ++i) {
        data_[i] = other.data_[i];
    }
}

TraceBuffer& TraceBuffer::operator=(
    const TraceBuffer& other
) {
    ++copy_assignment_count_;

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

TraceBuffer::TraceBuffer(
    TraceBuffer&& other
) noexcept
    : size_(other.size_),
      data_(other.data_) {
    ++move_constructor_count_;

    other.size_ = 0;
    other.data_ = nullptr;
}

TraceBuffer& TraceBuffer::operator=(
    TraceBuffer&& other
) noexcept {
    ++move_assignment_count_;

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

std::size_t TraceBuffer::size() const noexcept {
    return size_;
}

const double* TraceBuffer::data() const noexcept {
    return data_;
}

void TraceBuffer::set(
    std::size_t index,
    double value
) {
    if (index >= size_) {
        throw std::out_of_range(
            "TraceBuffer index out of range"
        );
    }

    data_[index] = value;
}

double TraceBuffer::get(std::size_t index) const {
    if (index >= size_) {
        throw std::out_of_range(
            "TraceBuffer index out of range"
        );
    }

    return data_[index];
}

int TraceBuffer::copy_constructor_count() noexcept {
    return copy_constructor_count_;
}

int TraceBuffer::copy_assignment_count() noexcept {
    return copy_assignment_count_;
}

int TraceBuffer::move_constructor_count() noexcept {
    return move_constructor_count_;
}

int TraceBuffer::move_assignment_count() noexcept {
    return move_assignment_count_;
}

void TraceBuffer::reset_counts() noexcept {
    copy_constructor_count_ = 0;
    copy_assignment_count_ = 0;
    move_constructor_count_ = 0;
    move_assignment_count_ = 0;
}
```

---

## 21. 观察复制与移动

文件：`src/04_move_constructor.cpp`

```cpp
#include "trace_buffer.hpp"

#include <iostream>
#include <utility>

int main() {
    TraceBuffer::reset_counts();

    TraceBuffer source(4);
    source.set(0, 1.5);

    const double* old_address =
        source.data();

    TraceBuffer copied(source);

    TraceBuffer moved(
        std::move(source)
    );

    std::cout
        << "copy constructor count: "
        << TraceBuffer::copy_constructor_count()
        << '\n';

    std::cout
        << "move constructor count: "
        << TraceBuffer::move_constructor_count()
        << '\n';

    std::cout
        << "copy uses different storage: "
        << (copied.data() != old_address)
        << '\n';

    std::cout
        << "move keeps old storage address: "
        << (moved.data() == old_address)
        << '\n';

    std::cout
        << "source is empty: "
        << (source.data() == nullptr &&
            source.size() == 0)
        << '\n';

    return 0;
}
```

预期关键结果：

```text
copy constructor count: 1
move constructor count: 1
copy uses different storage: 1
move keeps old storage address: 1
source is empty: 1
```

---

## 22. `std::move`使用边界清单

### 适合使用

#### 1. 明确把具名对象资源交给另一个对象

```cpp
destination = std::move(source);
```

前提：之后不再依赖`source`原值。

#### 2. 把右值引用参数继续交给拥有者

```cpp
void store(AudioBuffer&& buffer) {
    stored_ = std::move(buffer);
}
```

因为具名`buffer`是左值表达式。

#### 3. 将局部资源放入长期存储，而不再需要局部原值

```cpp
buffers.push_back(
    std::move(buffer)
);
```

这是后续学习容器时的典型场景。

### 通常不应使用

#### 1. 返回当前函数局部对象

```cpp
return std::move(result); // 通常不推荐
```

应写：

```cpp
return result;
```

#### 2. 对`const`对象移动

```cpp
const AudioBuffer buffer(4);
AudioBuffer target(std::move(buffer));
```

通常退回复制。

#### 3. 对之后还要使用原值的对象移动

```cpp
send(std::move(buffer));
process(buffer); // 逻辑风险
```

#### 4. 对临时对象机械添加

```cpp
consume(
    std::move(AudioBuffer(4))
);
```

临时对象本身已经可以匹配右值引用。

#### 5. 为内置类型追求性能

```cpp
int b = std::move(a);
```

不会产生有意义的资源转移。

#### 6. 不理解所有权契约时盲目移动函数参数

```cpp
call(std::move(argument));
```

先确认`call`会接管什么，以及调用后是否仍需使用`argument`。

---

## 23. 常见错误

### 错误1：以为`std::move`立即清空对象

```cpp
std::move(buffer);
```

单独写这一行通常什么资源转移都没有发生。必须有接收移动结果的操作。

### 错误2：右值引用参数直接按移动使用

```cpp
void store(AudioBuffer&& buffer) {
    AudioBuffer result(buffer); // 复制
}
```

若明确接管：

```cpp
AudioBuffer result(
    std::move(buffer)
);
```

### 错误3：移动后继续按原状态读取

```cpp
target = std::move(source);
source.get(0);
```

移动后只能依赖类型明确保证的状态。

### 错误4：认为用了`std::move`就一定移动

如果没有可调用的移动函数，可能复制。

### 错误5：移动`const`对象

移动通常需要修改源对象，`const`会阻止这一点。

### 错误6：返回局部对象时强加`std::move`

这可能妨碍复制省略。

### 错误7：移动赋值忘记释放目标旧资源

```cpp
data_ = other.data_;
```

会丢失目标旧地址并造成泄漏。

### 错误8：移动后不清空源所有权

两个对象可能同时释放同一地址。

### 错误9：给可能抛异常的移动操作错误承诺`noexcept`

异常逃出`noexcept`函数会终止程序。

---

## 24. 练习1：标注表达式

对下列每个表达式标注“按当前入门规则属于左值还是右值”：

```cpp
TraceBuffer buffer(4);

buffer
TraceBuffer(4)
std::move(buffer)
```

然后回答：

1. 哪个表达式有名字？
2. 哪个是临时对象？
3. 哪个只是发生了转换，还没有真实移动？

---

## 25. 练习2：重载选择

实现：

```cpp
void inspect(const TraceBuffer&);
void inspect(TraceBuffer&&);
```

分别传入：

```cpp
TraceBuffer buffer(4);

inspect(buffer);
inspect(TraceBuffer(4));
inspect(std::move(buffer));
```

要求：

- 运行前先预测输出；
- 每个重载打印不同文字；
- 解释为什么第三次调用不会由`std::move`本身修改`buffer`。

---

## 26. 练习3：具名右值引用

实现：

```cpp
void receive(TraceBuffer&& buffer);
```

函数内部依次调用：

```cpp
inspect(buffer);
inspect(std::move(buffer));
```

回答：

1. `buffer`声明类型是什么？
2. 表达式`buffer`为什么仍然匹配左值版本？
3. `std::move(buffer)`为什么匹配右值版本？
4. 仅调用`inspect`是否真的接管资源？

---

## 27. 练习4：移动构造地址实验

```cpp
TraceBuffer source(1024);
const double* address_before = source.data();

TraceBuffer target(
    std::move(source)
);
```

验证：

```text
target.data() == address_before
target.size() == 1024
source.data() == nullptr
source.size() == 0
move_constructor_count()增加1
copy_constructor_count()不增加
```

---

## 28. 练习5：移动赋值资源实验

```cpp
TraceBuffer source(1024);
TraceBuffer target(128);

const double* source_address = source.data();

target = std::move(source);
```

验证：

- `target`接管`source_address`；
- `target.size()`变为1024；
- `source`变为空状态；
- 移动赋值计数增加1；
- ASan不报告目标旧资源泄漏。

---

## 29. 练习6：`const`移动实验

```cpp
const TraceBuffer source(4);

TraceBuffer::reset_counts();

TraceBuffer target(
    std::move(source)
);
```

记录：

- 拷贝构造计数；
- 移动构造计数；
- `source.data()`与`target.data()`是否相同；
- 为什么`const`源对象不能被置空。

预期理解：这里通常调用拷贝构造。

---

## 30. 练习7：没有移动函数的回退

创建`CopyOnly`：

```cpp
class CopyOnly {
public:
    CopyOnly() = default;
    CopyOnly(const CopyOnly&);
};
```

不要提供移动构造，然后执行：

```cpp
CopyOnly first;
CopyOnly second(std::move(first));
```

要求打印实际调用，并回答：

```text
为什么代码中出现std::move，实际却执行了copy？
```

---

## 31. 练习8：发现过早移动

分析下面代码：

```cpp
TraceBuffer buffer(4);
buffer.set(0, 2.5);

TraceBuffer stored(
    std::move(buffer)
);

std::cout << buffer.get(0);
```

要求：

- 解释最后一行为什么不可靠；
- 将读取操作移动到正确位置；
- 验证移动后`buffer.size()`；
- 捕获越界异常，证明对象仍然有效但原值已不存在。

---

## 32. 练习9：返回值边界

实现两个函数：

```cpp
TraceBuffer make_normal() {
    TraceBuffer result(32);
    return result;
}

TraceBuffer make_forced_move() {
    TraceBuffer result(32);
    return std::move(result);
}
```

使用编译警告：

```bash
-Wall -Wextra -Wpedantic
```

观察编译器是否对第二种写法给出阻止复制省略的提示。记录构造、复制和移动计数，但注意不同编译器和优化设置可能表现不同。

最终保留第一种写法。

---

## 33. 练习10：移动语义综合测试

文件：`tests/trace_buffer_tests.cpp`

至少包含：

```text
1. 左值构造调用拷贝构造
2. std::move构造调用移动构造
3. 移动构造接管原地址
4. 移动后源对象为空且可析构
5. 左值赋值调用拷贝赋值
6. std::move赋值调用移动赋值
7. 移动赋值释放目标旧资源
8. const对象的std::move退回复制
9. 具名右值引用仍按左值参与重载
10. 移动后对象可以重新赋值
11. 所有计数符合预期
12. ASan零错误
```

测试名称应明确表达条件，例如：

```text
move constructor keeps source storage address
moved-from source becomes empty
const source falls back to copy
named rvalue reference is an lvalue expression
```

---

## 34. 编译命令

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
  src/trace_buffer.cpp \
  src/04_move_constructor.cpp \
  -I include \
  -o target/04_move_constructor
```

运行：

```bash
./target/04_move_constructor
echo $?
```

测试程序：

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
  src/trace_buffer.cpp \
  src/test_runner.cpp \
  tests/trace_buffer_tests.cpp \
  -I include \
  -o target/trace_buffer_tests
```

如果你复用`day13`或`day14`的测试运行器，请按实际路径调整，或者将其复制到`day15`并统一改名为：

```text
test_runner.hpp
test_runner.cpp
```

---

## 35. ASan与报告

```bash
./target/trace_buffer_tests \
  > target/report/trace_buffer_tests.txt \
  2>&1

echo $?
```

验收：

```text
没有double-free
没有heap-use-after-free
没有heap-buffer-overflow
没有资源泄漏报告
所有断言通过
退出码为0
```

ASan无法告诉你“这个位置本来应该复制还是移动”，因此还需要：

- 构造/赋值计数器；
- 指针地址对比；
- 对象状态断言。

---

## 36. 今日必须回答的问题

1. 左值和右值的入门区别是什么？
2. `T&`与`T&&`分别是什么？
3. 临时对象为什么适合转移资源？
4. `std::move`是否亲自搬运资源？
5. 真正移动资源的是哪段代码？
6. 为什么`AudioBuffer&& other`中的`other`表达式仍然是左值？
7. 为什么函数内部可能需要`std::move(other)`？
8. 移动构造与移动赋值如何区分？
9. 移动赋值为什么先释放目标旧资源？
10. 被移动对象是否立即死亡？
11. “有效但状态未指定”是什么意思？
12. 我们的`TraceBuffer`移动后明确处于什么状态？
13. 为什么`std::move(const_object)`通常复制？
14. 为什么没有移动构造时`std::move`可能复制？
15. 临时对象是否必须再套`std::move`？
16. 返回局部对象为什么通常不写`std::move`？
17. 什么叫过早移动？
18. 移动是否永远比复制快？
19. 什么情况下可以合理声明`noexcept`？
20. 使用`std::move`之前最应该问自己的问题是什么？

---

## 37. 今日验收清单

### 概念

- [ ] 能用自己的话区分左值和右值；
- [ ] 知道`std::move`只做表达式转换；
- [ ] 知道具名右值引用是左值表达式；
- [ ] 知道移动后对象仍可析构和重新赋值；
- [ ] 知道`const`可能阻止资源转移；
- [ ] 知道没有移动函数时可能退回复制；
- [ ] 知道局部返回值通常不写`std::move`；
- [ ] 能判断是否过早移动。

### 编码

- [ ] 实现左右值重载观察程序；
- [ ] 实现移动构造；
- [ ] 实现移动赋值；
- [ ] 移动后清空源所有权；
- [ ] 为不抛异常的移动函数添加`noexcept`；
- [ ] 实现复制和移动计数器；
- [ ] 使用地址验证资源接管。

### 测试

- [ ] 拷贝与移动计数符合预期；
- [ ] 移动构造接管原地址；
- [ ] 移动赋值释放旧资源；
- [ ] `const`移动实验符合预期；
- [ ] 具名右值引用实验符合预期；
- [ ] 移动后对象能安全析构；
- [ ] 测试全部通过；
- [ ] ASan零错误；
- [ ] 保存测试报告。

---

## 38. 与AI部署工程的联系

AI部署中常见的大型资源包括：

```text
音频样本缓冲区
张量数据
模型权重缓存
网络请求载荷
推理结果对象
GPU资源句柄
运行时上下文
```

如果一个对象拥有数百MB数据，毫无必要的深拷贝可能造成：

- 延迟升高；
- 内存峰值翻倍；
- 服务吞吐下降；
- GPU/CPU之间额外数据传输。

移动语义可以在所有权明确时减少这些复制，但错误移动也会造成：

- 原对象被提前清空；
- 后续逻辑读取无效状态；
- 所有权不清晰；
- 重复释放或资源泄漏。

因此工程目标不是“尽量多用`std::move`”，而是：

> 明确资源所有权，在最后一次需要原值之后进行转移，并通过接口和测试证明状态变化符合约定。

---

## 39. 今日Git提交建议

```bash
git status
git add day15
git commit -m "Learn rvalue references and std move boundaries"
```

README建议记录：

```text
1. std::move不执行移动，只参与重载选择
2. 具名右值引用是左值表达式
3. const对象通常不能转移资源
4. 没有移动函数时可能退回复制
5. 返回局部对象通常不使用std::move
6. moved-from对象仍然有效但不能依赖原值
```

---

## 40. 今日一句话总结

```text
std::move只是明确放弃旧值并允许选择移动操作的信号；真正的资源转移由移动构造或移动赋值完成，而且只有在不再依赖源对象原值时才应该使用。
```
