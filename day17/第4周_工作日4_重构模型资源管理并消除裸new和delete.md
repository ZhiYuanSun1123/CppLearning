# 第 4 周·工作日 4：重构模型资源管理代码并消除裸 `new` 和 `delete`

## 0. 前置知识与超纲说明

### 今天必须掌握

- “消除裸 `new/delete`”是消除**手工所有权管理**，不是禁止所有裸指针；
- 能放在栈上的对象优先直接作为局部变量或成员变量；
- 动态连续数组优先使用 `std::vector<T>`；
- 独占的动态单对象优先使用 `std::unique_ptr<T>`；
- 只有真正共享生命周期时才使用 `std::shared_ptr<T>`；
- 非拥有访问优先使用引用，允许为空时可以使用非拥有裸指针；
- 使用 `std::make_unique`和 `std::make_shared`创建智能指针；
- 工厂函数可以返回 `std::unique_ptr<Base>`；
- 用 `std::move`把唯一所有权交给成员、容器或函数；
- 重构后尽量遵循 Rule of Zero，不再手写析构、复制和移动函数；
- 使用析构日志、生命周期计数、单元测试和 ASan 验证重构结果；
- 重构必须保持原有业务行为，而不仅仅是“能够编译”。

### 今天会复习

- 栈、堆与对象生命周期；
- RAII；
- `new/delete`与 `new[]/delete[]`；
- Rule of Five；
- 移动构造和 `std::move`；
- `unique_ptr`、`shared_ptr`和 `weak_ptr`；
- 虚函数、抽象类、运行时多态和虚析构；
- 异常处理；
- ASan和单元测试。

### 今天第一次系统理解

- 如何从一段旧式裸指针代码判断资源所有者；
- 如何为每个裸指针标注“拥有”或“仅观察”；
- 如何按照所有权逐步重构，而不是机械替换类型；
- 基本异常保证与“先构造新资源，再提交状态”的思想；
- Rule of Zero在工程代码中的价值；
- 如何让函数签名表达借用、接管和共享所有权；
- 如何验证重构前后外部行为一致。

### 今天只需了解，后续再深入

- 自定义删除器；
- PImpl模式；
- `std::span`与非拥有连续视图；
- `std::pmr`和自定义内存资源；
- `shared_ptr`控制块底层实现；
- C API、ONNX Runtime、TensorRT和CUDA句柄的专用RAII封装；
- 静态分析工具的高级规则配置。

### 超纲提示

文档中可能出现以下尚未系统学习的内容：

```text
模板语法：unique_ptr<T>、vector<T>
工厂模式：根据名称创建不同后端
Rule of Zero：让成员类型自动管理资源
依赖注入：从外部把后端所有权交给服务
```

今天只要求会使用和理解所有权，不要求实现模板或背设计模式术语。模板会在第6周系统学习。

### 今天明确不做

- 不把每个裸指针机械替换为 `shared_ptr`；
- 不为了“看起来现代”改变原有业务逻辑；
- 不在一次重构中同时修改大量无关功能；
- 不使用 `release()`逃避所有权设计；
- 不把栈对象地址交给智能指针管理；
- 不用智能指针掩盖对象之间的循环拥有；
- 不提前实现 ONNX、TensorRT 或硬件资源管理；
- 不手写一个新的智能指针类。

> 今天的核心不是删除两个关键字，而是让每一份资源的创建、拥有、借用、转移和销毁都能从类型与接口中看出来。

---

## 1. 今日目标

完成后你应该能够：

- 阅读旧式模型资源代码并画出所有权关系；
- 区分拥有型裸指针与非拥有型裸指针；
- 将裸动态数组重构为 `vector`；
- 将独占模型对象重构为 `unique_ptr`；
- 让模型工厂返回 `unique_ptr<ModelBackend>`；
- 通过移动把模型后端安装到服务中；
- 删除不再需要的析构函数和 Rule of Five代码；
- 正确保留只观察对象的引用或裸指针；
- 判断何时不应该改成 `shared_ptr`；
- 验证异常路径不泄漏；
- 验证替换模型时旧资源及时释放；
- 使用警告、测试、ASan和文本搜索完成重构验收。

建议用时：约 **3～4小时**。

```text
30分钟：旧代码审计与所有权标注
35分钟：数组和单对象资源替换规则
35分钟：接口所有权重构
35分钟：工厂与多态后端重构
30分钟：Rule of Zero与异常安全
60～90分钟：模型服务重构、测试和ASan验证
```

---

## 2. 今日目录

已经建立：

```text
day17/
├── include/
├── src/
├── tests/
├── target/
│   └── report/
└── 第4周_工作日4_重构模型资源管理并消除裸new和delete.md
```

建议你逐步创建：

```text
include/
├── legacy_model_service.hpp
├── model_backend.hpp
├── model_factory.hpp
├── model_service.hpp
└── test_runner.hpp

src/
├── 01_legacy_resource_code.cpp
├── 02_stack_value_refactor.cpp
├── 03_vector_refactor.cpp
├── 04_unique_backend.cpp
├── 05_factory_refactor.cpp
├── 06_service_ownership.cpp
├── 07_exception_path.cpp
├── 08_non_owning_access.cpp
├── model_backend.cpp
├── model_factory.cpp
├── model_service.cpp
└── test_runner.cpp

tests/
└── model_resource_tests.cpp
```

源码文件由你完成。建议先保存一份最小旧实现，再逐步重构并对比测试结果。

---

## 3. “裸 `new/delete`”具体指什么

下面是显式手工管理动态对象：

```cpp
ModelBackend* backend = new QwenBackend();

backend->infer("audio.wav");

delete backend;
backend = nullptr;
```

风险包括：

- 中间抛异常时跳过 `delete`；
- 提前 `return`时忘记释放；
- 多个返回分支需要重复清理；
- 不清楚谁负责销毁；
- 可能重复释放；
- 可能使用已经释放的地址；
- 基类没有虚析构时，通过基类地址销毁子类存在风险。

今天要将这种“拥有型裸指针”改成：

```cpp
auto backend =
    std::make_unique<QwenBackend>();

backend->infer("audio.wav");
```

作用域结束时自动释放。

---

## 4. 消除裸 `new/delete`不等于消除全部裸指针

下面的函数只临时观察对象：

```cpp
void print_backend(
    const ModelBackend* backend
) {
    if (backend != nullptr) {
        std::cout << backend->name() << '\n';
    }
}
```

这个裸指针可以是合理的，因为：

- 函数不拥有对象；
- 函数不执行 `delete`；
- 参数允许为空；
- 生命周期由调用者保证。

如果对象不能为空，更推荐引用：

```cpp
void print_backend(
    const ModelBackend& backend
) {
    std::cout << backend.name() << '\n';
}
```

今天的目标是：

```text
代码中不再出现手工拥有型new/delete
非拥有访问仍可按语义使用引用或裸指针
```

---

## 5. 重构前先做资源清单

不要打开文件后直接全局替换。先列出资源：

| 成员/变量 | 创建位置 | 当前销毁位置 | 是否拥有 | 是否允许为空 | 是否共享 |
|---|---|---|---|---|---|
| `backend_` | 构造/加载函数 | 析构函数 | 是 | 是 | 否 |
| `samples_` | 加载音频 | 析构函数 | 是 | 否 | 否 |
| `logger_` | 外部传入 | 外部 | 否 | 是 | 否 |
| `session_` | 外部工厂 | 多个请求 | 可能共享 | 否 | 是 |

然后分别决定：

```text
backend_ → unique_ptr<ModelBackend>
samples_ → vector<float>
logger_  → Logger* 或 Logger&（非拥有）
session_ → 只有证明确实共享时才shared_ptr
```

如果你不能回答“谁负责销毁”，就还不能安全重构。

---

## 6. 替换决策表

| 旧代码 | 首选替代 | 说明 |
|---|---|---|
| `T* p = new T(...)` | `auto p = make_unique<T>(...)` | 单一动态对象 |
| `T* p = new T[n]` | `vector<T> p(n)` | 动态连续数组 |
| 固定长度小数组 | `array<T, N>` | 长度编译期确定，后续系统学习 |
| 可为空非拥有参数 | `const T*` | 不负责销毁 |
| 不可为空非拥有参数 | `const T&` | 不负责销毁 |
| 多个真正所有者 | `shared_ptr<T>` | 必须有共享生命周期理由 |
| 观察共享对象 | `weak_ptr<T>` | 不延长生命周期 |
| 普通成员对象 | 直接写 `T member_;` | 不需要动态分配 |

注意：最好的替代有时不是智能指针，而是一个普通值对象。

---

## 7. 第一优先级：能用值对象就不用堆

旧代码：

```cpp
class Request {
public:
    Request()
        : question_(new std::string()) {
    }

    ~Request() {
        delete question_;
    }

private:
    std::string* question_;
};
```

如果 `question_`始终存在，完全没必要动态分配：

```cpp
class Request {
private:
    std::string question_;
};
```

改进结果：

- 没有 `new/delete`；
- 没有空指针状态；
- 没有额外堆分配；
- 不需要自定义析构；
- 默认复制和移动行为通常正确。

规则：

```text
对象与宿主生命周期一致且总是存在 → 直接成员对象
```

---

## 8. 动态数组优先重构为 `vector`

旧代码：

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

重构：

```cpp
#include <vector>

class AudioBuffer {
public:
    explicit AudioBuffer(std::size_t size)
        : data_(size, 0.0F) {
    }

private:
    std::vector<float> data_;
};
```

长度可以通过：

```cpp
data_.size()
```

获得，因此 `size_`也可能不再需要。

原类可能需要手写 Rule of Five；重构后 `vector`自动正确管理复制、移动和析构，类通常可以回到 Rule of Zero。

---

## 9. 什么是 Rule of Zero

如果类的成员本身已经正确管理资源：

```cpp
class ModelService {
private:
    std::unique_ptr<ModelBackend> backend_;
    std::vector<float> samples_;
    std::string model_name_;
};
```

通常不需要手写：

```cpp
~ModelService();
ModelService(const ModelService&);
ModelService& operator=(const ModelService&);
ModelService(ModelService&&);
ModelService& operator=(ModelService&&);
```

让成员类型决定行为：

- `unique_ptr`使类默认不可复制；
- `unique_ptr`支持移动；
- `vector`和 `string`自动释放资源；
- 析构时所有成员逆序自动析构。

这就是 Rule of Zero：

> 业务类尽量不直接管理裸资源，从而不需要自己实现特殊成员函数。

编译器是否隐式生成某个操作仍受成员类型和已声明特殊成员函数影响。今天只要求通过编译测试确认你的类是可移动、不可复制，而不背完整生成规则。

---

## 10. 独占多态后端使用 `unique_ptr<Base>`

接口：

```cpp
class ModelBackend {
public:
    virtual ~ModelBackend() = default;

    virtual std::string name() const = 0;

    virtual std::string infer(
        const std::string& audio_path
    ) const = 0;
};
```

服务独占一个后端：

```cpp
class ModelService {
public:
    explicit ModelService(
        std::unique_ptr<ModelBackend> backend
    );

private:
    std::unique_ptr<ModelBackend> backend_;
};
```

实现：

```cpp
ModelService::ModelService(
    std::unique_ptr<ModelBackend> backend
)
    : backend_(std::move(backend)) {
    if (!backend_) {
        throw std::invalid_argument(
            "backend cannot be null"
        );
    }
}
```

这里的语义非常清楚：

```text
调用者创建后端
通过移动把唯一所有权交给ModelService
ModelService析构时自动销毁后端
```

---

## 11. 为什么构造函数按值接收 `unique_ptr`

```cpp
explicit ModelService(
    std::unique_ptr<ModelBackend> backend
);
```

按值接收表示函数要获得一份所有权。因为 `unique_ptr`不能复制，调用者必须明确移动：

```cpp
auto backend =
    std::make_unique<QwenBackend>();

ModelService service(
    std::move(backend)
);
```

调用后：

```cpp
backend == nullptr
```

构造函数内部再次：

```cpp
backend_(std::move(backend))
```

是因为形参 `backend`虽然类型与移动有关，但它有名字，在表达式中是左值。必须再次明确把它的资源交给成员。

---

## 12. 工厂函数返回 `unique_ptr<Base>`

声明：

```cpp
std::unique_ptr<ModelBackend>
create_backend(const std::string& type);
```

实现：

```cpp
std::unique_ptr<ModelBackend>
create_backend(const std::string& type) {
    if (type == "qwen") {
        return std::make_unique<QwenBackend>();
    }

    if (type == "whisper") {
        return std::make_unique<WhisperBackend>();
    }

    throw std::invalid_argument(
        "unsupported backend: " + type
    );
}
```

调用：

```cpp
auto backend = create_backend("qwen");
ModelService service(std::move(backend));
```

工厂隐藏了具体子类创建逻辑，调用者只依赖统一接口。

不需要写：

```cpp
return std::move(backend);
```

返回局部智能指针时直接 `return backend;`即可。

---

## 13. 替换已有后端

```cpp
class ModelService {
public:
    void replace_backend(
        std::unique_ptr<ModelBackend> backend
    );

private:
    std::unique_ptr<ModelBackend> backend_;
};
```

实现：

```cpp
void ModelService::replace_backend(
    std::unique_ptr<ModelBackend> backend
) {
    if (!backend) {
        throw std::invalid_argument(
            "backend cannot be null"
        );
    }

    backend_ = std::move(backend);
}
```

赋值时：

1. 新后端已经成功创建并进入形参；
2. 检查新后端有效；
3. 移动赋值给成员；
4. `backend_`原来拥有的旧后端自动销毁；
5. 不需要手写 `delete`。

---

## 14. 异常路径为什么更安全

旧代码：

```cpp
ModelBackend* backend = new QwenBackend();

load_config();       // 可能抛异常
backend->warmup();

delete backend;
```

如果 `load_config()`抛异常，最后的 `delete`不会执行。

重构：

```cpp
auto backend =
    std::make_unique<QwenBackend>();

load_config();       // 可能抛异常
backend->warmup();
```

异常展开栈时，局部 `unique_ptr`的析构函数仍会执行，被管理对象自动释放。

这与之前的 `FileHandle`完全一致：

```text
资源绑定到局部RAII对象
正常返回和异常退出都执行析构
```

但它不能保证 `warmup()`一定完成，只能保证已获得资源被正确清理。

---

## 15. 先创建新对象，再修改旧状态

危险重构：

```cpp
backend_.reset();
backend_ = create_backend(type);
```

如果创建新后端失败，旧后端已经提前丢失，服务进入空状态。

更稳妥：

```cpp
auto new_backend = create_backend(type);

// 创建成功后再提交
backend_ = std::move(new_backend);
```

如果 `create_backend`抛异常，成员 `backend_`仍保持原值。

这与拷贝赋值中“先申请新内存，再释放旧内存”的原则相同：

```text
先准备可能失败的新状态
成功后再替换旧状态
```

---

## 16. 不要为了消除裸指针滥用 `shared_ptr`

下面只有服务拥有后端：

```cpp
class ModelService {
private:
    std::unique_ptr<ModelBackend> backend_;
};
```

改成：

```cpp
std::shared_ptr<ModelBackend> backend_;
```

并不会自动“更安全”。它会让其他代码能够延长后端寿命，销毁时机变得更难判断。

只有需求明确为：

```text
服务可能被销毁，但已经分发出去的异步请求仍必须让模型会话继续存活
```

才有理由把模型会话设计成共享所有权。

当前练习中的服务只有一个所有者，所以首选 `unique_ptr`。

---

## 17. 非拥有接口优先使用引用

服务只调用后端，不保存所有权：

```cpp
std::string run_once(
    const ModelBackend& backend,
    const std::string& audio_path
) {
    return backend.infer(audio_path);
}
```

如果后端允许不存在：

```cpp
std::string try_run(
    const ModelBackend* backend,
    const std::string& audio_path
) {
    if (backend == nullptr) {
        return "backend unavailable";
    }

    return backend->infer(audio_path);
}
```

不要写成：

```cpp
void run_once(
    std::shared_ptr<ModelBackend> backend
);
```

除非函数确实需要保存一份共享所有权。仅仅临时调用不需要增加引用计数，也不需要改变接口的生命周期语义。

---

## 18. `get()`只能用于临时观察和旧接口适配

如果旧函数是非拥有接口：

```cpp
void print_backend(
    const ModelBackend* backend
);
```

可以：

```cpp
auto backend = create_backend("qwen");
print_backend(backend.get());
```

调用期间 `backend`仍然拥有对象。

不能：

```cpp
ModelBackend* raw = backend.get();
backend.reset();
raw->infer("audio.wav");
```

`raw`已经悬空。

也不能在旧函数里：

```cpp
delete backend;
```

如果旧 API 会接管并释放裸指针，必须先明确它的所有权契约，不能直接传 `get()`。

---

## 19. 一个旧式模型服务示例

下面代码故意包含多个问题，仅用于审计：

```cpp
class LegacyModelService {
public:
    LegacyModelService(
        ModelBackend* backend,
        std::size_t sample_count
    )
        : backend_(backend),
          samples_(new float[sample_count]),
          sample_count_(sample_count) {
    }

    ~LegacyModelService() {
        delete backend_;
        delete[] samples_;
    }

    void replace_backend(ModelBackend* backend) {
        delete backend_;
        backend_ = backend;
    }

private:
    ModelBackend* backend_;
    float* samples_;
    std::size_t sample_count_;
};
```

问题：

1. 构造函数参数看不出是否接管所有权；
2. 调用者可能仍然 `delete backend`；
3. 默认拷贝会浅拷贝两个地址；
4. 两个对象析构时可能重复释放；
5. 替换后端时先删除旧后端，新参数若就是同一地址会出问题；
6. 数组长度和地址分别维护；
7. 析构函数和所有特殊成员函数都需要手工考虑；
8. 异常路径复杂。

---

## 20. 重构后的模型服务

```cpp
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

class ModelService {
public:
    ModelService(
        std::unique_ptr<ModelBackend> backend,
        std::size_t sample_count
    )
        : backend_(std::move(backend)),
          samples_(sample_count, 0.0F) {
        if (!backend_) {
            throw std::invalid_argument(
                "backend cannot be null"
            );
        }
    }

    void replace_backend(
        std::unique_ptr<ModelBackend> backend
    ) {
        if (!backend) {
            throw std::invalid_argument(
                "backend cannot be null"
            );
        }

        backend_ = std::move(backend);
    }

    const ModelBackend& backend() const {
        return *backend_;
    }

    std::size_t sample_count() const noexcept {
        return samples_.size();
    }

private:
    std::unique_ptr<ModelBackend> backend_;
    std::vector<float> samples_;
};
```

改进：

- 没有裸 `new/delete`；
- 所有权由构造函数签名表达；
- 数组由 `vector`管理；
- 不需要自定义析构函数；
- 默认不可复制，避免浅拷贝；
- 可以通过移动转移整个服务；
- 旧后端在替换时自动销毁；
- 异常路径由 RAII清理。

---

## 21. 注意构造函数检查顺序

前面的实现先把形参移动给成员：

```cpp
: backend_(std::move(backend))
```

然后检查：

```cpp
if (!backend_)
```

这是合理的，因为成员现在是实际所有者。

也可以在函数体中检查形参再移动，但成员必须先被默认初始化：

```cpp
ModelService::ModelService(
    std::unique_ptr<ModelBackend> backend
)
    : backend_(nullptr) {
    if (!backend) {
        throw std::invalid_argument(
            "backend cannot be null"
        );
    }

    backend_ = std::move(backend);
}
```

今天推荐第一种，代码更紧凑。若检查失败，已移入成员的空 `unique_ptr`仍能安全析构。

---

## 22. 重构过程要保持行为不变

重构不是添加功能。重构前先固定行为：

```text
输入qwen时创建QwenBackend
输入whisper时创建WhisperBackend
未知类型抛invalid_argument
空后端被拒绝
推理返回原有格式
替换后端后新后端生效
旧后端立即析构
```

先为这些行为写测试，再更改内部资源管理。

如果重构后测试仍通过，说明外部行为保持一致；如果只验证“编译成功”，无法确认业务逻辑有没有被改坏。

---

## 23. 建议的渐进式重构顺序

### 第一步：建立基线

- 编译旧代码；
- 运行现有测试；
- 保存输出；
- 使用 ASan记录已有问题。

### 第二步：标注所有权

给每个指针写临时审计注释：

```cpp
ModelBackend* backend_; // owning
Logger* logger_;        // non-owning
```

### 第三步：先替换数组

```text
new T[n]/delete[] → vector<T>
```

### 第四步：替换独占单对象

```text
new T/delete → make_unique<T>/unique_ptr<T>
```

### 第五步：修改接口

让接管所有权的参数按值接收 `unique_ptr`，临时使用改为引用。

### 第六步：删除手工特殊成员函数

确认成员都能正确管理资源后，删除不再必要的析构、拷贝和移动代码。

### 第七步：验证

- 编译警告为零；
- 单元测试通过；
- ASan无错误；
- 搜索没有裸 `new/delete`；
- 析构计数归零。

每一步保持程序可编译，出现问题时更容易定位。

---

## 24. 不能机械删除析构函数

只有当所有资源都交给 RAII成员管理后，才能删除手写析构。

错误顺序：

```text
成员仍是拥有型裸指针
先删析构函数
程序泄漏
```

正确顺序：

```text
确认资源所有权
把成员改成值对象/vector/unique_ptr
修改构造和接口
测试通过
最后删除不再需要的析构和Rule of Five
```

---

## 25. 拷贝能力可能发生变化

旧类拥有裸指针时，编译器可能允许危险的浅拷贝：

```cpp
LegacyModelService second = first;
```

改成 `unique_ptr`后，类会自然变成不可复制：

```cpp
ModelService second = first; // 编译期错误
```

这通常是正确改进，因为独占后端不应被复制。

仍可移动：

```cpp
ModelService second = std::move(first);
```

是否允许移动要通过实际编译验证。如果你手写了析构函数，可能影响隐式移动操作的生成，所以 Rule of Zero有助于获得符合成员语义的默认行为。

---

## 26. 什么时候真的需要深拷贝

如果业务明确要求两个完全独立的模型服务副本，不能简单把 `unique_ptr`换成 `shared_ptr`，因为共享不等于复制。

可能需要设计：

```cpp
virtual std::unique_ptr<ModelBackend>
clone() const = 0;
```

每个子类创建自己的副本。这是多态克隆模式，今天只需了解，不作为必做内容。

当前练习把 `ModelService`设计为不可复制、可移动。

---

## 27. 搜索裸 `new/delete`

在 day17目录运行：

```bash
rg -n '\bnew\b|\bdelete\b' day17
```

注意结果可能包含：

- Markdown中的讲解代码；
- 故意保留的 legacy示例；
- 注释；
- `= delete`，它不是释放表达式。

因此不能只看“搜索结果为零”，需要逐条判断。

若只检查重构后的正式源码：

```bash
rg -n '\bnew\b|\bdelete\b' \
  day17/include \
  day17/src \
  day17/tests
```

如果保留 `01_legacy_resource_code.cpp`作为错误案例，应明确排除：

```bash
rg -n '\bnew\b|\bdelete\b' \
  day17/include \
  day17/src \
  day17/tests \
  -g '!01_legacy_resource_code.cpp'
```

---

## 28. 常见错误

### 错误 1：把所有裸指针改成 `shared_ptr`

这会把非拥有关系错误地变成共享所有权。

### 错误 2：使用智能指针但仍手动 `delete`

```cpp
auto backend = std::make_unique<QwenBackend>();
delete backend.get();
```

最终会重复释放。

### 错误 3：调用 `release()`只为消除编译错误

```cpp
backend.release();
```

这通常只是把所有权问题变成泄漏。

### 错误 4：把 `unique_ptr`按复制方式传参

```cpp
service.install(backend);
```

接管唯一所有权时应明确：

```cpp
service.install(std::move(backend));
```

### 错误 5：移动后继续使用原所有者

```cpp
service.install(std::move(backend));
backend->infer("audio.wav");
```

此时 `backend`为空。

### 错误 6：通过基类智能指针销毁，但基类析构非虚

多态基类必须使用虚析构函数。

### 错误 7：先清空旧资源再创建新资源

新资源创建失败时会丢失旧状态。应先创建成功，再移动提交。

### 错误 8：重构时顺便修改业务输出

同时改变资源管理和业务行为，会让测试失败时难以判断原因。

### 错误 9：删掉所有裸指针接口

非拥有、可空观察接口仍可以合理使用 `const T*`。

### 错误 10：只看 ASan，不写行为测试

没有内存错误不代表调用了正确后端或返回了正确结果。

---

## 29. 练习 1：所有权审计

阅读下面代码，不修改：

```cpp
class InferenceService {
public:
    InferenceService(
        ModelBackend* backend,
        Logger* logger
    );

    ~InferenceService();

private:
    ModelBackend* backend_;
    Logger* logger_;
};
```

根据以下业务契约分析：

```text
服务负责销毁backend
logger由main创建并保证比服务活得久
服务不负责销毁logger
```

要求：

1. 标注哪个是拥有型指针；
2. 标注哪个是非拥有型指针；
3. 给出成员类型重构方案；
4. 说明为什么两者不能都改成 `shared_ptr`；
5. 说明 `logger_`使用引用成员或裸指针各有什么前提。

---

## 30. 练习 2：值对象重构

把下面成员：

```cpp
std::string* model_name_;
```

连同构造、析构和访问函数重构为：

```cpp
std::string model_name_;
```

要求：

- 删除对应 `new/delete`；
- 删除无意义的空指针判断；
- 保持原有 `name()`输出一致；
- 解释为什么普通成员是比 `unique_ptr<string>`更好的选择。

---

## 31. 练习 3：音频数组重构

将：

```cpp
float* samples_;
std::size_t sample_count_;
```

重构为：

```cpp
std::vector<float> samples_;
```

要求实现：

```cpp
std::size_t sample_count() const noexcept;
void set_sample(std::size_t index, float value);
float sample(std::size_t index) const;
```

要求：

- 使用 `vector::size()`；
- 越界时抛 `std::out_of_range`；
- 不保留重复的长度状态；
- 不出现 `new[]/delete[]`；
- 验证复制后两个缓冲区互不影响；
- 验证移动后目标保存数据。

---

## 32. 练习 4：后端独占所有权重构

把：

```cpp
ModelBackend* backend_;
```

重构为：

```cpp
std::unique_ptr<ModelBackend> backend_;
```

要求：

- 构造函数按值接收 `unique_ptr<ModelBackend>`；
- 初始化列表使用 `std::move`；
- 拒绝空后端；
- 删除手写 `delete backend_`；
- 基类析构为虚函数；
- 调用端使用 `make_unique`；
- 移交后验证调用端指针为空。

---

## 33. 练习 5：模型工厂

实现：

```cpp
std::unique_ptr<ModelBackend>
create_backend(const std::string& type);
```

支持：

```text
qwen    → QwenBackend
whisper → WhisperBackend
```

未知类型：

```cpp
throw std::invalid_argument(...);
```

要求：

- 使用 `make_unique`；
- 返回基类智能指针；
- 不写裸 `new`；
- 不写 `return std::move(...)`；
- 测试两个后端的动态分派；
- 测试未知类型异常消息。

---

## 34. 练习 6：安全替换后端

实现：

```cpp
void ModelService::replace_backend(
    const std::string& type
);
```

要求按照：

```cpp
auto new_backend = create_backend(type);
backend_ = std::move(new_backend);
```

完成。

测试：

1. 初始为 Qwen；
2. 成功替换为 Whisper；
3. 旧 Qwen析构计数增加；
4. 新调用执行 Whisper；
5. 替换为未知类型抛异常；
6. 异常后原 Whisper仍可使用。

第 6 点用于证明“先创建、后提交”保留了旧状态。

---

## 35. 练习 7：异常路径资源释放

创建测试后端：

```cpp
class FailingBackend : public ModelBackend {
public:
    FailingBackend();
    ~FailingBackend() override;
    void warmup();
};
```

让 `warmup()`故意抛异常。使用 `unique_ptr`创建对象并调用。

要求：

- 捕获异常；
- 验证析构函数执行；
- 验证 `alive_count`恢复为0；
- ASan无泄漏；
- 解释 RAII保证了什么、没有保证什么。

---

## 36. 练习 8：非拥有访问

分别实现：

```cpp
void print_required(
    const ModelBackend& backend
);

void print_optional(
    const ModelBackend* backend
);
```

要求：

- `print_required`不进行空判断；
- `print_optional`处理 `nullptr`；
- 两者都不保存地址；
- 两者都不释放对象；
- 用 `unique_ptr`调用时分别传 `*backend`和 `backend.get()`；
- 解释为什么这不违反“消除裸 `new/delete`”。

---

## 37. 练习 9：Rule of Zero验证

完成重构后的 `AudioBuffer`和 `ModelService`，然后：

1. 删除手写析构函数；
2. 删除手写拷贝与移动操作；
3. 验证 `AudioBuffer`可复制、可移动；
4. 验证 `ModelService`不可复制；
5. 验证 `ModelService`可以移动；
6. 解释这些行为分别由哪些成员类型决定。

可以用编译实验验证，不要求今天使用尚未学习的类型特征模板。

---

## 38. 练习 10：完整模型资源管理项目

整合：

```text
ModelBackend抽象接口
QwenBackend与WhisperBackend
create_backend工厂
ModelService独占后端
vector管理音频样本
引用/裸指针提供非拥有访问
后端安全替换
异常路径验证
生命周期计数器
单元测试
ASan报告
```

正式源码要求：

- 不出现裸 `new`；
- 不出现资源释放表达式 `delete`或 `delete[]`；
- 不使用 `release()`；
- 不为了方便使用 `shared_ptr`；
- 不手写析构和 Rule of Five，除非某个类确实直接管理非RAII资源；
- 所有权转移均可从函数签名看出。

---

## 39. 单元测试要求

文件：`tests/model_resource_tests.cpp`

至少覆盖：

```text
1. 工厂创建QwenBackend
2. 工厂创建WhisperBackend
3. 未知后端类型抛invalid_argument
4. 服务拒绝空unique_ptr
5. 服务构造后调用者unique_ptr为空
6. 服务调用正确的动态后端
7. vector保存正确的音频样本数量
8. vector越界访问抛out_of_range
9. 成功替换后端后旧对象析构
10. 替换后调用新后端
11. 创建新后端失败时旧后端仍可用
12. 非拥有引用调用不改变生命周期
13. 非拥有空指针得到预期结果
14. 服务移动后目标对象可用
15. 所有生命周期计数最终归零
16. ASan零错误
```

测试名称示例：

```text
factory creates qwen backend
service takes unique ownership
failed replacement preserves old backend
old backend is destroyed after successful replacement
non-owning access does not extend lifetime
all backend instances are destroyed
```

---

## 40. 生命周期计数器

为了测试，可以在基类或测试后端中加入：

```cpp
class TrackedBackend : public ModelBackend {
public:
    TrackedBackend() {
        ++alive_count_;
    }

    ~TrackedBackend() override {
        --alive_count_;
    }

    static int alive_count() noexcept {
        return alive_count_;
    }

private:
    inline static int alive_count_ = 0;
};
```

注意：这只是测试观测手段，不是资源管理方式。测试结束时：

```cpp
EXPECT_EQ(TrackedBackend::alive_count(), 0);
```

如果你继续使用自己的 `TestRunner`，按 day13的断言风格实现即可。

---

## 41. 编译命令

示例程序：

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
  src/model_factory.cpp \
  src/model_service.cpp \
  src/08_non_owning_access.cpp \
  -I include \
  -o target/08_non_owning_access
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
  src/model_backend.cpp \
  src/model_factory.cpp \
  src/model_service.cpp \
  src/test_runner.cpp \
  tests/model_resource_tests.cpp \
  -I include \
  -o target/model_resource_tests
```

如果出现：

```text
Undefined symbols for architecture arm64
```

优先检查：

- 是否只声明没有实现；
- 对应 `.cpp`是否加入命令；
- 类名或函数名是否拼写不一致；
- 成员函数声明与实现的 `const`是否一致。

---

## 42. 运行和保存报告

```bash
./target/model_resource_tests \
  > target/report/model_resource_tests.txt \
  2>&1

echo $?
```

最终应该满足：

```text
所有行为测试通过
退出码为0
没有heap-use-after-free
没有double-free
没有heap-buffer-overflow
没有已检测到的内存泄漏
alive_count最终为0
```

如需查看完整输出：

```bash
cat target/report/model_resource_tests.txt
```

---

## 43. 重构前后对比报告

在 `target/report/refactor_summary.md`中记录：

```text
一、重构前资源
- backend_：拥有型裸指针
- samples_：拥有型动态数组
- logger_：非拥有观察指针

二、重构决策
- backend_ → unique_ptr<ModelBackend>
- samples_ → vector<float>
- logger_ → 保留非拥有指针或改为引用

三、删除的手工代码
- 析构中的delete
- 析构中的delete[]
- 裸数组长度同步逻辑
- 手写复制/移动代码

四、验证证据
- 编译警告
- 单元测试
- ASan结果
- 生命周期计数
- 裸new/delete搜索结果

五、行为保持
- 工厂结果一致
- 推理输出一致
- 异常类型一致
- 替换逻辑一致
```

这份报告是比“我学过智能指针”更有工程价值的产出。

---

## 44. 今日必须回答的问题

1. 消除裸 `new/delete`是否意味着不允许任何裸指针？
2. 拥有型指针与非拥有型指针有什么区别？
3. 为什么能用值对象时不应该动态分配？
4. `new T[n]`通常应替换为什么？
5. `new Derived`且只有一个所有者时应替换为什么？
6. 为什么不能把所有指针都改成 `shared_ptr`？
7. 什么情况下参数应该使用 `const T&`？
8. 什么情况下可以使用 `const T*`？
9. 什么情况下按值接收 `unique_ptr<T>`？
10. 为什么接收 `unique_ptr`的调用通常需要 `std::move`？
11. 为什么具名形参移入成员时还要再次 `std::move`？
12. 为什么多态基类必须有虚析构函数？
13. 工厂为什么返回 `unique_ptr<Base>`？
14. 返回局部智能指针为什么通常不写 `std::move`？
15. Rule of Zero是什么？
16. `vector`如何帮助删除手写 Rule of Five？
17. 为什么替换资源时应该先成功创建新资源？
18. `get()`适合解决什么问题？
19. 为什么不能释放 `get()`返回的地址？
20. 为什么调用 `release()`往往不是正确重构？
21. 重构为什么必须先固定原有行为？
22. ASan通过是否代表业务行为一定正确？
23. 生命周期计数器能提供什么证据？
24. 怎样检查正式源码中是否还存在裸 `new/delete`？
25. 今天的模型服务为什么使用 `unique_ptr`而不是 `shared_ptr`？

---

## 45. 今日验收清单

### 所有权设计

- [ ] 为每个旧指针标注拥有或观察关系；
- [ ] 能用值对象的成员不再动态分配；
- [ ] 动态数组使用 `vector`；
- [ ] 独占后端使用 `unique_ptr`；
- [ ] 非拥有访问使用引用或明确的观察指针；
- [ ] 没有无理由引入 `shared_ptr`；
- [ ] 所有权转移能从接口签名看出。

### 代码

- [ ] 工厂返回 `unique_ptr<ModelBackend>`；
- [ ] 使用 `make_unique`创建具体后端；
- [ ] 服务构造函数接管后端；
- [ ] 后端替换采用先创建、后提交；
- [ ] 基类析构为虚函数；
- [ ] 删除不再需要的析构函数；
- [ ] 删除不再需要的 Rule of Five代码；
- [ ] 正式源码无拥有型裸 `new/delete`。

### 验证

- [ ] 重构前行为基线已记录；
- [ ] Qwen与Whisper工厂测试通过；
- [ ] 空后端测试通过；
- [ ] 替换成功测试通过；
- [ ] 替换失败保留旧状态测试通过；
- [ ] 所有生命周期计数归零；
- [ ] 编译警告为零；
- [ ] ASan无已检测到的错误；
- [ ] 生成重构对比报告。

---

## 46. 与 AI 部署工程的联系

后续部署代码会管理：

```text
模型后端对象
运行时Session
音频样本缓冲区
预处理器
Tokenizer
推理请求
网络连接
CPU与GPU资源句柄
```

今天建立的基本策略是：

```text
普通配置、名称、请求数据
    → 值对象

动态音频/张量CPU数据
    → vector或专用容器

服务独占的多态后端
    → unique_ptr<Base>

多个异步任务确实共同拥有的会话
    → shared_ptr

监控器或索引只观察共享会话
    → weak_ptr

调用期间临时使用且不能为空
    → const T&

调用期间临时使用且允许为空
    → const T*
```

当你以后接触 ONNX Runtime、TensorRT 或 CUDA 时，会发现部分资源由 C API创建，需要调用专用函数释放，而不是普通 `delete`。届时会在今天的原则上使用：

```text
专用RAII包装类
或带自定义删除器的智能指针
```

今天不设计硬件，也不实现这些框架；今天的项目产出是一个资源所有权清晰、可测试、无裸拥有型 `new/delete`的模拟模型服务。

---

## 47. 今日项目产出

今天完成后，你应该得到一个小型但可展示的工程重构记录：

```text
项目：Model Resource Management Refactor

重构前：
- 裸指针拥有模型后端
- 裸动态数组保存音频样本
- 手写析构和复制风险

重构后：
- unique_ptr管理多态后端
- vector管理音频样本
- 工厂返回明确所有权
- 服务支持异常安全的后端替换
- Rule of Zero减少手写资源代码
- 单元测试和ASan验证
```

未来整理简历项目时，这部分可以作为“代码质量与资源安全改造”证据，但单独一天的练习不应夸大成工业级项目。它会继续积累到后面的完整推理服务项目中。

---

## 48. 今日 Git 提交建议

先检查：

```bash
git status
git diff
```

提交：

```bash
git add day17
git commit -m "Refactor model resource ownership with RAII"
```

README建议记录：

```text
1. 消除的是手工所有权管理，不是所有裸指针
2. 能用值对象时不动态分配
3. 动态数组优先使用vector
4. 独占多态后端使用unique_ptr<Base>
5. 非拥有访问使用引用或观察指针
6. 先创建新资源，成功后再替换旧资源
7. RAII成员让业务类尽量遵循Rule of Zero
8. 重构结果由行为测试、生命周期计数和ASan共同验证
```

---

## 49. 今日一句话总结

```text
现代C++资源重构不是把裸指针全部换成shared_ptr，而是先明确所有权，再用值对象、vector、unique_ptr、引用和必要的shared_ptr分别表达正确的生命周期关系，并通过测试证明行为与资源释放都正确。
```
