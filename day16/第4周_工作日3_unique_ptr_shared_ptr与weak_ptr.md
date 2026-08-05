# 第 4 周·工作日 3：掌握 `unique_ptr`、`shared_ptr` 和 `weak_ptr`

## 0. 前置知识与超纲说明

### 今天必须掌握

- 智能指针管理的核心是**资源所有权**，而不只是“避免写 `delete`”；
- `std::unique_ptr<T>`表示独占所有权；
- `unique_ptr`不能复制，但可以移动；
- `std::make_unique`是创建独占对象的首选方式；
- `get()`只观察地址，不转移所有权；
- `release()`放弃所有权但不释放资源，使用不当会泄漏；
- `reset()`会替换或释放当前管理的对象；
- `std::shared_ptr<T>`表示多个所有者共享对象生命周期；
- `std::make_shared`是创建共享对象的常用方式；
- `use_count()`只适合学习和调试，不适合作为业务逻辑依据；
- 不得用同一个裸指针分别构造多个独立的 `shared_ptr`；
- `std::weak_ptr<T>`只观察共享对象，不增加强引用计数；
- `weak_ptr::lock()`用于安全地尝试获得临时 `shared_ptr`；
- `weak_ptr`可以打破 `shared_ptr`循环引用；
- 智能指针也不能自动修复越界、悬空观察指针和错误的所有权设计。

### 今天会复习

- 栈对象与堆对象；
- RAII；
- 构造函数与析构函数；
- 拷贝与移动语义；
- `std::move`的使用边界；
- 运行时多态和虚析构函数；
- 空指针判断；
- ASan与单元测试。

### 今天第一次系统理解

- 独占所有权、共享所有权和非拥有观察关系；
- 智能指针对象本身与它管理的堆对象是两个不同对象；
- `shared_ptr`的强引用计数与 `weak_ptr`的观察关系；
- 为什么共享所有权并不等于“为了方便到处使用 `shared_ptr`”；
- 为什么接口参数不应该机械地全部写成智能指针；
- 为什么多态对象通常通过基类智能指针保存。

### 今天只需了解，后续再深入

- `unique_ptr<T, Deleter>`自定义删除器；
- `shared_ptr`控制块的具体实现；
- `std::enable_shared_from_this`；
- `shared_ptr`的别名构造函数；
- 原子版共享指针与多线程引用计数细节；
- C API、文件句柄和 CUDA 资源的自定义删除器；
- `std::pmr`和自定义内存分配器。

### 模板超纲提示

你还没有系统学习模板。下面写法中的 `<Model>`：

```cpp
std::unique_ptr<Model>
std::shared_ptr<Model>
std::weak_ptr<Model>
```

表示“这个智能指针模板管理的对象类型是 `Model`”。今天只要求你会使用，不要求实现智能指针模板。模板会在第 6 周系统学习。

### 今天明确不做

- 不自己实现 `unique_ptr`或 `shared_ptr`；
- 不背控制块的底层实现；
- 不用 `shared_ptr`替代所有裸指针和引用；
- 不用 `use_count()`决定并发业务流程；
- 不对栈对象地址建立智能指针所有权；
- 不用多个 `shared_ptr`分别接管同一个裸指针；
- 不把智能指针理解成万能内存安全工具。

> 今天最重要的问题不是“该用哪种智能指针”，而是：这个对象究竟由谁负责销毁？

---

## 1. 今日目标

完成后你应该能够：

- 用一句话说明三种智能指针的所有权语义；
- 默认使用 `unique_ptr`表达独占资源；
- 用移动语义转移 `unique_ptr`所有权；
- 区分 `get()`、`release()`与 `reset()`；
- 在确实需要多个长期所有者时使用 `shared_ptr`；
- 观察 `shared_ptr`复制、移动和销毁对引用计数的影响；
- 发现并解释 `shared_ptr`循环引用；
- 用 `weak_ptr`打破循环引用；
- 使用 `expired()`和 `lock()`安全访问对象；
- 用基类智能指针管理不同推理后端；
- 根据函数的所有权语义选择引用、裸指针、`unique_ptr`或 `shared_ptr`参数；
- 使用析构日志、测试和 ASan 验证对象只销毁一次且没有泄漏。

建议用时：约 **3～4 小时**。

```text
40分钟：所有权模型与unique_ptr
35分钟：unique_ptr移动、get、release、reset
40分钟：shared_ptr与引用计数
35分钟：weak_ptr与循环引用
30分钟：智能指针作为函数参数
60～90分钟：统一模型后端练习、测试和ASan验证
```

---

## 2. 今日目录

已经建立：

```text
day16/
├── include/
├── src/
├── tests/
├── target/
│   └── report/
└── 第4周_工作日3_unique_ptr_shared_ptr与weak_ptr.md
```

建议你按练习逐步创建：

```text
include/
├── model_backend.hpp
├── model_registry.hpp
└── test_runner.hpp

src/
├── 01_unique_ownership.cpp
├── 02_unique_move.cpp
├── 03_unique_operations.cpp
├── 04_shared_count.cpp
├── 05_invalid_shared_construction.cpp
├── 06_weak_observer.cpp
├── 07_shared_cycle.cpp
├── 08_break_cycle.cpp
├── 09_polymorphic_backend.cpp
├── model_backend.cpp
├── model_registry.cpp
└── test_runner.cpp

tests/
└── smart_pointer_tests.cpp
```

源码由你自己创建和完成；文档会提供学习示例、接口要求和验收标准。

---

## 3. 为什么已经有 RAII，还需要智能指针

你之前写过类似资源类：

```cpp
class AudioBuffer {
public:
    explicit AudioBuffer(std::size_t size)
        : size_(size),
          data_(new float[size]{}) {
    }

    ~AudioBuffer() {
        delete[] data_;
    }

private:
    std::size_t size_;
    float* data_;
};
```

这是 RAII：对象析构时释放数组。

但如果你的类只是想拥有一个普通对象：

```cpp
ModelBackend* backend_;
```

你还要自己考虑：

- 构造失败时是否泄漏；
- 析构时是否 `delete`；
- 能否复制；
- 如何移动；
- 谁是这个地址的真正所有者；
- 是否可能重复释放。

标准智能指针已经封装了常见的堆对象所有权管理。优先使用智能指针，可以减少重复编写容易出错的资源管理代码。

注意：

```text
RAII是设计思想
智能指针是用RAII管理动态对象的标准库工具
```

智能指针不是 RAII 的全部。文件、锁、Socket、CUDA 句柄也可以用其他 RAII 类型管理。

---

## 4. 先区分“指针”和“所有权”

下面两个问题不同：

```text
1. 我能否通过这个地址访问对象？
2. 我是否负责最终销毁这个对象？
```

裸指针可以只表示观察：

```cpp
void print_model(const ModelBackend* backend);
```

这里未必表示 `print_model`负责 `delete backend`。

智能指针主要用于明确第二个问题：

```cpp
std::unique_ptr<ModelBackend> backend;
```

表示这个 `unique_ptr`拥有对象，生命周期结束时负责销毁它。

### 三种关系

| 类型 | 所有权含义 | 是否影响对象生命周期 |
|---|---|---|
| `unique_ptr<T>` | 唯一所有者 | 是 |
| `shared_ptr<T>` | 共享所有者之一 | 是 |
| `weak_ptr<T>` | 非拥有观察者 | 否 |

默认选择顺序：

```text
能用普通局部对象 → 不使用动态分配
确实需要动态对象且单一所有者 → unique_ptr
确实需要多个长期所有者 → shared_ptr
只想观察shared_ptr对象 → weak_ptr
```

---

## 5. `unique_ptr`：独占所有权

需要包含：

```cpp
#include <memory>
```

创建：

```cpp
auto backend =
    std::make_unique<ModelBackend>("Qwen-Omni");
```

它表达：

```text
backend是当前唯一所有者
backend生命周期结束时自动delete对象
```

使用对象：

```cpp
backend->infer();
```

也可以解引用：

```cpp
ModelBackend& reference = *backend;
reference.infer();
```

### 为什么首选 `make_unique`

不推荐直接写：

```cpp
std::unique_ptr<ModelBackend> backend(
    new ModelBackend("Qwen-Omni")
);
```

推荐：

```cpp
auto backend =
    std::make_unique<ModelBackend>("Qwen-Omni");
```

优点：

- 不直接暴露 `new`；
- 所有权在创建时立即明确；
- 代码更短；
- 更不容易在复杂表达式中产生资源管理错误。

---

## 6. 为什么 `unique_ptr`不能复制

```cpp
auto first = std::make_unique<int>(42);
auto second = first;
```

这会产生编译期错误。

如果允许复制，就会出现两个“唯一所有者”：

```text
first  ──> int对象
second ──> 同一个int对象
```

那么谁负责销毁？如果两者都销毁就会重复释放。

因此 `unique_ptr`删除了拷贝操作。它的思想和你前面见过的：

```cpp
FileHandle(const FileHandle&) = delete;
FileHandle& operator=(const FileHandle&) = delete;
```

完全一致。

---

## 7. 使用移动转移 `unique_ptr`所有权

```cpp
#include <memory>
#include <utility>

auto first = std::make_unique<int>(42);

auto second = std::move(first);
```

移动后：

```text
first == nullptr
second拥有原来的int对象
```

可以验证：

```cpp
if (first == nullptr) {
    std::cout << "first no longer owns the object\n";
}

std::cout << *second << '\n';
```

这里复用了昨天的知识：

```text
std::move本身只允许选择移动操作
unique_ptr的移动构造真正转移内部地址
```

---

## 8. `unique_ptr`常用操作

### 8.1 空指针判断

```cpp
if (backend) {
    backend->infer();
}
```

也可以写：

```cpp
if (backend != nullptr) {
    backend->infer();
}
```

### 8.2 `get()`：只借出地址

```cpp
ModelBackend* observed = backend.get();
```

`get()`不会：

- 转移所有权；
- 释放对象；
- 让 `backend`变空。

此时：

```text
backend仍然拥有对象
observed只是暂时观察同一对象
```

绝对不要这样做：

```cpp
delete observed;
```

否则 `backend`以后还会再次释放同一个对象。

### 8.3 `reset()`：释放或替换对象

释放当前对象：

```cpp
backend.reset();
```

执行后：

```text
原对象立即被销毁
backend变为nullptr
```

替换对象通常更清晰地写成：

```cpp
backend =
    std::make_unique<ModelBackend>("Whisper");
```

### 8.4 `release()`：放弃所有权但不释放

```cpp
ModelBackend* raw = backend.release();
```

执行后：

```text
backend变为nullptr
对象仍然存在
raw必须由其他明确的所有者接管或手动delete
```

如果忘记处理 `raw`：

```cpp
backend.release();
```

就会泄漏。

入门阶段的规则：

> 除非正在把资源交给一个明确要求裸指针所有权的旧式 API，否则不要使用 `release()`。

### 三者对比

| 操作 | 返回裸指针 | 原智能指针变空 | 是否释放对象 |
|---|---:|---:|---:|
| `get()` | 是 | 否 | 否 |
| `release()` | 是 | 是 | 否 |
| `reset()` | 否 | 是 | 是 |

---

## 9. `unique_ptr`数组

可以写：

```cpp
auto samples = std::make_unique<float[]>(1024);
samples[0] = 0.5F;
```

它会自动使用正确的数组释放方式。

但是如果你只需要动态数组，通常优先：

```cpp
std::vector<float> samples(1024);
```

因为 `vector`还保存长度，并提供更完整的容器操作。

当前选择：

```text
动态连续元素集合 → 优先vector
独占的单个多态对象 → unique_ptr很常见
```

---

## 10. `shared_ptr`：共享所有权

创建：

```cpp
auto first =
    std::make_shared<ModelBackend>("Qwen-Omni");
```

复制：

```cpp
auto second = first;
```

现在：

```text
first和second共同拥有同一个对象
只要至少一个shared_ptr仍然拥有它，对象就继续存在
最后一个所有者销毁或reset时，对象才被销毁
```

观察计数：

```cpp
std::cout << first.use_count() << '\n';
```

学习示例：

```cpp
auto first = std::make_shared<int>(42);
std::cout << first.use_count() << '\n'; // 1

{
    auto second = first;
    std::cout << first.use_count() << '\n'; // 2
}

std::cout << first.use_count() << '\n'; // 1
```

### 何时才需要共享所有权

合理例子：

- 多个异步任务都需要保证同一模型会话仍然存在；
- 缓存和正在执行的请求共同持有一个模型实例；
- 多个服务组件生命周期彼此独立，但都需要同一只读资源。

不合理理由：

```text
“不知道谁拥有，所以全部shared_ptr”
```

这会掩盖架构问题，并增加引用计数和生命周期分析成本。

---

## 11. `shared_ptr`复制和移动的区别

复制：

```cpp
auto second = first;
```

结果：

```text
first仍然拥有对象
second也拥有对象
强引用计数增加
```

移动：

```cpp
auto second = std::move(first);
```

结果：

```text
first通常变为空
second接管first的那一份所有权
强引用计数通常不会因为这次转移而增加
```

注意：`shared_ptr`可以移动，但它表达的整体模型仍是“允许多个所有者”。

---

## 12. `make_shared`与控制块

`shared_ptr`除了保存对象地址，还需要管理共享状态，通常包括：

```text
强引用计数
弱引用相关状态
删除器等管理信息
```

这些管理信息通常位于“控制块”中。

```cpp
auto model = std::make_shared<ModelBackend>("Qwen-Omni");
```

通常可以一次性安排对象和控制信息，写法也更安全、清晰。

今天只需要形成直觉：

```text
shared_ptr副本必须共享同一个控制块，才能共同管理一次销毁
```

不要求学习控制块的实现代码。

---

## 13. 最危险错误：同一裸指针建立两个控制块

错误示例，只用于阅读，**不要直接运行**：

```cpp
ModelBackend* raw =
    new ModelBackend("Qwen-Omni");

std::shared_ptr<ModelBackend> first(raw);
std::shared_ptr<ModelBackend> second(raw);
```

`first`与 `second`不知道彼此存在，各自建立控制关系：

```text
first控制块  ──> 同一个raw对象
second控制块 ──> 同一个raw对象
```

最终两者都会尝试释放同一对象，导致重复释放。

正确：

```cpp
auto first =
    std::make_shared<ModelBackend>("Qwen-Omni");

auto second = first;
```

这里二者共享同一个控制块。

---

## 14. `use_count()`为什么不能指导业务逻辑

在单线程学习实验中可以打印：

```cpp
std::cout << model.use_count() << '\n';
```

但不要写出依赖瞬时计数的业务判断：

```cpp
if (model.use_count() == 1) {
    // 假设自己是唯一所有者并修改共享状态
}
```

在并发程序中，其他线程可能马上复制或释放所有者；即使在单线程中，这也容易让程序逻辑与隐蔽的临时副本绑定。

规则：

```text
use_count用于学习、诊断和测试观察
不要把它作为核心业务协议
```

---

## 15. `weak_ptr`：不拥有对象的安全观察者

`weak_ptr`只能从共享所有权关系创建：

```cpp
auto shared = std::make_shared<ModelBackend>(
    "Qwen-Omni"
);

std::weak_ptr<ModelBackend> weak = shared;
```

此时：

```text
shared拥有对象
weak观察对象
weak不会让对象延长寿命
```

不能直接解引用 `weak_ptr`，因为对象可能已经销毁。

错误思路：

```cpp
weak->infer(); // 不允许
```

正确方式是 `lock()`：

```cpp
if (auto model = weak.lock()) {
    model->infer();
} else {
    std::cout << "model expired\n";
}
```

`lock()`：

- 对象仍存在时，返回一个临时的非空 `shared_ptr`；
- 对象已销毁时，返回空 `shared_ptr`；
- 获得的临时 `shared_ptr`在作用域内保证对象不被销毁。

---

## 16. `expired()`与 `lock()`

可以查询：

```cpp
if (weak.expired()) {
    std::cout << "expired\n";
}
```

但真正访问对象时优先直接使用 `lock()`：

```cpp
if (auto model = weak.lock()) {
    model->infer();
}
```

不要写成：

```cpp
if (!weak.expired()) {
    auto model = weak.lock();
    model->infer();
}
```

在并发环境中，“检查”和“获得所有权”之间状态可能变化。直接检查 `lock()`返回值更完整。

今天只需记住：

```text
需要使用weak_ptr指向的对象 → lock一次并检查结果
```

---

## 17. `shared_ptr`循环引用

考虑两个对象互相持有：

```cpp
class Session;

class Model {
public:
    std::shared_ptr<Session> session;
};

class Session {
public:
    std::shared_ptr<Model> model;
};
```

建立关系：

```cpp
auto model = std::make_shared<Model>();
auto session = std::make_shared<Session>();

model->session = session;
session->model = model;
```

外部变量离开作用域后，内部仍然互相拥有：

```text
Model   --shared_ptr--> Session
Session --shared_ptr--> Model
```

两边的强引用计数都不能降到零，所以析构函数不会执行，产生内存泄漏。

智能指针没有失效；错误在于所有权关系被设计成一个闭环。

---

## 18. 用 `weak_ptr`打破循环

如果业务上 `Session`拥有 `Model`，而 `Model`只需要反向观察 `Session`，则把观察边改成 `weak_ptr`：

```cpp
class Session;

class Model {
public:
    std::weak_ptr<Session> session;
};

class Session {
public:
    std::shared_ptr<Model> model;
};
```

关系变成：

```text
Session --shared ownership--> Model
Model   --weak observation--> Session
```

弱观察不会增加强引用计数。外部所有者消失时，对象可以正常销毁。

判断哪一边使用 `weak_ptr`时不要猜，先回答：

```text
谁在业务上负责保证谁活着？
谁只是需要临时找到对方？
```

---

## 19. 多态对象与智能指针

你已经学过统一模型接口：

```cpp
class ModelBackend {
public:
    virtual ~ModelBackend() = default;
    virtual void infer() const = 0;
};

class QwenBackend : public ModelBackend {
public:
    void infer() const override;
};
```

可以用基类独占指针保存子类对象：

```cpp
std::unique_ptr<ModelBackend> backend =
    std::make_unique<QwenBackend>();

backend->infer();
```

这里发生运行时多态，最终调用：

```text
QwenBackend::infer()
```

基类析构函数必须是虚函数：

```cpp
virtual ~ModelBackend() = default;
```

否则通过基类指针销毁子类对象可能产生未定义行为。

这正是智能指针、移动语义和运行时多态在 AI 推理服务中的结合点。

---

## 20. 函数参数应该写什么类型

不要看到智能指针就把所有函数参数改成智能指针。参数类型应该表达函数的所有权行为。

### 只使用对象，不接管所有权

对象必须存在：

```cpp
void run(const ModelBackend& backend);
```

允许没有对象：

```cpp
void try_run(const ModelBackend* backend);
```

这里的裸指针是非拥有观察指针，函数不能 `delete`它。

### 函数接管唯一所有权

```cpp
void install(
    std::unique_ptr<ModelBackend> backend
);
```

调用者必须移动：

```cpp
install(std::move(backend));
```

调用后原 `backend`变空。

### 函数需要保留一份共享所有权

```cpp
void register_model(
    std::shared_ptr<ModelBackend> backend
);
```

如果函数要把它保存到成员或容器中，按值接收可以明确产生一份共享所有权。

### 函数只临时观察已有 `shared_ptr`

通常仍可使用：

```cpp
void print_info(const ModelBackend& backend);
```

不需要仅仅为了调用函数就增加一次共享计数。

---

## 21. 返回智能指针

工厂函数常返回 `unique_ptr`：

```cpp
std::unique_ptr<ModelBackend>
create_backend(const std::string& type) {
    if (type == "qwen") {
        return std::make_unique<QwenBackend>();
    }

    throw std::invalid_argument(
        "unknown backend type"
    );
}
```

使用：

```cpp
auto backend = create_backend("qwen");
backend->infer();
```

不需要写：

```cpp
return std::move(backend);
```

这延续了昨天的规则：返回局部对象通常直接返回对象名；移动型返回值能够被正确转移或直接构造。

---

## 22. 智能指针不能保证什么

智能指针可以帮助保证：

- 所有权结束时自动释放对象；
- `unique_ptr`不会被普通复制；
- 正确构造的 `shared_ptr`只在最后一个所有者离开时释放对象；
- 异常路径中局部智能指针仍会析构。

智能指针不能自动保证：

- 数组下标不越界；
- `get()`得到的观察指针始终有效；
- 没有循环引用；
- 多线程访问对象内部数据没有竞争；
- 业务处理一定完成；
- 不会错误地调用 `release()`造成泄漏；
- 所有权关系一定合理。

例如：

```cpp
int* observed = owner.get();
owner.reset();

std::cout << *observed;
```

`observed`已经悬空。智能指针只管理拥有者本身，不能追踪所有借出去的裸观察指针。

---

## 23. `shared_ptr`的线程安全边界（了解）

不同 `shared_ptr`副本对引用计数的增加和减少可以被安全管理，但这不等于它们指向的对象内部自动线程安全。

```cpp
auto shared = std::make_shared<ModelState>();
```

两个线程分别持有 `shared`副本，不会因为普通引用计数销毁而直接冲突；但如果两个线程同时修改：

```cpp
shared->request_count
```

对象内部仍可能产生数据竞争，依然需要互斥锁或原子变量。

当前只记：

```text
shared_ptr保护共享生命周期
不自动保护被指向对象的业务数据
```

多线程细节会在并发阶段系统学习。

---

## 24. 完整学习示例：模型对象生命周期

文件：`src/01_unique_ownership.cpp`

```cpp
#include <iostream>
#include <memory>
#include <string>
#include <utility>

class Model {
public:
    explicit Model(const std::string& name)
        : name_(name) {
        std::cout << "Construct: " << name_ << '\n';
    }

    ~Model() {
        std::cout << "Destroy: " << name_ << '\n';
    }

    void infer() const {
        std::cout << "Infer: " << name_ << '\n';
    }

private:
    std::string name_;
};

int main() {
    auto first =
        std::make_unique<Model>("Qwen-Omni");

    first->infer();

    auto second = std::move(first);

    std::cout
        << "first is empty: "
        << (first == nullptr)
        << '\n';

    second->infer();

    return 0;
}
```

预期：

```text
Construct: Qwen-Omni
Infer: Qwen-Omni
first is empty: 1
Infer: Qwen-Omni
Destroy: Qwen-Omni
```

重点观察：

- 对象只构造一次；
- 所有权从 `first`转移给 `second`；
- 对象只析构一次；
- 没有手写 `delete`。

---

## 25. 完整学习示例：`weak_ptr`失效

文件：`src/06_weak_observer.cpp`

```cpp
#include <iostream>
#include <memory>

class Model {
public:
    ~Model() {
        std::cout << "Destroy Model\n";
    }

    void infer() const {
        std::cout << "Infer\n";
    }
};

int main() {
    std::weak_ptr<Model> observer;

    {
        auto owner = std::make_shared<Model>();
        observer = owner;

        if (auto model = observer.lock()) {
            model->infer();
        }
    }

    if (auto model = observer.lock()) {
        model->infer();
    } else {
        std::cout << "Model expired\n";
    }

    return 0;
}
```

预期：

```text
Infer
Destroy Model
Model expired
```

说明 `observer`本身仍存在，但它没有阻止模型销毁。

---

## 26. 常见错误

### 错误 1：为了省事全部使用 `shared_ptr`

如果只有一个明确所有者，应优先 `unique_ptr`。共享所有权会让销毁时机更难追踪。

### 错误 2：复制 `unique_ptr`

```cpp
auto second = first;
```

应根据业务选择移动所有权，或者只传引用观察对象。

### 错误 3：移动后继续解引用原 `unique_ptr`

```cpp
auto second = std::move(first);
first->infer();
```

`first`已经为空，解引用会导致未定义行为。

### 错误 4：把 `get()`误认为转移所有权

```cpp
Model* raw = owner.get();
delete raw;
```

这会破坏 `owner`的管理关系。

### 错误 5：调用 `release()`后不处理返回地址

```cpp
owner.release();
```

对象不会被释放，产生泄漏。

### 错误 6：同一裸地址分别构造多个 `shared_ptr`

会建立多个控制块并重复释放。

### 错误 7：认为 `weak_ptr`可以直接访问对象

对象可能已经销毁，必须通过 `lock()`获得临时所有权。

### 错误 8：双方都用 `shared_ptr`互相持有

可能造成循环引用，导致对象永不析构。

### 错误 9：认为 `shared_ptr`让对象内部自动线程安全

它管理生命周期，不管理对象字段的数据竞争。

### 错误 10：对栈对象建立智能指针所有权

错误：

```cpp
Model model;
std::unique_ptr<Model> owner(&model);
```

作用域结束时智能指针会尝试 `delete`一个栈对象，导致未定义行为。

---

## 27. 练习 1：`unique_ptr`基本生命周期

创建带构造和析构日志的 `Model`，完成：

```cpp
auto model = std::make_unique<Model>("Qwen-Omni");
model->infer();
```

要求：

- 不写 `new`和 `delete`；
- 记录构造与析构顺序；
- 证明析构只发生一次；
- 用自己的话说明智能指针对象何时销毁、被管理对象何时销毁。

---

## 28. 练习 2：转移独占所有权

```cpp
auto first = std::make_unique<Model>("Whisper");
auto second = std::move(first);
```

验证：

```text
first == nullptr
second != nullptr
second仍然可以调用infer
对象最终只析构一次
```

回答：

1. `std::move`是否直接销毁 `first`？
2. 真正转移内部地址的是谁的移动构造？
3. 移动后为什么必须先判断再使用 `first`？

---

## 29. 练习 3：比较 `get()`、`release()`和 `reset()`

写三个独立小实验，不要混在同一所有者上：

1. `get()`后验证智能指针仍然非空；
2. `reset()`后验证对象立即析构且指针为空；
3. `release()`后把返回地址交给另一个 `unique_ptr`重新接管：

```cpp
Model* raw = first.release();
std::unique_ptr<Model> second(raw);
```

这是为了理解语义，不是推荐的日常写法。

要求解释如果第 3 步只调用 `release()`却不接管，会发生什么。

---

## 30. 练习 4：`shared_ptr`计数实验

按作用域创建副本：

```cpp
auto first = std::make_shared<Model>("SharedModel");

{
    auto second = first;
    auto third = second;
}
```

在每个节点记录 `use_count()`：

- 创建 `first`之后；
- 创建 `second`之后；
- 创建 `third`之后；
- 内部作用域结束后；
- `first.reset()`之前。

要求：只把计数作为观察结果，不写依赖计数的业务分支。

---

## 31. 练习 5：错误共享构造分析

只阅读并解释，不运行：

```cpp
Model* raw = new Model("Broken");

std::shared_ptr<Model> first(raw);
std::shared_ptr<Model> second(raw);
```

回答：

1. 创建了几个堆对象？
2. 创建了几个互不相知的控制块？
3. 每个控制块认为自己负责什么？
4. 为什么最终可能重复释放？
5. 如何用 `make_shared`和复制修正？

---

## 32. 练习 6：`weak_ptr`观察模型

完成以下流程：

```text
创建空weak_ptr
进入局部作用域
创建shared_ptr并赋给weak_ptr
使用lock成功访问模型
离开局部作用域
再次lock并发现对象已经过期
```

要求：

- 打印模型析构时机；
- 不直接解引用 `weak_ptr`；
- 每次只调用一次 `lock()`并检查返回结果；
- 解释为什么 `weak_ptr`仍存在但模型已销毁。

---

## 33. 练习 7：制造并修复循环引用

第一版：

```text
Model使用shared_ptr持有Session
Session使用shared_ptr持有Model
```

运行并观察析构日志。使用 ASan/LeakSanitizer 检查泄漏；不同平台的泄漏检测支持可能不同，因此析构日志也是必要证据。

第二版：

```text
Session使用shared_ptr拥有Model
Model使用weak_ptr观察Session
```

再次运行，验证两个析构函数都执行。

回答：为什么把任意一条真正的“观察边”改成弱引用就可以打破强所有权闭环？

---

## 34. 练习 8：多态推理后端

定义抽象接口：

```cpp
class ModelBackend {
public:
    virtual ~ModelBackend() = default;
    virtual std::string name() const = 0;
    virtual std::string infer(
        const std::string& input
    ) const = 0;
};
```

实现：

```text
QwenBackend
WhisperBackend
```

分别用：

```cpp
std::unique_ptr<ModelBackend>
```

保存两个不同子类对象并调用统一接口。

要求：

- 使用 `make_unique`；
- 不出现裸 `new/delete`；
- 基类具有虚析构函数；
- 调用正确的子类实现；
- 析构日志证明子类和基类都正确析构。

---

## 35. 练习 9：模型注册表

实现简化 `ModelRegistry`：

```cpp
class ModelRegistry {
public:
    void install(
        std::unique_ptr<ModelBackend> backend
    );

    const ModelBackend* current() const noexcept;

private:
    std::unique_ptr<ModelBackend> backend_;
};
```

要求：

- `install`明确接管所有权；
- 调用者使用 `std::move`交出后端；
- 再次安装时旧后端自动析构；
- `current()`只返回非拥有观察指针；
- 注册表为空时返回 `nullptr`；
- 不允许复制 `ModelRegistry`造成含糊所有权；
- 判断能否让编译器自动生成移动操作。

思考：若多个独立请求必须跨越注册表生命周期共同持有模型，这个设计是否仍适合 `unique_ptr`？什么时候才需要调整成 `shared_ptr`？

---

## 36. 练习 10：单元测试

文件：`tests/smart_pointer_tests.cpp`

至少覆盖：

```text
1. make_unique创建对象并在作用域结束时销毁
2. unique_ptr移动后源指针为空
3. unique_ptr移动后目标可正常使用
4. reset立即释放旧对象
5. get不转移所有权
6. shared_ptr复制增加强引用计数
7. shared_ptr副本离开作用域后计数下降
8. 最后一个shared_ptr释放后对象析构
9. weak_ptr不增加强引用计数
10. 对象存在时weak_ptr::lock成功
11. 对象销毁后weak_ptr::lock失败
12. weak_ptr打破循环后析构函数执行
13. 基类unique_ptr正确调用子类实现
14. 替换注册后端时旧对象析构
15. ASan零错误
```

建议给模型类增加仅用于测试的静态生命周期计数：

```cpp
inline static int alive_count_ = 0;
```

构造时增加，析构时减少。测试结束后应为零。

---

## 37. 编译命令

单文件示例：

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
  src/01_unique_ownership.cpp \
  -I include \
  -o target/01_unique_ownership
```

运行：

```bash
./target/01_unique_ownership
echo $?
```

多文件测试示例：

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
  src/model_backend.cpp \
  src/model_registry.cpp \
  src/test_runner.cpp \
  tests/smart_pointer_tests.cpp \
  -I include \
  -o target/smart_pointer_tests
```

注意每一个只声明在头文件、实现于 `.cpp`的成员函数，其 `.cpp`都必须加入链接命令，否则会出现：

```text
Undefined symbols for architecture arm64
```

---

## 38. ASan与泄漏验证

运行并保存报告：

```bash
./target/smart_pointer_tests \
  > target/report/smart_pointer_tests.txt \
  2>&1

echo $?
```

在 macOS 上，AddressSanitizer 对泄漏检测的支持与运行环境有关。可以先尝试：

```bash
ASAN_OPTIONS=detect_leaks=1 \
./target/smart_pointer_tests \
  > target/report/smart_pointer_tests_leaks.txt \
  2>&1
```

如果环境不支持 LeakSanitizer，不代表程序一定无泄漏，因此还要配合：

- 构造/析构日志；
- `alive_count`计数；
- `weak_ptr::expired()`结果；
- 测试退出码。

最终“ASan零错误”表示：

```text
在本次测试覆盖到的执行路径上，ASan没有报告它能够检测的内存错误
```

它不是对所有输入和所有路径的数学证明。

---

## 39. 今日必须回答的问题

1. 智能指针与 RAII 是什么关系？
2. “能访问对象”和“拥有对象”有什么区别？
3. 为什么默认优先局部对象，其次才考虑动态分配？
4. `unique_ptr`表达什么所有权？
5. 为什么 `unique_ptr`不能复制？
6. 如何转移 `unique_ptr`所有权？
7. 移动后原 `unique_ptr`处于什么状态？
8. `make_unique`相比直接写 `new`有什么好处？
9. `get()`是否转移所有权？
10. 为什么不能 `delete owner.get()`？
11. `release()`与 `reset()`有什么根本区别？
12. 为什么动态数组通常优先 `vector`？
13. `shared_ptr`表达什么所有权？
14. `shared_ptr`复制与移动对所有权有什么不同？
15. 最后一个 `shared_ptr`消失时会发生什么？
16. 为什么同一裸指针不能分别构造多个 `shared_ptr`？
17. `use_count()`为什么不适合业务逻辑？
18. `weak_ptr`是否拥有对象？
19. 为什么 `weak_ptr`不能直接解引用？
20. `lock()`成功和失败分别返回什么？
21. 什么是 `shared_ptr`循环引用？
22. `weak_ptr`为什么能打破循环？
23. 基类智能指针管理子类时为什么要求虚析构？
24. 函数只观察对象时为什么不必按值接收 `shared_ptr`？
25. 智能指针能否自动保证线程安全和下标安全？

---

## 40. 今日验收清单

### 概念

- [ ] 能区分独占、共享和观察关系；
- [ ] 知道智能指针对象与被管理对象不同；
- [ ] 知道 `unique_ptr`不能复制但可以移动；
- [ ] 能解释 `get/release/reset`的区别；
- [ ] 知道何时才真正需要共享所有权；
- [ ] 知道 `shared_ptr`的副本必须共享同一控制关系；
- [ ] 能解释循环引用；
- [ ] 能用 `weak_ptr::lock()`安全观察对象；
- [ ] 知道智能指针的安全边界。

### 编码

- [ ] 使用 `make_unique`创建模型对象；
- [ ] 使用移动转移独占所有权；
- [ ] 没有裸 `new/delete`；
- [ ] 完成 `shared_ptr`计数实验；
- [ ] 完成 `weak_ptr`失效实验；
- [ ] 制造并修复一次循环引用；
- [ ] 用基类 `unique_ptr`保存两个后端；
- [ ] 实现接管唯一所有权的模型注册表。

### 测试

- [ ] 所有对象构造和析构次数一致；
- [ ] `unique_ptr`移动后源为空；
- [ ] `shared_ptr`最后一个所有者消失后对象析构；
- [ ] `weak_ptr`不延长对象生命周期；
- [ ] 修复循环引用后析构日志完整；
- [ ] 测试全部通过；
- [ ] ASan无已检测到的错误；
- [ ] 保存报告到 `target/report`。

---

## 41. 与 AI 部署工程的联系

AI 部署系统中会出现不同生命周期：

```text
推理后端实例
模型运行时会话
请求上下文
音频缓冲区
线程任务
缓存条目
网络连接
GPU执行上下文
```

典型所有权设计：

```text
服务唯一拥有一个可替换后端
    → unique_ptr<ModelBackend>

多个异步请求必须共同保证同一模型实例存活
    → shared_ptr<ModelSession>

监控器或缓存索引只想观察模型是否仍存在
    → weak_ptr<ModelSession>

函数只在调用期间使用模型且不保存
    → const ModelBackend&
```

真正有价值的工程能力不是“会写三种指针语法”，而是能把系统中的所有权画清楚：

```text
谁创建资源
谁长期拥有资源
谁只是临时使用
何时释放资源
异常和并发路径下是否仍然成立
```

以后学习 ONNX Runtime、TensorRT 和 CUDA 时，你还会遇到不能直接使用普通 `delete`的 C API 资源。届时会在今天的所有权基础上补充自定义删除器和专用 RAII 封装；今天不用提前实现。

---

## 42. 今日错误案例说明建议

在报告中记录至少四类错误：

```text
案例1：复制unique_ptr导致编译期错误
原因：独占所有权不能产生第二个所有者

案例2：移动后解引用空unique_ptr
原因：所有权已转移，源指针为空

案例3：release后不接管导致泄漏
原因：智能指针放弃所有权，但资源仍存在

案例4：双方shared_ptr互相拥有导致泄漏
原因：强引用闭环使计数无法归零
```

对于故意产生未定义行为的代码：

- 单独放在错误案例文件；
- 不与正常测试一起运行；
- 在文件头写清“仅用于 ASan 实验”；
- 修复后再进行最终零错误验收。

---

## 43. 今日 Git 提交建议

```bash
git status
git add day16
git commit -m "Learn smart pointer ownership models"
```

README 建议记录：

```text
1. unique_ptr表示独占所有权并通过移动转移
2. shared_ptr只用于确实需要共享生命周期的对象
3. weak_ptr观察对象但不延长其生命周期
4. get不转移、release不释放、reset会释放
5. shared_ptr循环引用需要用weak_ptr打破
6. 函数参数类型应表达是否接管所有权
7. 智能指针不保证对象内部线程安全或访问安全
```

---

## 44. 今日一句话总结

```text
优先用unique_ptr表达唯一所有者，只有确实共享生命周期时才用shared_ptr，只观察共享对象时用weak_ptr；选择智能指针的依据始终是所有权，而不是语法方便。
```
