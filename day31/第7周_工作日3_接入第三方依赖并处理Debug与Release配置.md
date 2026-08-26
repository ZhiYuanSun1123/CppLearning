# 第7周·工作日3：接入第三方依赖并处理 Debug 与 Release 配置

## 0. 今日任务

今天继续复用 `day27` 的张量元数据项目，不重新编写一套业务代码。

你要完成两件事：

1. 使用现代 CMake 接入第三方格式化库 `fmt`；
2. 分别构建、运行和测试 Debug、Release 两种配置。

完成后的关系如下：

```text
tensor_metadata                       自己的核心静态库
        │
        ├── tensor_metadata_tests     只依赖核心库
        │
        └── tensor_metadata_demo      依赖核心库和fmt
                         │
                         └── fmt::fmt 第三方target
```

构建目录如下：

```text
同一套源代码
    │
    ├── day31/build-debug/
    │       ├── 调试信息
    │       ├── 通常较少优化
    │       └── 用于开发和定位错误
    │
    └── day31/build-release/
            ├── 启用优化
            ├── 通常定义NDEBUG
            └── 用于性能和交付验证
```

今天结束时，你应该能够：

1. 解释什么是第三方依赖；
2. 区分头文件库和需要编译的库；
3. 理解 `find_package()`、`FetchContent` 和包管理器的分工；
4. 使用 `FetchContent` 接入固定版本的 `fmt`；
5. 使用 `fmt::fmt` 这种导入 target；
6. 解释为什么不应该手写第三方库的 `-I` 和 `-L` 路径；
7. 解释 `PRIVATE`、`PUBLIC`、`INTERFACE` 在依赖传播中的作用；
8. 区分 Debug 和 Release；
9. 在 macOS 默认的单配置生成器下正确设置 `CMAKE_BUILD_TYPE`；
10. 使用生成器表达式给不同配置设置宏；
11. 分别运行两套程序和测试；
12. 输出一份可复现的依赖与构建配置报告。

---

## 1. 今天内容的分类

为了避免把 CMake 命令混在一起，今天分成五类。

### 第一类：依赖是什么

```text
第三方头文件
第三方静态库或动态库
编译要求
传递依赖
版本约束
```

### 第二类：获得依赖的方式

```text
find_package
FetchContent
add_subdirectory
Conan或vcpkg
```

### 第三类：连接 target

```text
target_link_libraries
PRIVATE
PUBLIC
INTERFACE
fmt::fmt
```

### 第四类：构建配置

```text
Debug
Release
RelWithDebInfo
MinSizeRel
CMAKE_BUILD_TYPE
生成器表达式
```

### 第五类：验证

```text
配置日志
构建日志
运行结果
CTest
文件大小
编译命令
```

### 今天必须掌握

```text
FetchContent_Declare
FetchContent_MakeAvailable
固定依赖版本
target_link_libraries
fmt::fmt
CMAKE_BUILD_TYPE
Debug与Release的区别
$<$<CONFIG:...>:...>
两个独立build目录
```

### 今天只需要认识

```text
Conan
vcpkg
CMake package config
Find模块
install与export
依赖锁文件
交叉编译toolchain file
```

这些内容会说明用途，但今天不要求搭建完整包管理体系。

---

## 2. 今日涉及的文件

今天使用这些明确的文件名：

```text
CppLearning/
├── day27/
│   ├── CMakeLists.txt                       # 练习中修改
│   ├── include/                             # 不修改
│   ├── src/                                 # 不修改
│   ├── app/
│   │   └── main.cpp                         # 练习中修改
│   └── tests/
│       └── test_tensor_metadata.cpp         # 不修改
│
└── day31/
    ├── build-debug/                         # CMake生成，不提交
    ├── build-release/                       # CMake生成，不提交
    ├── report/
    │   ├── debug-configure.txt
    │   ├── debug-build.txt
    │   ├── debug-run.txt
    │   ├── release-configure.txt
    │   ├── release-build.txt
    │   ├── release-run.txt
    │   ├── test-results.txt
    │   └── dependency-and-config-report.md
    └── 第7周_工作日3_接入第三方依赖并处理Debug与Release配置.md
```

不要把下面这些生成目录提交到 Git：

```text
day31/build-debug/
day31/build-release/
```

---

## 3. 什么是第三方依赖

如果代码不是你当前项目自己实现的，但你的项目需要使用它，它就是第三方依赖。

今天使用：

```text
fmt
```

它提供类型安全的格式化功能，例如：

```cpp
const auto message = fmt::format(
    "tensor={}, rank={}",
    name,
    rank
);
```

这里的 `fmt::format()` 不属于 C++17 标准库，也不是我们自己实现的，因此必须把 `fmt` 接入构建系统。

一个第三方依赖通常不只有头文件，还可能包含：

```text
include/                 对外声明
src/                     实现
libxxx.a                 静态库
libxxx.dylib             macOS动态库
CMake配置文件            告诉消费者如何使用
编译选项和宏             使用该库所需条件
间接依赖                 它自己依赖的其他库
版本信息                 避免不兼容
```

所以“接入依赖”不是简单地让 `#include` 不报红。

完整过程是：

```text
获得依赖
   ↓
让CMake认识依赖
   ↓
把依赖target连接给消费者
   ↓
编译阶段找到头文件
   ↓
链接阶段找到实现
   ↓
动态库还要在运行时被找到
```

---

## 4. 头文件库和编译库

### 4.1 只有头文件的库

有些库的大部分或全部实现都写在头文件中：

```text
third_party/include/example.hpp
```

消费者编译时直接看到实现，通常不需要额外链接二进制文件。

但它仍可能要求：

```text
include目录
编译宏
C++标准版本
其他依赖
```

因此现代 CMake 仍然常用一个 `INTERFACE` target 表达这些要求。

### 4.2 需要编译的库

另一类库包含 `.cpp` 实现：

```text
头文件提供声明
库文件提供实现
```

消费者通常同时需要：

```text
编译阶段：头文件路径
链接阶段：库文件和依赖关系
```

今天使用 `fmt::fmt`，它由 fmt 项目对外提供。我们不应该猜测其真实文件路径，而应该链接官方定义的 target。

---

## 5. 为什么不能只写 `#include <fmt/format.h>`

这一行只是源代码层面的声明：

```cpp
#include <fmt/format.h>
```

编译器仍然需要知道：

1. `fmt/format.h` 在哪个目录；
2. `fmt` 使用什么编译配置；
3. 是否需要链接 `libfmt`；
4. 当前平台对应哪个库文件；
5. `fmt` 是否还有其他依赖。

如果只写头文件但没有正确连接依赖，可能出现两类错误。

### 编译错误

```text
fatal error: 'fmt/format.h' file not found
```

含义：编译器找不到声明。

### 链接错误

```text
Undefined symbols for architecture arm64
```

含义：声明找到了，但链接器没有找到实现。

---

## 6. 接入第三方依赖的四种常见方式

### 6.1 `find_package()`

典型写法：

```cmake
find_package(fmt CONFIG REQUIRED)

target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    fmt::fmt
)
```

它的前提是：依赖已经安装，并且 CMake 能找到依赖提供的配置文件。

适合：

```text
系统已经安装依赖
Conan或vcpkg已经准备依赖
SDK自带CMake package config
企业统一维护开发环境
```

优点：

```text
配置快
不会每个项目都重新下载和构建
适合系统或包管理器提供的依赖
```

风险：

```text
不同电脑可能安装了不同版本
CMake可能找不到配置文件
环境可复现性依赖外部安装步骤
```

### 6.2 `FetchContent`

典型写法：

```cmake
include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.2.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(fmt)
```

它会在配置阶段获取依赖，并把第三方项目加入当前构建。

适合：

```text
教学项目
小型开源项目
依赖数量不多
希望克隆后直接配置
```

优点：

```text
不要求使用者预先安装fmt
版本写在项目中
项目更容易复现
```

风险：

```text
第一次配置需要网络
依赖较多时配置和构建会变慢
大型工程的版本和缓存管理会变复杂
```

### 6.3 `add_subdirectory()`

如果第三方源码已经放进仓库：

```cmake
add_subdirectory(third_party/fmt)
```

然后仍然链接：

```cmake
target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    fmt::fmt
)
```

优点是离线可构建；缺点是仓库体积变大，而且第三方源码更新需要自己管理。

### 6.4 Conan或vcpkg

它们属于 C++ 包管理工具，负责：

```text
下载依赖
处理版本
处理平台和编译器差异
缓存已经构建的包
向CMake提供依赖信息
```

今天只需要认识，不要求安装。

后续依赖变多时，再系统学习其中一个。现在同时学习 CMake、Conan、vcpkg 会分散注意力。

---

## 7. 为什么依赖必须固定版本

不推荐：

```cmake
GIT_TAG main
```

因为 `main` 会持续变化。今天能构建，不代表一个月后仍然能构建。

今天使用：

```cmake
GIT_TAG 11.2.0
```

这表达：

```text
项目明确依赖fmt 11.2.0
```

更严格的生产项目还可能固定 Git commit：

```cmake
GIT_TAG 40626af88bd7df9a5fb80be7b25ac85b122d6c21
```

今天固定版本标签即可。

---

## 8. 什么是第三方 target

`FetchContent_MakeAvailable(fmt)` 执行后，fmt 项目会创建可供消费者使用的 target：

```cmake
fmt::fmt
```

这个名字不是文件路径，而是逻辑 target 名称。

它可以携带：

```text
头文件目录
库文件
编译宏
编译特性
间接依赖
不同平台的链接信息
```

因此推荐：

```cmake
target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    fmt::fmt
)
```

不推荐手写：

```cmake
include_directories(/某个电脑上的/fmt/include)
link_directories(/某个电脑上的/fmt/lib)
target_link_libraries(tensor_metadata_demo fmt)
```

后一种写法把当前电脑的路径泄漏进项目，很难跨平台和复现。

---

## 9. `PRIVATE`、`PUBLIC`、`INTERFACE` 如何选择

先确定两个角色：

```text
生产者target：提供功能
消费者target：链接并使用生产者
```

判断规则不是“库重要不重要”，而是：

> 这个依赖是否出现在当前 target 的公开接口中？

### `PRIVATE`

只有当前 target 的实现使用该依赖。

例如 `app/main.cpp` 使用 `fmt`：

```cmake
target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    fmt::fmt
)
```

`tensor_metadata_demo` 是最终可执行程序，也没有下游消费者需要继承 fmt，因此使用 `PRIVATE`。

### `PUBLIC`

当前 target 自己使用，而且它的公开头文件也暴露该依赖。

假设公开头文件出现：

```cpp
#include <fmt/format.h>
```

那么下游消费者编译这个头文件时也需要 fmt，依赖可能应该是 `PUBLIC`。

### `INTERFACE`

当前 target 的实现不需要，只有它的消费者需要。

常见于：

```text
纯头文件库
构建配置target
警告选项target
```

今天的正确选择是：

```text
tensor_metadata核心库：不依赖fmt
tensor_metadata_demo：PRIVATE依赖fmt::fmt
```

这样能够保持核心库轻量。

---

## 10. 什么是 Debug 和 Release

它们不是两种 C++ 语言，而是同一套源码的两种构建配置。

### Debug

目标：方便开发和定位问题。

典型特点：

```text
保留调试信息
优化较少或不优化
变量和调用栈更容易观察
程序通常更大、更慢
断言通常启用
```

### Release

目标：性能和交付。

典型特点：

```text
启用优化
调试体验较差
程序行为可能经过重排和内联
通常定义NDEBUG
assert通常被移除
```

不要得出错误结论：

```text
Debug = 正确
Release = 只是更快
```

真实情况是：Release 优化可能暴露未定义行为、生命周期错误和数据竞争。两种配置都必须测试。

---

## 11. 四种常见构建配置

| 配置 | 主要用途 |
|---|---|
| `Debug` | 日常开发、断点调试 |
| `Release` | 性能和交付验证 |
| `RelWithDebInfo` | 优化同时保留较完整调试信息 |
| `MinSizeRel` | 优先减小二进制体积 |

目前重点掌握：

```text
Debug
Release
```

`RelWithDebInfo` 在服务器性能问题排查中也很常见，但今天只需要认识。

---

## 12. 单配置和多配置生成器

这是今天最容易混淆的地方。

### 单配置生成器

常见：

```text
Unix Makefiles
Ninja
```

配置时确定构建类型：

```bash
cmake \
  -S day27 \
  -B day31/build-debug \
  -DCMAKE_BUILD_TYPE=Debug
```

一个 build 目录只对应一种配置。

### 多配置生成器

常见：

```text
Xcode
Visual Studio
Ninja Multi-Config
```

它们通常在构建时选择配置：

```bash
cmake --build build --config Debug
cmake --build build --config Release
```

你当前 macOS 命令行默认通常使用 Unix Makefiles，属于单配置生成器，因此今天建立两个独立目录。

---

## 13. 不要在 `CMakeLists.txt` 中强行写死构建类型

不推荐：

```cmake
set(CMAKE_BUILD_TYPE Debug)
```

原因：

1. 项目作者替使用者做了决定；
2. 使用者很难切换 Release；
3. 对多配置生成器不适用；
4. 命令行传入的配置可能被覆盖。

推荐在配置命令中选择：

```bash
-DCMAKE_BUILD_TYPE=Debug
```

或者：

```bash
-DCMAKE_BUILD_TYPE=Release
```

---

## 14. 不要急着手写 `-O0` 和 `-O3`

CMake已经为常见构建配置准备了默认编译参数。

例如在 Clang 工具链中，Debug 和 Release 通常会获得不同的调试与优化选项。

初学阶段推荐：

```text
选择正确的CMAKE_BUILD_TYPE
让CMake和工具链提供默认配置
```

不推荐一上来写：

```cmake
target_compile_options(target PRIVATE -O3)
```

否则可能出现：

```text
Debug也被强制优化
不同编译器不兼容
选项互相覆盖
难以判断最终使用了什么参数
```

确实需要定制时，应使用生成器表达式并考虑不同编译器，但今天不把优化参数手工定制作为任务。

---

## 15. 使用生成器表达式区分配置

今天给演示程序定义一个配置宏：

```cmake
target_compile_definitions(
    tensor_metadata_demo
    PRIVATE
    "$<$<CONFIG:Debug>:TENSOR_DEMO_DEBUG=1>"
    "$<$<CONFIG:Release>:TENSOR_DEMO_RELEASE=1>"
)
```

其中：

```text
$<CONFIG:Debug>
```

表示判断当前配置是否为 Debug。

完整形式：

```text
$<条件:条件成立时产生的内容>
```

因此：

```text
$<$<CONFIG:Debug>:TENSOR_DEMO_DEBUG=1>
```

意思是：

```text
如果当前配置是Debug
就给该target定义TENSOR_DEMO_DEBUG=1
```

生成器表达式是在 CMake 生成构建系统时，根据具体配置计算的。

---

## 16. `NDEBUG` 和 `assert`

标准断言：

```cpp
#include <cassert>

assert(value > 0);
```

如果定义了：

```text
NDEBUG
```

那么标准 `assert()` 通常会被移除。

因此不能把业务错误处理只写成：

```cpp
assert(audio_path_exists);
```

Release 中它可能完全不执行。

正确分工：

```text
assert
    检查程序员认为绝不应该违反的内部条件

异常、optional、错误码
    处理文件不存在、模型加载失败、用户输入错误等运行时问题
```

今天不要求修改现有业务错误处理，只需要理解 Release 不能依赖 `assert()` 保证业务正确性。

---

## 17. Sanitizer 与构建类型的关系

你前面学过 ASan。它通常更适合开发和测试配置，而不是直接放进正式交付版本。

推荐思路：

```text
Debug + ASan
    定位内存错误

Release
    验证优化后的功能和性能
```

但 Debug 和 ASan 是两个不同维度：

```text
Debug
    构建配置

ASan
    编译器插桩工具
```

不能认为 Debug 自动等于启用 ASan。

> 超纲提示：后面可以使用 CMake 选项 `ENABLE_ASAN` 控制 Sanitizer。今天不要求扩展，避免同时引入过多配置。

---

## 18. 今天对 `CMakeLists.txt` 的修改

文件：

```text
day27/CMakeLists.txt
```

### 18.1 在 `project()` 后接入 fmt

加入：

```cmake
include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.2.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(fmt)
```

分类：

```text
include(FetchContent)
    加载CMake自带的FetchContent模块

FetchContent_Declare
    声明从哪里获得哪个版本

FetchContent_MakeAvailable
    获取依赖并把它加入当前构建
```

### 18.2 只给演示程序链接 fmt

找到：

```cmake
target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    tensor_metadata
    project_warnings
)
```

修改为：

```cmake
target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    tensor_metadata
    project_warnings
    fmt::fmt
)
```

为什么不给 `tensor_metadata` 链接 fmt？

```text
核心库没有使用fmt
只有app/main.cpp负责展示结果
不应该让核心库承担无用依赖
```

### 18.3 给演示程序添加配置宏

加入：

```cmake
target_compile_definitions(
    tensor_metadata_demo
    PRIVATE
    "$<$<CONFIG:Debug>:TENSOR_DEMO_DEBUG=1>"
    "$<$<CONFIG:Release>:TENSOR_DEMO_RELEASE=1>"
)
```

不要使用全局的：

```cmake
add_definitions(...)
```

因为我们只希望演示程序得到这些宏。

---

## 19. 修改演示程序使用 fmt

文件：

```text
day27/app/main.cpp
```

加入头文件：

```cpp
#include <fmt/format.h>
```

加入构建模式函数：

```cpp
[[nodiscard]] const char* build_mode() noexcept {
#if defined(TENSOR_DEMO_DEBUG)
    return "Debug";
#elif defined(TENSOR_DEMO_RELEASE)
    return "Release";
#else
    return "Unknown";
#endif
}
```

在 `main()` 中输出：

```cpp
std::cout
    << fmt::format(
        "Tensor metadata demo [{}]",
        build_mode()
    )
    << '\n';
```

这段代码一次验证两件事：

1. `fmt::fmt` 是否接入成功；
2. 当前构建配置宏是否正确。

> 超纲提示：这里的 `#if defined(...)` 是预处理条件编译。今天只要求会读和使用；预处理器的系统细节后续再展开。

---

## 20. 一个整理后的关键 CMake 片段

下面不是完整文件，而是今天新增和修改的关键位置：

```cmake
cmake_minimum_required(VERSION 3.20)

project(
    TensorMetaDataProject
    VERSION 1.0
    LANGUAGES CXX
)

include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.2.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(fmt)

# 中间保留原来的project_warnings和tensor_metadata配置。

add_executable(
    tensor_metadata_demo
    app/main.cpp
)

target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    tensor_metadata
    project_warnings
    fmt::fmt
)

target_compile_definitions(
    tensor_metadata_demo
    PRIVATE
    "$<$<CONFIG:Debug>:TENSOR_DEMO_DEBUG=1>"
    "$<$<CONFIG:Release>:TENSOR_DEMO_RELEASE=1>"
)
```

---

## 21. 配置 Debug 构建

在 `CppLearning` 根目录执行：

```bash
cmake \
  -S day27 \
  -B day31/build-debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON \
  2>&1 | tee day31/report/debug-configure.txt
```

第一次配置可能需要从 GitHub 下载 fmt，因此需要网络。

检查缓存中的配置：

```bash
cmake \
  -LA \
  -N \
  day31/build-debug \
  | grep CMAKE_BUILD_TYPE
```

预期：

```text
CMAKE_BUILD_TYPE:STRING=Debug
```

---

## 22. 构建并运行 Debug

构建：

```bash
cmake \
  --build day31/build-debug \
  --parallel \
  2>&1 | tee day31/report/debug-build.txt
```

运行：

```bash
./day31/build-debug/tensor_metadata_demo \
  2>&1 | tee day31/report/debug-run.txt
```

输出中应该包含：

```text
Tensor metadata demo [Debug]
```

运行测试：

```bash
ctest \
  --test-dir day31/build-debug \
  --output-on-failure
```

---

## 23. 配置 Release 构建

使用另一个目录：

```bash
cmake \
  -S day27 \
  -B day31/build-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=ON \
  2>&1 | tee day31/report/release-configure.txt
```

检查：

```bash
cmake \
  -LA \
  -N \
  day31/build-release \
  | grep CMAKE_BUILD_TYPE
```

预期：

```text
CMAKE_BUILD_TYPE:STRING=Release
```

---

## 24. 构建并运行 Release

构建：

```bash
cmake \
  --build day31/build-release \
  --parallel \
  2>&1 | tee day31/report/release-build.txt
```

运行：

```bash
./day31/build-release/tensor_metadata_demo \
  2>&1 | tee day31/report/release-run.txt
```

输出中应该包含：

```text
Tensor metadata demo [Release]
```

运行测试：

```bash
ctest \
  --test-dir day31/build-release \
  --output-on-failure
```

---

## 25. 为什么必须使用两个 build 目录

不要在同一个目录反复执行：

```bash
-DCMAKE_BUILD_TYPE=Debug
-DCMAKE_BUILD_TYPE=Release
```

虽然 CMake 可以重新配置，但这样容易：

```text
混淆旧产物
混淆日志
忘记当前配置
无法同时比较两个版本
调试器连接错误的程序
```

推荐：

```text
day31/build-debug
day31/build-release
```

两套目录互不影响，也可以同时保留。

---

## 26. 检查真正使用的编译命令

你已经设置：

```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

因此两个构建目录会生成：

```text
day31/build-debug/compile_commands.json
day31/build-release/compile_commands.json
```

检查演示程序：

```bash
grep -n \
  "TENSOR_DEMO_" \
  day31/build-debug/compile_commands.json
```

以及：

```bash
grep -n \
  "TENSOR_DEMO_" \
  day31/build-release/compile_commands.json
```

你应分别看到类似：

```text
TENSOR_DEMO_DEBUG=1
```

和：

```text
TENSOR_DEMO_RELEASE=1
```

还可以观察调试与优化参数，但不同编译器和 CMake 版本的具体选项可能不同，不要死记某个固定字符串。

---

## 27. 检查 fmt 的依赖边界

在编译命令数据库中查找 fmt：

```bash
grep -n \
  "fmt" \
  day31/build-debug/compile_commands.json
```

重点观察：

```text
app/main.cpp需要fmt头文件
核心src/*.cpp不应该因为业务需要而include fmt
tests/test_tensor_metadata.cpp也不需要fmt
```

这里说明：

```text
依赖被限制在真正使用它的target中
```

注意：FetchContent 把 fmt 加入同一构建，所以你可能在编译数据库中看到 fmt 自己的源码，这属于正常现象。

---

## 28. 比较两个可执行文件

查看文件信息：

```bash
file \
  day31/build-debug/tensor_metadata_demo \
  day31/build-release/tensor_metadata_demo
```

查看大小：

```bash
stat -f '%N %z bytes' \
  day31/build-debug/tensor_metadata_demo \
  day31/build-release/tensor_metadata_demo
```

不要机械地断言 Release 一定更小。

大小还受到以下因素影响：

```text
是否携带调试符号
静态库还是动态库
模板实例化
链接器行为
是否strip
架构
fmt的构建方式
```

正确做法是记录观察结果，再解释可能原因。

---

## 29. 一次运行两套测试

把结果写入同一个报告：

```bash
{
  echo '===== Debug ====='
  ctest \
    --test-dir day31/build-debug \
    --output-on-failure

  echo '===== Release ====='
  ctest \
    --test-dir day31/build-release \
    --output-on-failure
} 2>&1 | tee day31/report/test-results.txt
```

合格条件：

```text
Debug测试全部通过
Release测试全部通过
```

如果 Debug 通过但 Release 失败，不能简单归咎于编译器。应该重点检查：

```text
未定义行为
悬空引用或指针
未初始化变量
依赖assert产生副作用
违反严格别名规则
数据竞争
依赖求值顺序
```

---

## 30. `find_package()` 与 `FetchContent` 不是竞争关系

两者解决的阶段不同：

```text
find_package
    查找已经可供使用的包

FetchContent
    在配置阶段获得源码并加入构建
```

真实工程中常见组合：

```text
包管理器准备依赖
        ↓
find_package找到依赖
        ↓
target_link_libraries链接导入target
```

教学项目为了减少安装步骤，常使用：

```text
FetchContent
        ↓
依赖自己创建target
        ↓
target_link_libraries
```

最重要的共同点是：

> 最终都应该尽量链接现代 CMake target，而不是手写绝对路径。

---

## 31. 第三方依赖是否应该出现在头文件中

能不暴露就尽量不暴露。

假设核心公开头文件写了：

```cpp
#include <fmt/format.h>

class TensorMetadata {
public:
    fmt::memory_buffer format() const;
};
```

这会让 fmt 成为公开接口的一部分：

```text
所有消费者编译TensorMetadata头文件时都需要fmt
fmt版本变化可能影响消费者
核心库和第三方库耦合加深
```

今天只在：

```text
day27/app/main.cpp
```

使用 fmt，是更清晰的依赖边界。

AI 部署工程同样适用：

```text
业务核心接口尽量稳定
ONNX Runtime、TensorRT、Core ML细节放在后端实现层
```

---

## 32. 第三方依赖常见错误

### 错误1：只让编辑器找到头文件

VS Code 的 `includePath` 只影响代码分析，不等于真实构建已经接入依赖。

### 错误2：手写本机绝对路径

```cmake
/Users/你的名字/local/fmt/include
```

换一台电脑立即失效。

### 错误3：跟随 `main` 分支

依赖会变化，项目无法稳定复现。

### 错误4：所有依赖都写成 `PUBLIC`

会把内部实现依赖传播给不需要的消费者。

### 错误5：把依赖宏设成全局

```cmake
add_compile_definitions(...)
```

可能污染全部 target。

### 错误6：把第三方警告当成自己的错误

第三方源码可能触发不同警告。真实项目需要区分自有代码与第三方代码的警告策略。

### 错误7：删除版本号

“永远下载最新版”不是可复现构建。

### 错误8：网络失败就认为 CMake 写错

FetchContent 需要访问远程仓库。DNS、代理或证书错误也会导致配置失败。

### 错误9：提交 `_deps` 和 build 目录

FetchContent 下载内容一般位于构建目录内部，不应提交。

---

## 33. Debug/Release 常见错误

### 错误1：没有设置构建类型

单配置生成器下，空的 `CMAKE_BUILD_TYPE` 不等于标准 Debug。

### 错误2：同一个目录反复切换

容易混淆缓存和产物。

### 错误3：只测试 Debug

Release 优化后可能暴露问题。

### 错误4：使用 `#ifdef DEBUG`

`DEBUG` 不是 C++ 标准保证存在的宏。应定义项目自己的宏，或者针对断言检查 `NDEBUG`。

### 错误5：Release 中保留依赖 `assert()` 的业务逻辑

断言可能被移除。

### 错误6：认为 Release 不能调试

它仍然可以被调试，只是变量可能被优化掉、代码可能被内联，体验更困难。

### 错误7：把 `CMAKE_BUILD_TYPE` 写死在项目中

破坏使用者选择，也不适用于所有生成器。

### 错误8：在多配置生成器上只传 `CMAKE_BUILD_TYPE`

Xcode、Visual Studio 等通常需要构建时使用 `--config`。

---

## 34. 与 AI 端侧部署的关系

今天不是无关的构建练习。

以后接入 AI 推理 SDK 时会看到相同模式。

### ONNX Runtime

```text
头文件
    onnxruntime_cxx_api.h

库
    libonnxruntime.dylib或libonnxruntime.so

CMake
    找到SDK并链接推理target
```

### TensorRT

```text
头文件
    NvInfer.h

库
    nvinfer、nvonnxparser等

构建配置
    Debug用于接口调试
    Release用于真实性能测试
```

### Core ML 或 Apple 端侧框架

```text
Apple framework
模型文件
C++或Swift接口层
Debug调试
Release测量真实延迟和包体积
```

### FFmpeg

```text
多个头文件目录
多个动态或静态库
编解码组件依赖
平台差异
```

今天掌握的是以后接入这些大型 SDK 的基础方法。

---

## 35. 综合练习1：接入 fmt 并限制依赖边界

这是今天第一个综合练习，不再拆成大量零散小题。

### 修改文件

```text
day27/CMakeLists.txt
day27/app/main.cpp
```

### 任务要求

1. 使用 `FetchContent` 声明 fmt；
2. 固定版本为 `11.2.0`；
3. 让 CMake 获得 fmt；
4. 只给 `tensor_metadata_demo` 链接 `fmt::fmt`；
5. 使用 `PRIVATE`；
6. 在 `app/main.cpp` 中使用 `fmt::format()`；
7. 核心库头文件和源文件不得包含 fmt 头文件；
8. 测试代码不得为了完成任务而依赖 fmt；
9. 第一次配置成功后记录下载和配置结果；
10. 解释为什么 fmt 不应该成为 `tensor_metadata` 的公开依赖。

### 覆盖知识点

```text
第三方依赖
版本固定
FetchContent
逻辑target
PRIVATE依赖
依赖边界
编译与链接
```

### 验收方式

```bash
cmake \
  -S day27 \
  -B day31/build-debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON

cmake \
  --build day31/build-debug \
  --parallel

./day31/build-debug/tensor_metadata_demo
```

程序成功输出格式化文本即通过核心验收。

---

## 36. 综合练习2：建立 Debug/Release 双配置验证

### 修改文件

```text
day27/CMakeLists.txt
day27/app/main.cpp
```

### 输出文件

```text
day31/report/debug-configure.txt
day31/report/debug-build.txt
day31/report/debug-run.txt
day31/report/release-configure.txt
day31/report/release-build.txt
day31/report/release-run.txt
day31/report/test-results.txt
day31/report/dependency-and-config-report.md
```

### 任务要求

1. 使用生成器表达式定义两个配置宏；
2. 程序运行时输出当前构建模式；
3. 建立 `build-debug`；
4. 建立 `build-release`；
5. 两个目录分别构建；
6. 两个版本分别运行；
7. 两个版本分别运行 CTest；
8. 从 `compile_commands.json` 检查配置宏；
9. 比较两个可执行文件大小；
10. 写出为什么性能测试应该使用 Release；
11. 写出为什么内存安全检查通常在开发配置完成；
12. 写出为什么 Release 也必须执行测试。

### 覆盖知识点

```text
Debug
Release
CMAKE_BUILD_TYPE
生成器表达式
编译宏
独立构建目录
测试
可执行文件分析
```

---

## 37. 集中验收清单

### 第三方依赖

- [ ] 能解释第三方依赖不仅是头文件；
- [ ] 能区分头文件库与编译库；
- [ ] 能解释 `find_package()` 的前提；
- [ ] 能解释 `FetchContent` 的作用；
- [ ] 能说明第一次配置为什么需要网络；
- [ ] fmt 版本固定为 `11.2.0`；
- [ ] 没有跟随 `main` 分支；
- [ ] 没有手写个人电脑绝对路径；
- [ ] 使用 `fmt::fmt` target；
- [ ] 使用 `PRIVATE` 链接给 demo；
- [ ] 核心库没有无意义地依赖 fmt；
- [ ] 测试 target 没有无意义地依赖 fmt。

### Debug

- [ ] 使用 `day31/build-debug`；
- [ ] `CMAKE_BUILD_TYPE` 是 Debug；
- [ ] Debug 构建成功；
- [ ] 程序输出 `[Debug]`；
- [ ] Debug 测试全部通过；
- [ ] 编译命令包含 `TENSOR_DEMO_DEBUG=1`。

### Release

- [ ] 使用 `day31/build-release`；
- [ ] `CMAKE_BUILD_TYPE` 是 Release；
- [ ] Release 构建成功；
- [ ] 程序输出 `[Release]`；
- [ ] Release 测试全部通过；
- [ ] 编译命令包含 `TENSOR_DEMO_RELEASE=1`。

### 理解

- [ ] 能解释为什么两个配置使用不同 build 目录；
- [ ] 能解释为什么不写死 `CMAKE_BUILD_TYPE`；
- [ ] 能解释为什么不直接写 `-O3`；
- [ ] 能解释 `NDEBUG` 对 `assert()` 的影响；
- [ ] 能解释 Debug 和 ASan 不是同一个概念；
- [ ] 能解释 Release 也必须测试；
- [ ] 能解释依赖的公开接口与实现细节区别；
- [ ] 完成构建配置报告。

---

## 38. 报告模板

创建文件：

```text
day31/report/dependency-and-config-report.md
```

复制并填写：

```markdown
# 第三方依赖与构建配置报告

## 1. 环境

- 操作系统：
- 处理器架构：
- CMake版本：
- 编译器版本：
- CMake生成器：

## 2. 第三方依赖

- 依赖名称：fmt
- 固定版本：11.2.0
- 接入方式：FetchContent
- 依赖消费者：tensor_metadata_demo
- 链接范围：PRIVATE
- 核心库是否依赖fmt：
- 为什么这样划分：

## 3. Debug构建

- 构建目录：day31/build-debug
- CMAKE_BUILD_TYPE：
- 程序报告的模式：
- 测试结果：
- 可执行文件大小：
- 编译命令中的配置宏：

## 4. Release构建

- 构建目录：day31/build-release
- CMAKE_BUILD_TYPE：
- 程序报告的模式：
- 测试结果：
- 可执行文件大小：
- 编译命令中的配置宏：

## 5. 差异解释

- Debug主要用途：
- Release主要用途：
- 为什么Release也需要测试：
- 为什么不能依赖assert处理业务错误：
- 为什么不在CMakeLists中写死构建类型：

## 6. AI部署联系

- ONNX Runtime接入与今天内容的联系：
- TensorRT接入与今天内容的联系：
- 为什么真实性能测试应使用Release：

## 7. 遇到的问题

- 问题：
- 错误阶段：配置期 / 编译期 / 链接期 / 运行期
- 原因：
- 修复：
```

---

## 39. 复盘问题

完成代码后，独立回答：

1. 第三方依赖和自己项目源码有什么区别？
2. 为什么找到头文件不代表接入完成？
3. 编译错误和链接错误在依赖问题中分别说明什么？
4. 头文件库是否完全不需要 CMake target？为什么？
5. `find_package()` 查找的是什么？
6. `FetchContent` 在什么阶段获得依赖？
7. `FetchContent_MakeAvailable()` 做了什么？
8. 为什么不使用 `GIT_TAG main`？
9. 为什么固定依赖版本有利于复现？
10. `fmt::fmt` 是路径还是 target？
11. target 能携带哪些使用要求？
12. 为什么不推荐手写 `include_directories()`？
13. 为什么不推荐手写 `link_directories()`？
14. 当前项目为什么对 fmt 使用 `PRIVATE`？
15. 什么情况下 fmt 可能需要成为 `PUBLIC` 依赖？
16. `INTERFACE` 依赖表达什么？
17. 为什么核心库应避免不必要的第三方依赖？
18. Debug 的主要目标是什么？
19. Release 的主要目标是什么？
20. Release 为什么可能暴露 Debug 没出现的问题？
21. 什么是单配置生成器？
22. 什么是多配置生成器？
23. 当前为什么建立两个 build 目录？
24. 为什么不在 `CMakeLists.txt` 中写死 Debug？
25. 为什么今天不直接手写 `-O0` 和 `-O3`？
26. `$<CONFIG:Debug>` 判断什么？
27. `target_compile_definitions()` 设置在哪里生效？
28. `NDEBUG` 对标准 `assert()` 有什么影响？
29. 为什么业务错误处理不能只依赖 `assert()`？
30. Debug 是否自动启用 ASan？
31. ASan 和 Debug 分别是什么维度？
32. 为什么 Debug 和 Release 都必须运行测试？
33. `compile_commands.json` 能帮助检查什么？
34. FetchContent 网络失败属于哪一个阶段？
35. 为什么不能提交构建目录中的 `_deps`？
36. ONNX Runtime 接入为什么会遇到相似问题？
37. TensorRT 的性能测试为什么应该使用 Release？
38. 如果将来依赖很多，为什么要考虑包管理器？

---

## 40. 今天不要扩展的内容

为控制学习范围，今天暂时不要做：

```text
同时安装Conan和vcpkg
编写自定义Find模块
编写install/export规则
给fmt打补丁
手工修改fmt源码
设置复杂交叉编译工具链
自行覆盖全部优化参数
做正式性能基准测试
接入ONNX Runtime或TensorRT
```

这些内容不是没用，而是应该建立在今天的依赖 target 和构建配置基础之上。

---

## 41. 今天的最小完成成果

如果时间有限，至少完成：

```text
1. 在day27/CMakeLists.txt中使用FetchContent接入fmt 11.2.0
2. 只让tensor_metadata_demo PRIVATE链接fmt::fmt
3. 在day27/app/main.cpp中调用fmt::format
4. 使用生成器表达式定义Debug和Release宏
5. 建立day31/build-debug
6. 建立day31/build-release
7. 两个版本均构建成功
8. 程序分别输出Debug和Release
9. 两套CTest全部通过
10. 完成dependency-and-config-report.md
```

完成这些，你就真正掌握了：

```text
第三方依赖声明
        +
现代CMake target连接
        +
依赖传播边界
        +
Debug/Release双配置
        +
构建与测试验证
```

这套方法会直接复用于后面的 GoogleTest、ONNX Runtime、FFmpeg、CUDA 和 TensorRT 接入。
