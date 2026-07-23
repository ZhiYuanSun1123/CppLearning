# 第 2 周·工作日 4：实现 RAII 文件句柄封装并验证异常路径自动释放

## 1. 今日任务

今天需要完成一个小型 `FileHandle` 类，用它管理 C 标准库中的 `FILE*` 文件资源。

完成后你应该能够：

- 解释什么是 RAII；
- 说明构造函数和析构函数分别负责什么；
- 解释为什么异常会导致普通语句被跳过；
- 理解异常传播时的栈展开；
- 使用析构函数自动调用 `fclose`；
- 防止一个文件句柄被多个对象重复关闭；
- 让资源类的析构函数保持 `noexcept`；
- 验证正常路径和异常路径都会释放文件；
- 认识标准库 `std::ifstream`、`std::ofstream` 本身也是 RAII 类型；
- 使用 AddressSanitizer 和编译警告验证程序。

建议用时：约 **3 小时**。

今天的最终产出：

```text
day07/
├── src/
│   ├── 01_manual_file.cpp
│   ├── 02_exception_leak.cpp
│   ├── 03_raii_file_baseline.cpp
│   ├── 04_raii_exception.cpp
│   ├── 05_open_failure.cpp
│   ├── 06_copy_forbidden.cpp
│   └── 07_file_handle_complete.cpp
├── include/
│   └── file_handle.hpp
├── target/
└── notes/
    └── day07-raii-file-handle.md
```

今天不要求你学习复杂的操作系统文件描述符，也不涉及硬件设计。这里选择文件，只是因为它是一种容易观察的资源；同一种 RAII 思想以后还会用于：

- 动态内存；
- 互斥锁；
- Socket；
- 数据库连接；
- CUDA 显存；
- TensorRT 对象；
- 音频设备和音频流。

---

## 2. 什么是资源

资源不只是 `new` 出来的内存。

程序向系统申请并且需要归还的东西，都可以看作资源，例如：

```text
动态内存       new / delete
文件           fopen / fclose
互斥锁         lock / unlock
Socket         socket / close
CUDA显存       cudaMalloc / cudaFree
```

这些接口通常成对出现。只获取、不释放会造成资源泄漏；释放两次则可能造成未定义行为。

以文件为例：

```cpp
std::FILE* file = std::fopen("output.txt", "w");

// 使用文件

std::fclose(file);
```

问题在于：程序不一定总能顺利执行到最后的 `fclose`。

---

## 3. 手动管理资源的问题

观察以下代码：

```cpp
#include <cstdio>
#include <stdexcept>

void save_result() {
    std::FILE* file = std::fopen("result.txt", "w");

    if (file == nullptr) {
        throw std::runtime_error("无法打开文件");
    }

    std::fputs("first line\n", file);

    throw std::runtime_error("模拟处理中途失败");

    std::fclose(file);
}
```

执行到：

```cpp
throw std::runtime_error("模拟处理中途失败");
```

之后，当前函数剩余的普通语句不会继续执行。因此：

```cpp
std::fclose(file);
```

被跳过，文件资源没有按照原计划释放。

这类代码的问题不是 `fclose` 写错了，而是资源释放依赖程序必须走到某一行。

实际工程中还可能存在更多提前退出路径：

```cpp
if (condition_a) {
    return;
}

if (condition_b) {
    throw std::runtime_error("error");
}

if (condition_c) {
    return;
}
```

如果每个分支都手动释放资源，代码很容易漏掉某一条路径。

---

## 4. RAII 的核心思想

RAII 的完整名称是：

```text
Resource Acquisition Is Initialization
```

可以理解为：

> 在对象构造时获得资源，在对象析构时释放资源。

基本结构：

```cpp
class Resource {
public:
    Resource() {
        // 获取资源
    }

    ~Resource() {
        // 释放资源
    }
};
```

局部对象离开作用域时，析构函数会自动执行。

正常离开作用域：

```cpp
void normal_path() {
    Resource resource;
} // 自动调用 resource 的析构函数
```

通过 `return` 提前离开：

```cpp
void early_return() {
    Resource resource;
    return;
} // 仍然自动调用析构函数
```

异常离开：

```cpp
void exception_path() {
    Resource resource;
    throw std::runtime_error("失败");
} // 栈展开过程中自动调用析构函数
```

RAII 的价值在于：

```text
资源释放依赖对象生命周期，
而不是依赖程序员记得在每条路径上手写清理语句。
```

---

## 5. 异常和栈展开

当程序执行：

```cpp
throw std::runtime_error("模拟错误");
```

运行时会寻找能够处理该异常的 `catch`。在从抛出位置向外寻找的过程中，已经成功构造的局部对象会按相反顺序析构，这个过程叫作栈展开。

例如：

```cpp
void process() {
    Resource first;
    Resource second;

    throw std::runtime_error("失败");
}
```

顺序大致是：

```text
构造 first
构造 second
抛出异常
析构 second
析构 first
进入外层 catch
```

因此，只要资源已经被 RAII 对象接管，异常就不会跳过它的析构函数。

注意：

- 只有已经构造成功的对象才会析构；
- 如果构造函数自己抛出异常，该对象的析构函数不会执行；
- 构造函数中已经获取的资源必须在抛出前妥善处理，或者先交给其他 RAII 成员管理；
- 不要在析构函数中向外抛出异常。

---

## 6. 第一个 FileHandle 封装

文件：`include/file_handle.hpp`

```cpp
#ifndef FILE_HANDLE_HPP
#define FILE_HANDLE_HPP

#include <cstdio>
#include <stdexcept>
#include <string>

class FileHandle {
public:
    FileHandle(const std::string& path, const char* mode)
        : file_(std::fopen(path.c_str(), mode)) {
        if (file_ == nullptr) {
            throw std::runtime_error("无法打开文件: " + path);
        }
    }

    ~FileHandle() noexcept {
        if (file_ != nullptr) {
            std::fclose(file_);
        }
    }

    std::FILE* get() noexcept {
        return file_;
    }

    const std::FILE* get() const noexcept {
        return file_;
    }

private:
    std::FILE* file_;
};

#endif
```

逐部分理解。

### 6.1 构造函数获取文件资源

```cpp
FileHandle(const std::string& path, const char* mode)
    : file_(std::fopen(path.c_str(), mode)) {
```

成员初始化列表将 `fopen` 返回的文件指针保存到 `file_`。

如果打开失败：

```cpp
if (file_ == nullptr) {
    throw std::runtime_error("无法打开文件: " + path);
}
```

构造失败时，调用者拿不到一个半有效的 `FileHandle` 对象。

这建立了一个清晰的不变量：

> 只要 `FileHandle` 对象构造成功，它就持有一个有效文件资源。

### 6.2 析构函数释放文件资源

```cpp
~FileHandle() noexcept {
    if (file_ != nullptr) {
        std::fclose(file_);
    }
}
```

当对象离开作用域时，析构函数自动调用 `fclose`。

`noexcept` 表示析构函数不会向调用者传播异常。资源清理失败可以记录日志，但不要在栈展开期间再抛出另一个异常。

### 6.3 get 不转移所有权

```cpp
std::FILE* get() noexcept {
    return file_;
}
```

`get()` 只是让调用者临时使用底层指针，不代表调用者获得所有权。

因此，外部代码不应该这样做：

```cpp
std::fclose(file.get()); // 错误：破坏 FileHandle 的所有权
```

否则 `FileHandle` 析构时还会再次关闭，造成重复释放。

---

## 7. 必须处理复制问题

当前类如果允许默认复制：

```cpp
FileHandle first("result.txt", "w");
FileHandle second = first;
```

那么 `first.file_` 和 `second.file_` 会指向同一个 `FILE` 对象。

作用域结束时：

```text
second 析构 → fclose
first 析构  → 再次 fclose
```

这会造成重复关闭。

对于当前阶段，最简单安全的处理是禁止复制：

```cpp
FileHandle(const FileHandle&) = delete;
FileHandle& operator=(const FileHandle&) = delete;
```

更新后的类：

```cpp
class FileHandle {
public:
    FileHandle(const std::string& path, const char* mode)
        : file_(std::fopen(path.c_str(), mode)) {
        if (file_ == nullptr) {
            throw std::runtime_error("无法打开文件: " + path);
        }
    }

    ~FileHandle() noexcept {
        if (file_ != nullptr) {
            std::fclose(file_);
        }
    }

    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    std::FILE* get() noexcept {
        return file_;
    }

private:
    std::FILE* file_;
};
```

今天的必做版本只要求“独占资源且不可复制”。移动语义放在可选提高部分。

---

## 8. 编译方式

项目根目录假设为：

```text
day07/
├── include/
├── src/
└── target/
```

编译：

```bash
clang++ -std=c++17 -g -O0 \
  -Wall -Wextra -Wpedantic \
  -fsanitize=address \
  -fno-omit-frame-pointer \
  src/04_raii_exception.cpp \
  -I include \
  -o target/04_raii_exception
```

运行：

```bash
./target/04_raii_exception
```

参数含义：

| 参数 | 作用 |
|---|---|
|`-std=c++17`|使用 C++17|
|`-g`|保留调试信息|
|`-O0`|教学阶段关闭优化，方便观察执行过程|
|`-Wall`|打开常用警告|
|`-Wextra`|打开额外警告|
|`-Wpedantic`|检查标准兼容性|
|`-fsanitize=address`|启用 ASan|
|`-fno-omit-frame-pointer`|保留较完整调用栈|
|`-I include`|添加头文件搜索目录|

在 VS Code 的 `tasks.json` 中，这三个警告参数必须是三个独立字符串：

```json
"-Wall",
"-Wextra",
"-Wpedantic"
```

不要写成：

```json
"-Wall -Wextra -Wpedantic"
```

---

## 9. 开始前自测

1. 文件为什么是一种资源？
2. `fopen` 和 `fclose` 为什么必须成对使用？
3. `throw` 之后，同一函数中剩余的普通语句还会执行吗？
4. 什么是栈展开？
5. 局部对象在异常传播时会不会析构？
6. RAII 中构造函数和析构函数分别承担什么责任？
7. 为什么析构函数通常不应该抛出异常？
8. 为什么持有同一 `FILE*` 的两个对象可能造成重复关闭？
9. `= delete` 在这里解决了什么问题？
10. `get()` 返回底层指针是否等于转移所有权？
11. `std::ofstream` 是否需要手动调用析构函数？
12. RAII 能否保证文件中的业务数据一定正确？

---

## 10. 实验 1：观察手动文件管理

文件：`01_manual_file.cpp`

```cpp
#include <cstdio>
#include <iostream>

int main() {
    std::FILE* file = std::fopen("manual.txt", "w");

    if (file == nullptr) {
        std::cerr << "打开文件失败\n";
        return 1;
    }

    std::fputs("manual resource management\n", file);

    if (std::fclose(file) != 0) {
        std::cerr << "关闭文件失败\n";
        return 1;
    }

    file = nullptr;
    return 0;
}
```

任务：

1. 编译并运行；
2. 确认 `manual.txt` 被创建；
3. 解释 `file == nullptr` 的含义；
4. 解释为什么 `fclose` 后将本地指针设为 `nullptr`；
5. 标出获取资源和释放资源的代码行；
6. 思考如果中间增加 `return` 或 `throw`，哪些路径会跳过 `fclose`。

---

## 11. 实验 2：观察异常跳过手动清理

文件：`02_exception_leak.cpp`

```cpp
#include <cstdio>
#include <iostream>
#include <stdexcept>

void write_without_raii() {
    std::FILE* file = std::fopen("without_raii.txt", "w");

    if (file == nullptr) {
        throw std::runtime_error("打开文件失败");
    }

    std::fputs("data before exception\n", file);
    std::cout << "文件已经打开，准备抛出异常\n";

    throw std::runtime_error("模拟业务处理失败");

    std::fclose(file);
}

int main() {
    try {
        write_without_raii();
    } catch (const std::exception& error) {
        std::cerr << "捕获异常: " << error.what() << '\n';
    }

    return 0;
}
```

任务：

1. 运行前预测哪些输出会出现；
2. 标出永远无法执行的 `fclose`；
3. 解释为什么 `catch` 捕获异常不等于文件已自动关闭；
4. 不要把“把 `fclose` 复制进每一个 `catch`”当作最终方案；
5. 记录手动资源管理在多条退出路径下的维护风险。

这个程序用于展示设计问题。不同平台在进程结束时会回收进程资源，但这不代表运行期间的泄漏是正确代码。

---

## 12. 实验 3：实现最小 RAII 文件类

文件：`03_raii_file_baseline.cpp`

```cpp
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

class FileHandle {
public:
    FileHandle(const std::string& path, const char* mode)
        : file_(std::fopen(path.c_str(), mode)) {
        if (file_ == nullptr) {
            throw std::runtime_error("无法打开文件: " + path);
        }

        std::cout << "FileHandle: 文件打开成功\n";
    }

    ~FileHandle() noexcept {
        if (file_ != nullptr) {
            std::fclose(file_);
            std::cout << "FileHandle: 文件已经关闭\n";
        }
    }

    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    void write(const std::string& text) {
        if (std::fputs(text.c_str(), file_) == EOF) {
            throw std::runtime_error("文件写入失败");
        }
    }

private:
    std::FILE* file_;
};

int main() {
    try {
        FileHandle file("raii_baseline.txt", "w");
        file.write("RAII baseline\n");
    } catch (const std::exception& error) {
        std::cerr << "错误: " << error.what() << '\n';
        return 1;
    }

    std::cout << "main 正常结束\n";
    return 0;
}
```

预期输出顺序：

```text
FileHandle: 文件打开成功
FileHandle: 文件已经关闭
main 正常结束
```

任务：

1. 画出 `file` 对象的生命周期；
2. 找到资源获取位置；
3. 找到资源释放位置；
4. 解释为什么代码中没有显式调用 `file` 的析构函数；
5. 解释析构输出为什么早于 `main 正常结束`；
6. 查看生成文件中的内容；
7. 使用 ASan 运行并确认没有报告。

不要手动这样调用：

```cpp
file.~FileHandle();
```

局部对象的析构由语言规则自动完成，手动调用后还可能在作用域结束时再次析构。

---

## 13. 实验 4：验证异常路径自动释放

文件：`04_raii_exception.cpp`

```cpp
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

class FileHandle {
public:
    FileHandle(const std::string& path, const char* mode)
        : file_(std::fopen(path.c_str(), mode)) {
        if (file_ == nullptr) {
            throw std::runtime_error("无法打开文件: " + path);
        }

        std::cout << "1. 构造：文件打开成功\n";
    }

    ~FileHandle() noexcept {
        if (file_ != nullptr) {
            std::fclose(file_);
            std::cout << "3. 析构：文件自动关闭\n";
        }
    }

    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    void write(const std::string& text) {
        if (std::fputs(text.c_str(), file_) == EOF) {
            throw std::runtime_error("文件写入失败");
        }
    }

private:
    std::FILE* file_;
};

void process_file() {
    FileHandle file("exception_path.txt", "w");
    file.write("data before exception\n");

    std::cout << "2. 即将抛出异常\n";
    throw std::runtime_error("模拟处理中途失败");

    std::cout << "这行不会执行\n";
}

int main() {
    try {
        process_file();
    } catch (const std::exception& error) {
        std::cout << "4. catch：捕获异常：" << error.what() << '\n';
    }

    std::cout << "5. 程序继续执行\n";
    return 0;
}
```

预期顺序：

```text
1. 构造：文件打开成功
2. 即将抛出异常
3. 析构：文件自动关闭
4. catch：捕获异常：模拟处理中途失败
5. 程序继续执行
```

任务：

1. 编译并运行程序；
2. 将实际输出和预测输出对比；
3. 确认“这行不会执行”没有出现；
4. 确认析构输出出现在 `catch` 之前；
5. 用自己的话解释栈展开；
6. 删除显式 `throw`，验证正常路径同样会关闭文件；
7. 把异常改成从 `file.write()` 中抛出，观察结论是否相同；
8. 使用 ASan 再运行一次。

核心验收不是“程序捕获了异常”，而是：

> 在异常离开 `process_file` 的过程中，局部 `FileHandle` 被析构，文件资源随之释放。

---

## 14. 实验 5：验证构造失败

文件：`05_open_failure.cpp`

```cpp
#include "file_handle.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        FileHandle file(
            "/this/directory/should/not/exist/result.txt",
            "w"
        );

        std::cout << "不应该执行到这里\n";
    } catch (const std::exception& error) {
        std::cerr << "预期内的打开失败: "
                  << error.what()
                  << '\n';
    }

    return 0;
}
```

任务：

1. 确认构造函数抛出了异常；
2. 确认调用者没有获得无效的 `FileHandle` 对象；
3. 解释为什么不应该在 `file_ == nullptr` 时继续执行；
4. 解释该对象的析构函数为什么不会执行；
5. 思考构造函数在抛出前是否已经获得了需要释放的有效文件。

这里 `fopen` 返回 `nullptr`，说明资源没有获取成功，因此没有有效文件需要关闭。

---

## 15. 实验 6：验证禁止复制

文件：`06_copy_forbidden.cpp`

```cpp
#include "file_handle.hpp"

int main() {
    FileHandle first("copy_test.txt", "w");
    FileHandle second = first;

    return 0;
}
```

这段代码预期编译失败。

任务：

1. 编译并保存编译错误；
2. 找到错误中关于 deleted constructor 的内容；
3. 解释如果允许复制，两个对象会持有什么；
4. 解释为什么两个析构函数会造成重复关闭；
5. 删除复制语句，重新编译；
6. 不要为了通过编译而删除类中的 `= delete`。

预期思想：

```text
这个类拥有资源，所以必须明确复制语义。
当前版本采用独占所有权，因此禁止复制。
```

---

## 16. 综合练习：完成可复用的 FileHandle

文件：`include/file_handle.hpp`

你需要独立完成以下接口：

```cpp
#ifndef FILE_HANDLE_HPP
#define FILE_HANDLE_HPP

#include <cstdio>
#include <string>

class FileHandle {
public:
    FileHandle(const std::string& path, const char* mode);
    ~FileHandle() noexcept;

    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    void write(const std::string& text);
    void flush();

    std::FILE* get() noexcept;
    const std::FILE* get() const noexcept;

private:
    std::FILE* file_;
};

#endif
```

建议实现：

```cpp
#include "file_handle.hpp"

#include <stdexcept>

FileHandle::FileHandle(
    const std::string& path,
    const char* mode
)
    : file_(std::fopen(path.c_str(), mode)) {
    if (file_ == nullptr) {
        throw std::runtime_error("无法打开文件: " + path);
    }
}

FileHandle::~FileHandle() noexcept {
    if (file_ != nullptr) {
        std::fclose(file_);
    }
}

void FileHandle::write(const std::string& text) {
    if (std::fputs(text.c_str(), file_) == EOF) {
        throw std::runtime_error("文件写入失败");
    }
}

void FileHandle::flush() {
    if (std::fflush(file_) != 0) {
        throw std::runtime_error("刷新文件失败");
    }
}

std::FILE* FileHandle::get() noexcept {
    return file_;
}

const std::FILE* FileHandle::get() const noexcept {
    return file_;
}
```

源文件：`src/07_file_handle_complete.cpp`

```cpp
#include "file_handle.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

void export_audio_result(
    const std::string& path,
    bool simulate_failure
) {
    FileHandle output(path, "w");

    output.write("audio_id,score\n");
    output.write("sample_001,0.95\n");
    output.flush();

    if (simulate_failure) {
        throw std::runtime_error("模拟音频推理结果处理中断");
    }

    output.write("sample_002,0.87\n");
}

int main() {
    try {
        export_audio_result("normal_result.csv", false);
        std::cout << "正常路径完成\n";
    } catch (const std::exception& error) {
        std::cerr << "正常路径意外失败: "
                  << error.what()
                  << '\n';
        return 1;
    }

    try {
        export_audio_result("exception_result.csv", true);
    } catch (const std::exception& error) {
        std::cerr << "异常路径已捕获: "
                  << error.what()
                  << '\n';
    }

    std::cout << "两条路径验证结束\n";
    return 0;
}
```

编译：

```bash
clang++ -std=c++17 -g -O0 \
  -Wall -Wextra -Wpedantic \
  -fsanitize=address \
  -fno-omit-frame-pointer \
  src/file_handle.cpp \
  src/07_file_handle_complete.cpp \
  -I include \
  -o target/07_file_handle_complete
```

运行：

```bash
./target/07_file_handle_complete
```

检查：

```bash
cat normal_result.csv
cat exception_result.csv
echo $?
```

预期：

- 正常路径文件包含两条数据；
- 异常路径文件至少包含 `flush()` 前已经写入的数据；
- 异常路径被 `catch` 处理；
- 程序退出码为 `0`；
- ASan 没有报告；
- 两条路径中的文件最终都会被析构函数关闭。

重要区别：

> RAII 保证资源得到释放，但不保证一次由多步构成的业务操作具有事务性。

异常文件可能只包含部分数据。这是业务一致性问题，不是资源泄漏问题。真正需要“要么完整写入，要么完全不产生结果”时，可以采用临时文件写完后再重命名等方案，后续工程阶段再学习。

---

## 17. 练习题

### 练习 1：生命周期顺序

不运行代码，先写出输出顺序：

```cpp
#include <iostream>
#include <stdexcept>

class Trace {
public:
    explicit Trace(const char* name)
        : name_(name) {
        std::cout << "construct " << name_ << '\n';
    }

    ~Trace() {
        std::cout << "destroy " << name_ << '\n';
    }

private:
    const char* name_;
};

void run() {
    Trace first("first");
    Trace second("second");
    throw std::runtime_error("stop");
}

int main() {
    try {
        run();
    } catch (const std::exception&) {
        std::cout << "caught\n";
    }
}
```

要求解释为什么析构顺序与构造顺序相反。

### 练习 2：找出资源泄漏路径

阅读：

```cpp
void save(bool fail) {
    std::FILE* file = std::fopen("data.txt", "w");

    if (file == nullptr) {
        return;
    }

    if (fail) {
        throw std::runtime_error("failed");
    }

    std::fclose(file);
}
```

回答：

1. 哪条路径正常关闭？
2. 哪条路径跳过关闭？
3. 用 `FileHandle` 改写。

### 练习 3：完成 write

补全：

```cpp
void write(const std::string& text) {
    // 写入失败时抛出 std::runtime_error
}
```

要求：

- 使用 `std::fputs`；
- 判断返回值是否为 `EOF`；
- 不在该函数中关闭文件。

### 练习 4：解释所有权

回答：

1. 谁拥有 `FILE*`？
2. `get()` 的调用者能否执行 `fclose`？
3. 为什么复制构造被删除？
4. 析构函数应当执行几次有效的 `fclose`？

### 练习 5：正常路径和异常路径

编写函数：

```cpp
void generate_report(bool fail);
```

要求：

- 创建一个 `FileHandle`；
- 写入标题；
- 当 `fail == true` 时抛出异常；
- 否则继续写入正文；
- 在 `main` 中分别调用 `true` 和 `false`；
- 两次调用结束后程序仍能继续运行。

### 练习 6：禁止复制实验

故意复制 `FileHandle`，观察编译错误，然后回答：

```text
这是编译期错误还是运行期错误？
为什么尽早在编译期阻止复制更好？
```

### 练习 7：析构函数为什么不抛异常

假设程序正在处理异常 A，栈展开时析构函数又抛出异常 B。查阅并记录这种情况为什么危险，以及 C++ 通常会如何处理。

本题只要求形成一段自己的解释，不要求实现会终止程序的危险演示。

### 练习 8：联系昨天的动态数组

比较：

```cpp
int* values = new int[5]{};
delete[] values;
```

和：

```cpp
std::vector<int> values(5);
```

回答：

1. 哪一个需要手写释放？
2. `std::vector` 如何体现 RAII？
3. 异常发生时，局部 `vector` 是否会自动析构？
4. 为什么业务代码优先使用容器而不是裸 `new[]`？

---

## 18. 可选提高：支持移动语义

今天必做部分可以只禁止复制。如果学有余力，可以让 `FileHandle` 支持所有权转移。

需要加入：

```cpp
FileHandle(FileHandle&& other) noexcept
    : file_(other.file_) {
    other.file_ = nullptr;
}

FileHandle& operator=(FileHandle&& other) noexcept {
    if (this != &other) {
        if (file_ != nullptr) {
            std::fclose(file_);
        }

        file_ = other.file_;
        other.file_ = nullptr;
    }

    return *this;
}
```

关键点：

- 移动不是复制底层句柄；
- 新对象接管所有权；
- 原对象必须置为 `nullptr`；
- 两个对象最终只能有一个执行有效的 `fclose`；
- 移动构造和移动赋值通常声明为 `noexcept`。

测试：

```cpp
FileHandle create_file() {
    FileHandle file("move.txt", "w");
    return file;
}

int main() {
    FileHandle file = create_file();
}
```

这一部分属于提高内容。如果移动语义尚未系统学习，只需要知道“独占资源类不能随意复制”，不要强求今天完全掌握移动赋值。

---

## 19. 标准库已经提供的 RAII 文件类型

实际 C++ 业务代码通常优先考虑：

```cpp
#include <fstream>

void save() {
    std::ofstream output("result.txt");

    if (!output) {
        throw std::runtime_error("无法打开文件");
    }

    output << "hello\n";
} // output 自动关闭文件
```

`std::ofstream` 本身就是 RAII 类型。

今天自己实现 `FileHandle` 的目的不是重新发明标准库，而是理解以后阅读下面这些代码时的共同思想：

```text
std::vector
std::string
std::unique_ptr
std::lock_guard
std::ifstream
std::ofstream
```

它们都利用对象生命周期管理资源。

选择原则：

- 普通 C++ 文件读写：优先 `std::ifstream`、`std::ofstream`；
- 必须调用 C 接口时：可以封装 `FILE*`；
- 不能确定所有权时：先明确谁负责释放，再写代码；
- 不要为了练习 RAII 而在真实项目中替换成熟标准库。

---

## 20. 与 AI 工程方向的联系

RAII 是你后续学习 C++ AI 部署的重要基础。

以后可能看到：

```cpp
cudaMalloc(&device_pointer, bytes);
cudaFree(device_pointer);
```

或者：

```cpp
TensorRTObject* object = create_object();
object->destroy();
```

也可能管理：

- ONNX Runtime Session；
- TensorRT Engine；
- CUDA Stream；
- CUDA Event；
- 音频输入流；
- Socket；
- 线程锁。

这些资源如果只靠手工清理，很容易在模型加载失败、推理异常或提前返回时泄漏。

RAII 的通用结构仍然是：

```text
构造函数获取资源
对象保持唯一、清晰的所有权
析构函数释放资源
异常路径依赖栈展开自动清理
```

因此，今天不是孤立地学习一个文件类，而是在为后续 C++ 推理部署代码建立资源管理习惯。

---

## 21. 今日笔记模板

创建：

```text
notes/day07-raii-file-handle.md
```

内容：

```markdown
# RAII文件句柄与异常安全

## 什么是资源

## RAII的定义

## 构造函数的责任

## 析构函数的责任

## 什么是栈展开

## 正常路径析构顺序

## 异常路径析构顺序

## 为什么禁止复制FileHandle

## get是否转移所有权

## 为什么析构函数不应抛异常

## FileHandle的不变量

## RAII能保证什么

## RAII不能保证什么

## std::ofstream为什么也是RAII

## 与vector和unique_ptr的联系

## 与AI部署资源管理的联系

## 今天遇到的问题
```

---

## 22. 仓库整理建议

建议目录：

```text
cpp-foundation-labs/
├── day07-raii-file-handle/
│   ├── README.md
│   ├── include/
│   │   └── file_handle.hpp
│   ├── src/
│   │   ├── file_handle.cpp
│   │   ├── 01_manual_file.cpp
│   │   ├── 02_exception_leak.cpp
│   │   ├── 03_raii_file_baseline.cpp
│   │   ├── 04_raii_exception.cpp
│   │   ├── 05_open_failure.cpp
│   │   ├── 06_copy_forbidden.cpp
│   │   └── 07_file_handle_complete.cpp
│   └── CMakeLists.txt
└── notes/
    └── day07-raii-file-handle.md
```

README 至少说明：

- 为什么要使用 RAII；
- `FileHandle` 拥有什么资源；
- 为什么禁止复制；
- 正常路径如何验证；
- 异常路径如何验证；
- RAII 保证资源释放，但不保证文件内容事务完整；
- 编译和运行命令。

提交示例：

```bash
git status
git add day07-raii-file-handle notes/day07-raii-file-handle.md
git commit -m "Add RAII file handle and exception safety lab"
git push
```

不要提交：

- 编译后的可执行文件；
- `.DS_Store`；
- 临时生成但没有说明用途的大量输出；
- 含有本机敏感绝对路径的日志。

---

## 23. 今日验收清单

- [ ] 能解释资源不仅包括动态内存；
- [ ] 能说明 `fopen` 与 `fclose` 必须成对；
- [ ] 能解释异常为什么会跳过后续普通语句；
- [ ] 能解释什么是栈展开；
- [ ] 已实现 `FileHandle` 构造函数；
- [ ] 构造失败会抛出异常；
- [ ] 已实现 `noexcept` 析构函数；
- [ ] 析构函数只在指针非空时关闭文件；
- [ ] 已删除复制构造函数；
- [ ] 已删除复制赋值运算符；
- [ ] 明白 `get()` 不转移所有权；
- [ ] 正常路径会自动关闭文件；
- [ ] `return` 路径会自动关闭文件；
- [ ] 异常路径会自动关闭文件；
- [ ] 析构输出发生在外层 `catch` 之前；
- [ ] 打开不存在的路径能正确报告错误；
- [ ] 禁止复制实验产生预期编译错误；
- [ ] 完成正常路径和异常路径综合测试；
- [ ] 使用 ASan 运行后没有内存错误；
- [ ] 编译器警告为零；
- [ ] 完成 RAII 学习笔记；
- [ ] README 能让其他人独立编译复现；
- [ ] 代码和文档已整理进仓库。

---

## 24. 今日完成标准

达到以下状态才算完成：

> 你能够使用一个不可复制的 C++ 对象独占 `FILE*`，在构造时打开文件、在析构时关闭文件，并通过故意抛出异常证明栈展开仍会自动执行析构函数。

如果时间不足，按以下优先级完成：

1. 理解手动管理在异常路径上的问题；
2. 实现最小 `FileHandle`；
3. 禁止复制；
4. 完成正常路径实验；
5. 完成异常路径实验；
6. 完成打开失败实验；
7. 使用 ASan 和编译警告验证；
8. 整理笔记和 README；
9. 最后再尝试移动语义。

