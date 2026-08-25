# 第7周·工作日1：掌握 target、include目录和编译选项的现代CMake写法

## 0. 今天要解决的问题

你前面的项目主要依靠手写命令编译：

```bash
clang++ \
  -std=c++17 \
  -Wall \
  -Wextra \
  -Wpedantic \
  src/main.cpp \
  src/tensor_shape.cpp \
  -I include \
  -o target/app
```

项目文件变多以后，手写命令会出现这些问题：

- 容易漏掉某个 `.cpp`；
- include路径到处重复；
- 每个命令的C++标准可能不同；
- 添加第三方库后链接参数难以维护；
- 测试程序和正式程序重复写相同参数；
- `clang-tidy`很难获得准确的编译信息；
- 换电脑或换编译器时需要重新拼命令。

今天要把编译规则写成目标导向的现代CMake：

```text
源文件
   ↓
library target
   ↓ 公开自己的使用要求
executable target
   ↓ 链接库并继承需要的配置
最终程序
```

今天完成后，你应该能够：

1. 解释CMake中的 target是什么；
2. 区分源代码目录、头文件目录、构建目录和安装目录；
3. 使用 `add_library()`和 `add_executable()`创建目标；
4. 使用 `target_link_libraries()`表达目标依赖；
5. 使用 `target_include_directories()`配置头文件搜索路径；
6. 正确选择 `PRIVATE`、`PUBLIC`和 `INTERFACE`；
7. 使用 `target_compile_features()`声明C++17要求；
8. 使用 `target_compile_options()`添加目标级警告；
9. 使用生成器表达式区分Clang/GCC和MSVC；
10. 完成配置、构建、测试和查看真实编译命令；
11. 生成 `compile_commands.json`供 `clang-tidy`使用；
12. 避免 `include_directories()`、全局flags和源码内构建等旧式写法。

---

## 1. 今天继续采用综合练习

今天只安排两个综合练习和一个集中验收：

```text
综合练习1
    为day27张量元数据模块建立library、demo和test三个target

综合练习2
    分析PRIVATE/PUBLIC/INTERFACE的传播，并加入目标级编译选项

集中验收
    配置、构建、运行、CTest、clang-tidy和报告
```

不会创建十几个只演示一条命令的小项目。

---

## 1.1 先按功能给今天出现的CMake命令分类

前面的版本把函数按项目出现顺序讲解，容易让“创建target、设置target、连接target”混在一起。从这里开始先建立分类。

### 第一类：声明项目

```cmake
cmake_minimum_required(VERSION 3.20)
project(TensorMetadataProject LANGUAGES CXX)
```

作用：

```text
确定最低CMake版本
声明项目名称
启用C++语言
```

### 第二类：创建target

```cmake
add_library(...)
add_executable(...)
```

作用：

```text
创建库target
创建可执行程序target
```

这一步回答：

> 项目里需要构建哪些东西？

### 第三类：设置当前target如何编译

```cmake
target_include_directories(...)
target_compile_features(...)
target_compile_options(...)
set_target_properties(...)
```

分别负责：

```text
target_include_directories
    头文件去哪里找

target_compile_features
    需要什么C++语言能力，例如C++17

target_compile_options
    给编译器添加哪些选项，例如-Wall

set_target_properties
    设置target的其他属性，例如是否允许编译器扩展语法
```

这一步回答：

> 这个target应该用什么规则编译？

### 第四类：连接target之间的依赖

```cmake
target_link_libraries(...)
```

作用：

```text
声明一个target依赖哪些库或配置target
```

这一步回答：

> 这个target需要使用谁？

### 第五类：测试

```cmake
include(CTest)
add_test(...)
```

作用：

```text
启用CTest
把测试可执行程序注册为可运行测试
```

### 第六类：项目级辅助配置

```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(...)
```

作用：

```text
生成compile_commands.json
定义用户可以打开或关闭的构建选项
```

### 今天的学习优先级

必须掌握：

```text
cmake_minimum_required
project
add_library
add_executable
target_include_directories
target_compile_features
target_compile_options
target_link_libraries
```

理解即可：

```text
set_target_properties
include(CTest)
add_test
CMAKE_EXPORT_COMPILE_COMMANDS
```

进阶内容，今天可以暂时跳过：

```text
INTERFACE警告库
生成器表达式
BUILD_INTERFACE / INSTALL_INTERFACE
Sanitizer配置target
动态库安装和rpath
```

第一次完成练习时，应先写最小版本；最小版本成功后，再增加进阶配置。

---

## 1.2 今天的最小CMakeLists先只保留核心函数

第一次练习先写：

```cmake
cmake_minimum_required(VERSION 3.20)

project(
    TensorMetadataProject
    LANGUAGES CXX
)

add_library(tensor_metadata STATIC
    src/data_type.cpp
    src/device.cpp
    src/tensor_shape.cpp
    src/tensor_metadata.cpp
)

target_include_directories(
    tensor_metadata
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(
    tensor_metadata
    PUBLIC
    cxx_std_17
)

target_compile_options(
    tensor_metadata
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(
    tensor_metadata_demo
    app/main.cpp
)

target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    tensor_metadata
)
```

先确认这份最小配置能够构建，再学习：

```text
project_warnings接口库
跨编译器选项
CTest
clang-tidy
Sanitizer
```

不要第一次就把所有扩展配置堆进同一个CMakeLists。

---

## 2. 今日目录结构

今天不复制 `day27`的实现，而是给它添加构建入口，并在 `day29`保存学习产出。

建议结构：

```text
CppLearning/
├── day27/
│   ├── CMakeLists.txt                 # 今天创建
│   ├── include/
│   │   ├── data_type.hpp
│   │   ├── device.hpp
│   │   ├── metadata_error.hpp
│   │   ├── tensor_layout.hpp
│   │   ├── tensor_metadata.hpp
│   │   └── tensor_shape.hpp
│   ├── src/
│   │   ├── data_type.cpp
│   │   ├── device.cpp
│   │   ├── tensor_metadata.cpp
│   │   └── tensor_shape.cpp
│   ├── app/
│   │   └── main.cpp                   # 今天创建
│   ├── tests/
│   │   └── test_tensor_metadata.cpp   # 今天创建
│   └── build/                         # CMake生成，不提交
└── day29/
    ├── examples/
    │   └── interface_library/
    │       └── CMakeLists.txt
    ├── report/
    │   ├── configure-output.txt
    │   ├── build-output.txt
    │   ├── test-output.txt
    │   ├── verbose-build.txt
    │   └── cmake-learning-report.md
    └── 第7周_工作日1_target_include目录与编译选项的现代CMake写法.md
```

今天主要修改或创建的文件：

```text
day27/CMakeLists.txt
day27/app/main.cpp
day27/tests/test_tensor_metadata.cpp
day29/examples/interface_library/CMakeLists.txt
day29/report/cmake-learning-report.md
```

---

## 3. 当前环境

当前机器已经安装：

```text
CMake 4.4.0
```

今天的项目不需要使用CMake 4.x专属语法。为了让项目能在更多开发环境构建，建议最低版本写：

```cmake
cmake_minimum_required(VERSION 3.20)
```

它表达的是：

> 这个项目要求CMake至少为3.20，并采用相应策略行为。

不要因为本机是4.4就机械写：

```cmake
cmake_minimum_required(VERSION 4.4)
```

否则会无必要地排除大量仍在使用CMake 3.x的环境。

---

## 4. CMake不是编译器

CMake负责生成构建系统，但它本身不编译C++源码。

关系大致是：

```text
CMakeLists.txt
      ↓ cmake配置
构建系统（Unix Makefiles或Ninja等）
      ↓ cmake --build
clang++ / g++真正编译和链接
      ↓
可执行文件或库
```

因此：

- CMake语法错误发生在配置阶段；
- C++语法错误发生在编译阶段；
- 缺少实现通常发生在链接阶段；
- 测试失败发生在运行阶段。

不要把所有失败都称为“CMake报错”。

---

## 5. 什么是target

target是CMake构建图中的一个命名节点。

常见target包括：

### 可执行目标

```cmake
add_executable(tensor_metadata_demo
    app/main.cpp
)
```

这里：

```text
target名称：tensor_metadata_demo
目标类型：可执行程序
源文件：app/main.cpp
```

### 库目标

```cmake
add_library(tensor_metadata
    src/data_type.cpp
    src/device.cpp
    src/tensor_shape.cpp
    src/tensor_metadata.cpp
)
```

这里：

```text
target名称：tensor_metadata
目标类型：库
```

### 接口库目标

```cmake
add_library(project_warnings INTERFACE)
```

接口库通常不编译自己的 `.cpp`，它可以用于集中传播配置。

target不是：

- `target/`输出目录；
- C++类；
- 单个 `.cpp`文件；
- CMake变量。

你的旧目录名称 `target/`与CMake术语target只是名字相同，含义不同。

---

## 6. 为什么现代CMake围绕target组织

一个target可以拥有自己的：

```text
源文件
头文件搜索路径
C++标准要求
编译选项
宏定义
链接库
链接选项
测试关系
```

例如：

```cmake
target_include_directories(tensor_metadata ...)
target_compile_features(tensor_metadata ...)
target_compile_options(tensor_metadata ...)
target_link_libraries(tensor_metadata_demo ...)
```

每条命令都明确说明配置属于哪个目标。

这比全局命令更安全：

```cmake
include_directories(include)
add_compile_options(-Wall)
```

因为全局配置会隐式影响当前目录和子目录中的多个目标，项目变大后很难判断来源。

现代CMake的核心思想是：

> target声明自己的构建要求和使用要求，依赖关系负责传播需要的部分。

---

## 7. 最小CMakeLists结构

```cmake
cmake_minimum_required(VERSION 3.20)

project(
    TensorMetadataProject
    VERSION 0.1.0
    LANGUAGES CXX
)

add_library(tensor_metadata
    src/data_type.cpp
    src/device.cpp
    src/tensor_shape.cpp
    src/tensor_metadata.cpp
)

add_executable(tensor_metadata_demo
    app/main.cpp
)

target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    tensor_metadata
)
```

这里形成构建图：

```text
data_type.cpp ───────────────┐
device.cpp ──────────────────┤
tensor_shape.cpp ────────────┼─> tensor_metadata库
tensor_metadata.cpp ─────────┘             │
                                           │ PRIVATE链接
main.cpp ──────────────────────────────────┴─> tensor_metadata_demo
```

---

## 8. `project()`做什么

```cmake
project(
    TensorMetadataProject
    VERSION 0.1.0
    LANGUAGES CXX
)
```

含义：

- 项目名称是 `TensorMetadataProject`；
- 项目版本为 `0.1.0`；
- 只启用C++语言；
- CMake会探测可用的C++编译器。

因为今天没有C源文件，不必写：

```cmake
LANGUAGES C CXX
```

只启用需要的语言能减少无关探测。

---

## 9. `add_library()`中的库类型

### 不明确指定

```cmake
add_library(tensor_metadata
    src/tensor_shape.cpp
)
```

最终是静态库还是动态库可能受到：

```cmake
BUILD_SHARED_LIBS
```

影响。

### 明确静态库

```cmake
add_library(tensor_metadata STATIC
    src/tensor_shape.cpp
)
```

macOS上通常生成类似：

```text
libtensor_metadata.a
```

### 明确动态库

```cmake
add_library(tensor_metadata SHARED
    src/tensor_shape.cpp
)
```

macOS上通常生成类似：

```text
libtensor_metadata.dylib
```

今天建议使用：

```cmake
add_library(tensor_metadata STATIC ...)
```

让结果明确，暂时不学习动态库导出符号等额外问题。

---

## 10. 头文件为什么不一定要写进 `add_library()`

真正需要编译的是 `.cpp`：

```cmake
add_library(tensor_metadata STATIC
    src/data_type.cpp
    src/device.cpp
    src/tensor_shape.cpp
    src/tensor_metadata.cpp
)
```

头文件通过 `#include`进入翻译单元，因此不写在源文件列表中也能编译。

但是为了IDE展示和项目清晰，也可以列出项目头文件：

```cmake
add_library(tensor_metadata STATIC
    src/data_type.cpp
    src/device.cpp
    src/tensor_shape.cpp
    src/tensor_metadata.cpp
    include/data_type.hpp
    include/device.hpp
    include/tensor_layout.hpp
    include/tensor_shape.hpp
    include/tensor_metadata.hpp
)
```

今天两种方式都可以。

不要使用：

```cmake
file(GLOB SOURCES "src/*.cpp")
```

作为初学项目的默认方案，因为新增或删除文件时构建系统的重新配置行为不够直观，而且源文件集合不明确。显式列出更适合学习和代码审查。

---

## 11. include目录解决什么问题

源文件中写：

```cpp
#include "tensor_metadata.hpp"
```

编译器需要知道去哪里找它。

手写命令使用：

```bash
-I day27/include
```

CMake的target写法是：

```cmake
target_include_directories(
    tensor_metadata
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

其中：

```text
CMAKE_CURRENT_SOURCE_DIR
    当前CMakeLists.txt所在源码目录

${CMAKE_CURRENT_SOURCE_DIR}/include
    day27/include的绝对路径
```

不要依赖构建命令恰好从哪个工作目录执行。

---

## 11.1 什么是“消费者”

这里的“消费者”不是购买产品的人，也不是程序最终面对的用户。

在CMake语境中：

> 如果target B依赖并使用target A，那么B就是A的消费者。

例如先创建库target：

```cmake
add_library(tensor_metadata STATIC
    src/data_type.cpp
    src/device.cpp
    src/tensor_shape.cpp
    src/tensor_metadata.cpp
)
```

再创建Demo target：

```cmake
add_executable(tensor_metadata_demo
    app/main.cpp
)
```

让Demo链接这个库：

```cmake
target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    tensor_metadata
)
```

依赖关系是：

```text
tensor_metadata_demo
        │
        │ 使用、依赖
        ▼
tensor_metadata
```

因此：

```text
tensor_metadata
    被使用的target，也可以称为被依赖target

tensor_metadata_demo
    使用库的target，是tensor_metadata的消费者
```

测试程序也链接这个库：

```cmake
target_link_libraries(
    tensor_metadata_tests
    PRIVATE
    tensor_metadata
)
```

所以：

```text
tensor_metadata_demo  ──┐
                        ├── 都是tensor_metadata的消费者
tensor_metadata_tests ──┘
```

“消费者”只是为了方便描述依赖方向。你也可以把它读成：

```text
依赖当前库的其他target
```

### 消费者在C++代码中做了什么

Demo中的代码可能写：

```cpp
#include "tensor_metadata.hpp"

int main() {
    TensorMetadata metadata(...);
}
```

它要成功构建，需要两类东西：

```text
编译main.cpp时
    需要知道tensor_metadata.hpp在哪里

链接最终程序时
    需要tensor_metadata库中的函数实现
```

所以库target要告诉消费者：

```text
我的公开头文件目录在哪里
使用我的公开头文件需要什么C++标准
我还公开依赖哪些其他库
```

这些信息叫作：

```text
使用要求（usage requirements）
```

### “当前target”又是谁

当你写：

```cmake
target_include_directories(
    tensor_metadata
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

命令中的第一个参数：

```text
tensor_metadata
```

就是当前正在配置的target。

因此这句话里的两个角色是：

```text
当前target
    tensor_metadata

它的消费者
    tensor_metadata_demo
    tensor_metadata_tests
    以及未来其他链接tensor_metadata的target
```

只有先确定这两个角色，才能判断应该写 `PRIVATE`、`PUBLIC`还是 `INTERFACE`。

### 消费者不是自动产生的

仅仅创建两个target：

```cmake
add_library(tensor_metadata STATIC ...)
add_executable(tensor_metadata_demo app/main.cpp)
```

它们之间还没有依赖关系。

必须写：

```cmake
target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    tensor_metadata
)
```

这时Demo才成为这个库的消费者，并获得库公开传播的使用要求。

### 消费关系可以形成多层

以后可能有：

```text
tensor_metadata
        ↑
inference_service
        ↑
audio_server
```

这里：

```text
inference_service是tensor_metadata的直接消费者
audio_server是inference_service的消费者
```

如果 `inference_service`把对 `tensor_metadata`的依赖声明为公开依赖，那么相关使用要求还可能继续传播给 `audio_server`。

---

## 12. `PRIVATE / PUBLIC / INTERFACE`的核心含义

这是今天最重要的知识点。

可以从两个问题判断：

```text
1. 当前target自己编译时需要吗？
2. 链接当前target的消费者也需要吗？
```

对应关系：

|关键字|当前target自己使用|依赖它的其他target继承|
|---|---:|---:|
|`PRIVATE`|是|否|
|`PUBLIC`|是|是|
|`INTERFACE`|否|是|

把“消费者”换成完整句子后：

```text
PRIVATE
    只给当前target自己使用
    不传给依赖当前target的其他target

PUBLIC
    当前target自己使用
    也传给依赖当前target的其他target

INTERFACE
    当前target自己不使用
    只传给依赖当前target的其他target
```

以include目录为例：

```text
PRIVATE include
    tensor_metadata自己的.cpp能找到
    demo和tests不能继承

PUBLIC include
    tensor_metadata自己的.cpp能找到
    demo和tests也能继承

INTERFACE include
    tensor_metadata自己的.cpp不能使用这项配置
    demo和tests可以继承
```


---

## 13. include目录什么时候用 `PUBLIC`

`tensor_metadata`的公开头文件位于：

```text
day27/include
```

库自己的 `.cpp`需要它：

```cpp
#include "tensor_metadata.hpp"
```

消费者 `main.cpp`也需要它：

```cpp
#include "tensor_metadata.hpp"
```

所以两边都需要，应该使用：

```cmake
target_include_directories(
    tensor_metadata
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

当demo链接：

```cmake
target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    tensor_metadata
)
```

demo会自动继承库公开的include目录，不需要再次写：

```cmake
target_include_directories(
    tensor_metadata_demo
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

这就是使用要求传播。

---

## 14. include目录什么时候用 `PRIVATE`

假设库有内部实现头文件：

```text
day27/src/detail/checked_math.hpp
```

只有库自己的 `.cpp`会包含：

```cpp
#include "detail/checked_math.hpp"
```

消费者不应该使用它。

可以写：

```cmake
target_include_directories(
    tensor_metadata
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
```

这条路径不会传播给demo和tests。

---

## 15. include目录什么时候用 `INTERFACE`

如果一个库只有头文件，没有自己的 `.cpp`：

```cmake
add_library(tensor_helpers INTERFACE)

target_include_directories(
    tensor_helpers
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

接口库自己没有编译步骤，因此：

```text
当前target自己使用：否
消费者使用：是
```

所以只能使用 `INTERFACE`传播要求。

普通编译库不能因为“这是接口”就随便把公开include写成 `INTERFACE`。如果库自己的 `.cpp`也需要该路径，使用 `PUBLIC`。

---

## 16. 用一个判断表选择作用域

|场景|应该使用|
|---|---|
|只在当前库 `.cpp`中使用的内部头目录|`PRIVATE`|
|当前库和链接它的用户都要包含的公开头目录|`PUBLIC`|
|纯头文件接口库给消费者使用的目录|`INTERFACE`|
|当前可执行程序自己需要的目录|通常 `PRIVATE`|

最常见错误是看到“库的头文件”就固定写 `PUBLIC`。真正判断依据是：

> 这个要求是否出现在公开接口中，消费者编译时是否也需要。

---

## 17. 更正规的公开include写法

如果以后需要安装库，可以写：

```cmake
target_include_directories(
    tensor_metadata
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
```

含义：

```text
在源码构建期间
    使用当前项目的include目录

安装后给消费者使用
    使用安装前缀下的include目录
```

今天只要求理解。当前项目没有实现 `install()`，可以先使用简单版：

```cmake
target_include_directories(
    tensor_metadata
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

不要为了“现代”堆上尚未使用的安装配置。

---

## 18. include写法与目录布局

当前day27头文件直接位于：

```text
include/tensor_metadata.hpp
```

因此使用：

```cpp
#include "tensor_metadata.hpp"
```

未来更适合作为可复用库的布局是：

```text
include/cpp_learning/tensor_metadata.hpp
```

消费者写：

```cpp
#include <cpp_learning/tensor_metadata.hpp>
```

优点：

- 减少不同库同名头文件冲突；
- 清楚表明头文件属于哪个库；
- 更适合安装和发布。

今天不强制移动day27已有文件，先学会正确配置target；后续项目工程化时再调整公共头文件命名空间目录。

---

## 19. 用 `target_compile_features()`声明C++标准

推荐：

```cmake
target_compile_features(
    tensor_metadata
    PUBLIC
    cxx_std_17
)
```

它表示：

> 编译和使用 `tensor_metadata`至少需要C++17。

为什么用 `PUBLIC`？

因为公开头文件中使用了：

```cpp
std::optional
std::string_view
[[nodiscard]]
```

消费者包含这些头文件时也必须使用C++17。

因此C++17是库的公开使用要求。

如果只有 `.cpp`内部使用C++17，而公开头文件不需要，可以考虑 `PRIVATE`，但当前项目不是这种情况。

---

## 20. `cxx_std_17`和 `-std=c++17`的区别

手写编译选项：

```cmake
target_compile_options(
    tensor_metadata
    PRIVATE
    -std=c++17
)
```

不推荐，因为：

- `-std=c++17`是Clang/GCC风格；
- MSVC使用不同选项；
- 它没有清楚表达语言特性要求；
- 不方便正确传播给消费者。

推荐：

```cmake
target_compile_features(
    tensor_metadata
    PUBLIC
    cxx_std_17
)
```

CMake根据当前编译器选择合适选项。

---

## 21. 是否还需要 `CMAKE_CXX_STANDARD`

下面写法合法而且常见：

```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
```

它会影响之后创建的多个目标。

今天重点练习target级表达，因此优先使用：

```cmake
target_compile_features(
    tensor_metadata
    PUBLIC
    cxx_std_17
)

set_target_properties(
    tensor_metadata
    PROPERTIES
    CXX_EXTENSIONS OFF
)
```

demo和test从库继承C++17要求；也可以对独立目标显式声明，增强可读性。

二者不是“一定有一个错误”，区别在作用范围和表达方式。

### `set_target_properties()`到底是什么

```cmake
set_target_properties(
    tensor_metadata
    PROPERTIES
    CXX_EXTENSIONS OFF
)
```

可以按以下结构阅读：

```text
set_target_properties
    给target设置属性

tensor_metadata
    要设置的target名称

PROPERTIES
    后面开始列出“属性名称 属性值”

CXX_EXTENSIONS OFF
    把CXX_EXTENSIONS属性设为OFF
```

通用形式：

```cmake
set_target_properties(
    target名称
    PROPERTIES
    属性1 值1
    属性2 值2
)
```

它不是只用来设置C++版本，而是一个通用的target属性设置函数。

### `CXX_EXTENSIONS`是什么

C++编译器除了标准C++语法，还可能提供自己的扩展语法。

对于Clang和GCC，语言模式可以粗略分为：

```text
-std=c++17
    使用标准C++17模式

-std=gnu++17
    使用C++17，并允许GNU扩展
```

设置：

```cmake
CXX_EXTENSIONS OFF
```

表示：

> 不希望这个target依赖编译器特有扩展，尽量使用标准C++模式。

因此两段配置分工不同：

```cmake
target_compile_features(
    tensor_metadata
    PUBLIC
    cxx_std_17
)
```

表示：

```text
至少需要C++17能力
```

而：

```cmake
set_target_properties(
    tensor_metadata
    PROPERTIES
    CXX_EXTENSIONS OFF
)
```

表示：

```text
使用C++17时，不要依赖编译器私有扩展模式
```

合起来可以理解为：

```text
语言版本：C++17
语言方言：尽量使用标准C++，关闭编译器扩展
```

### 它为什么不是 `target_compile_features()`的一部分

因为两者描述不同维度：

```text
cxx_std_17
    需要哪个版本的语言能力

CXX_EXTENSIONS OFF
    是否允许编译器扩展方言
```

### 今天必须写吗

不是必须。

初次练习可以先只写：

```cmake
target_compile_features(
    tensor_metadata
    PUBLIC
    cxx_std_17
)
```

构建成功并理解target传播后，再补：

```cmake
set_target_properties(
    tensor_metadata
    PROPERTIES
    CXX_EXTENSIONS OFF
)
```

它属于可移植性增强配置，不应该阻碍你先掌握核心CMake流程。

### 等价的单属性写法

也可以写：

```cmake
set_property(
    TARGET tensor_metadata
    PROPERTY CXX_EXTENSIONS OFF
)
```

两种写法都可以。当前文档使用 `set_target_properties()`，因为后面可以一次设置多个target属性。

---

## 22. 编译特性、编译选项和宏定义不是一回事

### 编译特性

```cmake
target_compile_features(... cxx_std_17)
```

表达需要的语言能力。

### 编译选项

```cmake
target_compile_options(... -Wall)
```

传给编译器的命令行选项。

### 编译宏定义

```cmake
target_compile_definitions(
    tensor_metadata
    PRIVATE
    TENSOR_METADATA_DEBUG=1
)
```

相当于编译命令中的：

```bash
-DTENSOR_METADATA_DEBUG=1
```

不要把宏写成：

```cmake
target_compile_options(... -DTENSOR_METADATA_DEBUG=1)
```

虽然某些编译器能够接受，但CMake已经有表达语义更准确的专用命令。

---

## 23. 目标级编译警告

对Clang和GCC：

```cmake
target_compile_options(
    tensor_metadata
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
```

为什么使用 `PRIVATE`？

因为警告策略是当前项目构建策略，通常不应该强制传播给链接这个库的外部消费者。

如果写 `PUBLIC`：

```text
你的库要求消费者也使用-Wall -Wextra -Wpedantic
```

这可能给下游项目带来意外告警。

---

## 24. 跨编译器编译选项

MSVC不认识：

```text
-Wall -Wextra -Wpedantic
```

可以使用生成器表达式：

```cmake
target_compile_options(
    tensor_metadata
    PRIVATE
    $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:
        -Wall
        -Wextra
        -Wpedantic
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
    >
)
```

概念：

```text
如果C++编译器ID属于AppleClang、Clang或GNU
    添加-Wall -Wextra -Wpedantic

如果是MSVC
    添加/W4
```

今天不要求掌握所有生成器表达式语法，但要能读懂条件配置。

---

## 25. 为什么不推荐修改 `CMAKE_CXX_FLAGS`

旧式写法：

```cmake
set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wall -Wextra"
)
```

问题：

- 全局影响所有目标；
- 字符串拼接容易出错；
- 跨编译器困难；
- 传播关系不明确；
- 外部依赖也可能受到影响；
- 不便为不同target设置不同规则。

现代写法：

```cmake
target_compile_options(
    tensor_metadata
    PRIVATE
    -Wall
    -Wextra
)
```

配置明确属于 `tensor_metadata`。

---

## 26. `target_link_libraries()`表达依赖

```cmake
target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    tensor_metadata
)
```

这里不只是“添加一个库文件”。它还会传播 `tensor_metadata`的公开使用要求：

```text
PUBLIC include目录
PUBLIC C++17要求
其他PUBLIC/INTERFACE依赖
```

所以demo不需要重复设置库公开的include目录和语言要求。

`PRIVATE`表示：

```text
demo自己依赖tensor_metadata
但demo不需要再把这个依赖传播给别人
```

可执行程序一般没有其他链接消费者，因此通常使用 `PRIVATE`。

---

## 27. 依赖传播示例

```cmake
add_library(tensor_metadata STATIC ...)

target_include_directories(
    tensor_metadata
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(
    tensor_metadata
    PUBLIC
    cxx_std_17
)

add_executable(tensor_metadata_demo
    app/main.cpp
)

target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    tensor_metadata
)
```

传播关系：

```text
tensor_metadata
    PUBLIC include目录 ─────────┐
    PUBLIC cxx_std_17 ──────────┤
                               ↓
                    tensor_metadata_demo
```

但库的：

```cmake
target_compile_options(
    tensor_metadata
    PRIVATE
    -Wall
)
```

不会传播给demo。

如果希望demo也有警告，需要给demo单独配置，或者使用一个专门的接口配置target。

---

## 28. 使用INTERFACE库集中警告选项

当多个本项目target都需要相同警告，可以创建：

```cmake
add_library(project_warnings INTERFACE)

target_compile_options(
    project_warnings
    INTERFACE
    $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:
        -Wall
        -Wextra
        -Wpedantic
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
    >
)
```

然后：

```cmake
target_link_libraries(
    tensor_metadata
    PRIVATE
    project_warnings
)

target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    tensor_metadata
    project_warnings
)
```

`project_warnings`没有二进制文件，它只是一个可以被链接并传播配置的CMake target。

这里“链接”接口库，本质上是在建立使用要求关系。

这是综合练习2的内容；第一次先把警告直接写在各目标上，确认理解后再抽取。

---

## 29. 测试target

```cmake
include(CTest)

if(BUILD_TESTING)
    add_executable(tensor_metadata_tests
        tests/test_tensor_metadata.cpp
    )

    target_link_libraries(
        tensor_metadata_tests
        PRIVATE
        tensor_metadata
        project_warnings
    )

    add_test(
        NAME tensor_metadata_tests
        COMMAND tensor_metadata_tests
    )
endif()
```

这里创建两个不同概念：

```text
tensor_metadata_tests
    可执行target

add_test中的tensor_metadata_tests
    CTest测试名称
```

它们可以同名，但不是同一个CMake对象。

运行：

```bash
ctest \
  --test-dir day27/build \
  --output-on-failure
```

---

## 30. 推荐的完整CMakeLists起点

在 `day27/CMakeLists.txt`中逐段手写并理解：

```cmake
cmake_minimum_required(VERSION 3.20)

project(
    TensorMetadataProject
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(project_warnings INTERFACE)

target_compile_options(
    project_warnings
    INTERFACE
    $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:
        -Wall
        -Wextra
        -Wpedantic
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
    >
)

add_library(tensor_metadata STATIC
    src/data_type.cpp
    src/device.cpp
    src/tensor_shape.cpp
    src/tensor_metadata.cpp
)

target_include_directories(
    tensor_metadata
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(
    tensor_metadata
    PUBLIC
    cxx_std_17
)

set_target_properties(
    tensor_metadata
    PROPERTIES
    CXX_EXTENSIONS OFF
)

target_link_libraries(
    tensor_metadata
    PRIVATE
    project_warnings
)

add_executable(tensor_metadata_demo
    app/main.cpp
)

target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    tensor_metadata
    project_warnings
)

include(CTest)

if(BUILD_TESTING)
    add_executable(tensor_metadata_tests
        tests/test_tensor_metadata.cpp
    )

    target_link_libraries(
        tensor_metadata_tests
        PRIVATE
        tensor_metadata
        project_warnings
    )

    add_test(
        NAME tensor_metadata_tests
        COMMAND tensor_metadata_tests
    )
endif()
```

这份代码是学习目标，不要求你不思考地复制。每增加一个块，就重新配置或构建并观察变化。

---

## 31. 当前day27源码的预检查

CMake只组织构建，不会自动修复C++代码。

当前 `day27`仍有需要你先处理的C++问题，例如：

```text
device.hpp和tensor_layout.hpp缺少include guard
metadata_error.hpp使用optional但没有include <optional>
tensor_shape.cpp在类外定义构造函数时误写explicit
tensor_shape.cpp重复定义is_scalar()
tensor_shape.cpp和tensor_metadata.cpp缺少所需标准头
bytes_per_element()的非法枚举路径缺少return
size_t维度与0比较时不可能小于0
```

这些属于C++编译或逻辑问题，不是CMake配置问题。

开始CMake练习前先用已有知识逐项修正，并保留编译器实际诊断。不要为了让CMake“变绿”而随意删除功能。

特别提醒：

```cpp
std::size_t value;

if (value < 0) {
}
```

条件永远不成立，因为 `size_t`是无符号类型。负维度应该在从有符号外部输入转换为 `size_t`之前校验。

---

## 32. 配置、构建和运行必须分开理解

### 配置

在项目根目录 `CppLearning`执行：

```bash
cmake \
  -S day27 \
  -B day27/build \
  -DCMAKE_BUILD_TYPE=Debug
```

含义：

```text
-S day27
    源码目录，里面有CMakeLists.txt

-B day27/build
    构建目录，生成文件放这里

-DCMAKE_BUILD_TYPE=Debug
    对单配置生成器选择Debug配置
```

### 构建

```bash
cmake \
  --build day27/build \
  --parallel
```

### 运行demo

常见单配置生成器下：

```bash
./day27/build/tensor_metadata_demo
```

### 运行测试

```bash
ctest \
  --test-dir day27/build \
  --output-on-failure
```

---

## 33. 为什么使用源码外构建

推荐：

```text
day27/
├── CMakeLists.txt
├── include/
├── src/
└── build/
    ├── CMakeCache.txt
    ├── CMakeFiles/
    ├── Makefile或其他构建文件
    └── 编译产物
```

不要在源码目录直接执行产生大量文件的构建：

```bash
cd day27
cmake .
```

源码外构建的优点：

- 生成文件与源码分离；
- 清理时只删除build目录；
- 可以并存Debug和Release构建；
- Git更容易排除产物；
- 避免CMake缓存污染源码目录。

建议 `.gitignore`包含：

```gitignore
build/
cmake-build-*/
CMakeFiles/
CMakeCache.txt
compile_commands.json
```

如果 `compile_commands.json`位于build中，上面的 `build/`已经覆盖它。

---

## 34. 查看真实编译命令

```bash
cmake \
  --build day27/build \
  --verbose \
  > day29/report/verbose-build.txt \
  2>&1
```

在输出中检查是否包含：

```text
-std=c++17或等价选项
-Wall
-Wextra
-Wpedantic
-I.../day27/include
```

注意：具体选项可能随生成器和编译器不同。你应该检查语义是否存在，而不是死记参数顺序。

---

## 35. `compile_commands.json`

在CMakeLists中：

```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

配置后通常生成：

```text
day27/build/compile_commands.json
```

它为每个翻译单元记录真实编译命令。

可以运行：

```bash
/opt/homebrew/opt/llvm/bin/clang-tidy \
  -p day27/build \
  day27/src/tensor_shape.cpp \
  --config-file=day28/config/.clang-tidy
```

这样不需要再手写：

```text
-std=c++17
-I day27/include
```

因为工具会从编译数据库读取。

这就是昨天 `clang-tidy`学习与今天CMake的直接连接点。

---

## 36. Debug和Release

单配置生成器常用：

```bash
cmake \
  -S day27 \
  -B day27/build-debug \
  -DCMAKE_BUILD_TYPE=Debug

cmake \
  -S day27 \
  -B day27/build-release \
  -DCMAKE_BUILD_TYPE=Release
```

分开构建目录：

```text
build-debug/
build-release/
```

不要频繁在同一缓存目录中切换配置后猜测旧选项是否还存在。

今天只要求完成Debug构建，Release作为扩展。

---

## 37. Sanitizer也应当是target级配置

可以增加选项：

```cmake
option(
    ENABLE_SANITIZERS
    "Enable address and undefined behavior sanitizers"
    OFF
)
```

然后创建接口target：

```cmake
add_library(project_sanitizers INTERFACE)

if(ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES
       "Clang|AppleClang|GNU")
        target_compile_options(
            project_sanitizers
            INTERFACE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )

        target_link_options(
            project_sanitizers
            INTERFACE
            -fsanitize=address,undefined
        )
    endif()
endif()
```

需要同时加入编译和链接选项。

然后只链接到本项目可执行target：

```cmake
target_link_libraries(
    tensor_metadata_tests
    PRIVATE
    project_sanitizers
)
```

配置：

```bash
cmake \
  -S day27 \
  -B day27/build-asan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_SANITIZERS=ON
```

这是扩展内容。今天核心仍然是target、include和编译选项。

---

## 38. 不应该全局强制第三方库的警告

以后项目会使用：

- ONNX Runtime；
- TensorRT；
- GoogleTest；
- FFmpeg；
- 其他第三方库。

如果全局启用极严格警告甚至 `-Werror`，第三方头文件的告警可能让你的项目无法构建。

更合理的是：

```text
自己的target
    启用项目警告

第三方target
    保持它自己的配置
```

这也是target级配置比全局flags更适合大型AI部署工程的原因。

---

## 39. `SYSTEM` include目录只要求认识

第三方头文件可以作为系统include处理：

```cmake
target_include_directories(
    some_target
    SYSTEM PRIVATE
    ${THIRD_PARTY_INCLUDE_DIR}
)
```

某些编译器会降低来自这些头文件的警告。

不要把自己的项目头文件标记为 `SYSTEM`来隐藏告警。

---

## 40. 常见错误

### 错误1：把target理解成输出目录

CMake target是构建图节点，不是 `target/`文件夹。

### 错误2：使用全局 `include_directories()`

它让依赖来源不明确，应优先用 `target_include_directories()`。

### 错误3：使用 `CMAKE_CXX_FLAGS`拼接所有参数

作用范围太大，跨编译器差，应使用目标级命令。

### 错误4：把 `-std=c++17`放进compile options

优先用 `target_compile_features(... cxx_std_17)`。

### 错误5：公开include使用 `PRIVATE`

库自己能编译，但消费者包含库头文件时找不到路径。

### 错误6：内部include使用 `PUBLIC`

会把实现细节泄露给消费者。

### 错误7：普通库把自己需要的include只写成 `INTERFACE`

消费者能获得路径，但库自己的 `.cpp`反而不能使用。

### 错误8：重复给每个消费者手写库的公开include路径

说明库target没有正确声明使用要求。

### 错误9：所有编译选项都写成 `PUBLIC`

会把本项目警告策略强制传播给下游。

### 错误10：忘记链接库target

只添加include路径能通过编译，但缺少函数实现时会链接失败。

### 错误11：认为CMake会修复C++语法错误

CMake只是把正确命令交给编译器。

### 错误12：在源码目录生成构建文件

会污染源码，推荐 `-S`与 `-B`分离。

### 错误13：手写绝对用户路径

```cmake
/Users/某个人/Project/include
```

换电脑立即失效。使用CMake提供的源码目录变量。

### 错误14：使用 `file(GLOB)`后认为新文件一定自动生效

学习阶段应显式列出源文件。

### 错误15：只构建库不运行测试

成功编译不代表业务行为正确。

### 错误16：修改CMakeLists后只运行旧二进制

需要重新配置或构建，让生成系统更新。

---

## 41. 综合练习1：为张量元数据建立target构建图

对应文件：

```text
day27/CMakeLists.txt
day27/app/main.cpp
day27/tests/test_tensor_metadata.cpp
```

任务合并覆盖：

- 修复day27当前基础编译问题；
- `project()`；
- `add_library()`；
- `add_executable()`；
- `target_include_directories()`；
- `target_compile_features()`；
- `target_compile_options()`；
- `target_link_libraries()`；
- CTest；
- compile database。

要求：

1. 建立 `tensor_metadata`静态库；
2. 库包含四个现有 `.cpp`；
3. 公开include目录使用 `PUBLIC`；
4. C++17要求使用 `PUBLIC`；
5. 警告选项使用 `PRIVATE`或警告接口库；
6. 创建demo target；
7. demo只能通过链接库继承公开include，不能重复手写；
8. 创建test target；
9. 注册CTest测试；
10. 生成 `compile_commands.json`；
11. 成功完成Debug配置与构建；
12. demo输出至少一份张量元数据；
13. test检查标量、空张量、普通张量和溢出情况。

`app/main.cpp`建议使用：

```cpp
TensorMetadata metadata(
    "mel_features",
    TensorShape({1, 3000, 80}),
    DataType::float32,
    TensorLayout::batch_time_feature,
    Device(DeviceType::cpu, 0)
);
```

输出：

```text
name=mel_features
rank=3
numel=240000
bytes=960000
```

---

## 42. 综合练习2：传播作用域实验

对应文件：

```text
day29/examples/interface_library/CMakeLists.txt
```

这个练习不需要重新写完整C++业务代码，可以在day27主项目上依次实验并记录结果。

完成以下实验：

### 实验A：正确PUBLIC include

```cmake
target_include_directories(
    tensor_metadata
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

确认demo不设置include也能编译。

### 实验B：错误改成PRIVATE

临时改为：

```cmake
PRIVATE
```

重新配置和构建，记录消费者出现的头文件错误，然后恢复 `PUBLIC`。

### 实验C：错误改成INTERFACE

临时改为：

```cmake
INTERFACE
```

观察库自己的 `.cpp`为什么找不到头文件，然后恢复。

### 实验D：警告接口库

创建：

```cmake
add_library(project_warnings INTERFACE)
```

让库、demo和tests分别以 `PRIVATE`方式链接它，并从verbose build确认每个本项目target都获得警告选项。

### 实验E：编译数据库

用 `clang-tidy -p day27/build`分析一个源文件，确认不再手写include路径。

最后把所有配置恢复到正确状态，不要把故意错误配置留在最终提交中。

---

## 43. 集中测试清单

### 配置阶段

1. `cmake -S day27 -B day27/build`成功；
2. 输出识别到C++编译器；
3. 没有写死用户绝对路径；
4. `compile_commands.json`生成；
5. BUILD_TESTING默认为启用；
6. CMakeLists修改后能重新配置。

### target阶段

7. 存在 `tensor_metadata`库target；
8. 存在 `tensor_metadata_demo`可执行target；
9. 存在 `tensor_metadata_tests`测试target；
10. 存在 `project_warnings`接口target；
11. demo链接tensor_metadata；
12. tests链接tensor_metadata；
13. 没有通过复制源码构建demo；
14. 不同target名称无冲突。

### include传播

15. 库自己的源文件能找到公开头文件；
16. demo不重复设置include也能找到公开头文件；
17. tests不重复设置include也能找到公开头文件；
18. 公共include使用PUBLIC；
19. 警告接口库使用INTERFACE；
20. 没有使用全局 `include_directories()`。

### 编译要求

21. 库声明 `cxx_std_17`；
22. C++17要求能够传播给消费者；
23. 没有用compile options硬编码 `-std=c++17`；
24. Clang获得 `-Wall -Wextra -Wpedantic`；
25. 警告选项没有错误地PUBLIC传播给外部；
26. CXX_EXTENSIONS关闭；
27. 没有修改 `CMAKE_CXX_FLAGS`；
28. verbose build能找到实际选项。

### 构建与运行

29. 静态库成功生成；
30. demo成功链接；
31. tests成功链接；
32. demo输出正确；
33. 标量numel为1；
34. 空张量numel为0；
35. `[1,3000,80]`numel为240000；
36. float32字节数为960000；
37. 溢出返回nullopt；
38. CTest全部通过；
39. 退出码为0；
40. 无新增编译器警告。

### 工具连接

41. clang-tidy能读取build目录；
42. clang-tidy能找到项目头文件；
43. 不需要在命令后重复 `-I`；
44. 报告保存配置输出；
45. 报告保存构建输出；
46. 报告保存测试输出；
47. 报告保存verbose build；
48. Git不包含build产物。

---

## 44. 命令执行与报告保存

配置：

```bash
cmake \
  -S day27 \
  -B day27/build \
  -DCMAKE_BUILD_TYPE=Debug \
  > day29/report/configure-output.txt \
  2>&1
```

构建：

```bash
cmake \
  --build day27/build \
  --parallel \
  > day29/report/build-output.txt \
  2>&1
```

测试：

```bash
ctest \
  --test-dir day27/build \
  --output-on-failure \
  > day29/report/test-output.txt \
  2>&1
```

查看测试退出码：

```bash
echo $?
```

详细构建：

```bash
cmake \
  --build day27/build \
  --verbose \
  > day29/report/verbose-build.txt \
  2>&1
```

如果构建没有重新执行编译，可以先修改一个源文件时间或使用构建工具提供的清理重建方式。不要为了看到命令随意删除源码。

---

## 45. 学习报告模板

对应文件：

```text
day29/report/cmake-learning-report.md
```

内容：

```markdown
# 现代CMake target学习报告

## 1. 环境

- CMake版本：
- C++编译器：
- 生成器：
- 构建类型：

## 2. Target列表

|Target|类型|源文件|直接依赖|
|---|---|---|---|

## 3. 使用要求

|要求|所属target|作用域|是否传播|原因|
|---|---|---|---|---|

## 4. include目录实验

- PUBLIC结果：
- PRIVATE结果：
- INTERFACE结果：
- 最终选择：

## 5. 编译特性与选项

- C++17如何声明：
- 为什么是PUBLIC：
- 警告如何声明：
- 为什么是PRIVATE/INTERFACE：

## 6. 构建图

- tensor_metadata：
- tensor_metadata_demo：
- tensor_metadata_tests：
- project_warnings：

## 7. 验证结果

- 配置：
- 构建：
- demo：
- CTest：
- clang-tidy：
- 退出码：

## 8. 遇到的错误

|阶段|错误摘要|根因|修复|
|---|---|---|---|

## 9. 不采用的旧写法

- include_directories：
- CMAKE_CXX_FLAGS：
- 手写-std：
- 源码内构建：

## 10. 后续扩展

- Sanitizer target：
- GoogleTest：
- install/export：
- ONNX Runtime依赖：
```

---

## 46. 复盘问题

完成后回答：

1. CMake是不是编译器？
2. 什么是CMake target？
3. target与 `target/`目录有什么区别？
4. `add_library()`创建了什么？
5. `add_executable()`创建了什么？
6. STATIC、SHARED和INTERFACE库有什么区别？
7. 为什么头文件不写进add_library也可能正常编译？
8. 为什么学习阶段不推荐默认使用file(GLOB)？
9. `target_include_directories()`解决什么问题？
10. `CMAKE_CURRENT_SOURCE_DIR`表示什么？
11. PRIVATE的当前target和消费者行为是什么？
12. PUBLIC的当前target和消费者行为是什么？
13. INTERFACE的当前target和消费者行为是什么？
14. day27公开include为什么应该是PUBLIC？
15. 如果错误写成PRIVATE，谁会编译失败？
16. 如果错误写成INTERFACE，谁会编译失败？
17. 为什么内部detail目录通常是PRIVATE？
18. 纯头文件库为什么适合INTERFACE？
19. `target_compile_features()`表达什么？
20. 为什么day27的C++17要求是PUBLIC？
21. 为什么不把 `-std=c++17`直接写入compile options？
22. `target_compile_options()`表达什么？
23. 为什么警告选项通常是PRIVATE？
24. 为什么不同编译器需要不同警告选项？
25. 什么是生成器表达式？
26. 为什么不推荐修改CMAKE_CXX_FLAGS？
27. `target_link_libraries()`除了链接二进制还会传播什么？
28. 为什么demo链接库后不需要重复include目录？
29. INTERFACE警告库有没有自己的目标文件？
30. 为什么还能对它使用target_link_libraries？
31. CTest测试和测试可执行target有什么区别？
32. `-S`和 `-B`分别是什么？
33. 为什么使用源码外构建？
34. `CMAKE_BUILD_TYPE=Debug`何时生效？
35. `compile_commands.json`记录什么？
36. 它如何帮助clang-tidy？
37. 为什么CMake成功配置不等于C++编译成功？
38. 为什么库成功编译不等于最终链接成功？
39. 为什么成功链接不等于测试通过？
40. 今天的target思维以后如何用于ONNX Runtime和TensorRT依赖？

---

## 47. 验收清单

### 概念验收

- [ ] 能解释target；
- [ ] 能画出库、demo和test的依赖关系；
- [ ] 能正确选择PRIVATE/PUBLIC/INTERFACE；
- [ ] 能区分compile features和compile options；
- [ ] 能解释使用要求传播；
- [ ] 能解释源码外构建；
- [ ] 能解释compile database。

### 文件验收

- [ ] 创建 `day27/CMakeLists.txt`；
- [ ] 创建 `day27/app/main.cpp`；
- [ ] 创建 `day27/tests/test_tensor_metadata.cpp`；
- [ ] 创建警告INTERFACE target；
- [ ] 创建demo target；
- [ ] 创建test target；
- [ ] 完成学习报告；
- [ ] 保存配置、构建、测试和verbose输出。

### 构建验收

- [ ] 配置成功；
- [ ] 构建成功；
- [ ] 无新增警告；
- [ ] demo运行成功；
- [ ] CTest通过；
- [ ] C++17正确生效；
- [ ] 公开include正确传播；
- [ ] `compile_commands.json`生成；
- [ ] clang-tidy能使用构建数据库；
- [ ] build目录没有提交到Git。

---

## 48. 和AI部署工程的联系

真实AI部署项目不会只有一个 `.cpp`。通常会拆分为：

```text
audio_preprocessing库
tensor_metadata库
onnx_runtime_backend库
tensorrt_backend库
inference_service库
server可执行程序
tests可执行程序
benchmark可执行程序
```

CMake target关系可能是：

```text
tensor_metadata ──────────────┐
audio_preprocessing ──────────┤
onnx_runtime_backend ─────────┼─> inference_service
                              │          │
                              │          ├─> server
                              │          ├─> tests
                              │          └─> benchmark
```

第三方依赖也通常以target形式接入：

```cmake
target_link_libraries(
    inference_backend
    PRIVATE
    onnxruntime_target
)
```

今天学的：

```text
target
include使用要求
C++标准传播
目标级警告
依赖图
compile database
```

都是后续构建C++推理服务的直接基础。

---

## 49. 今天结束时的最小成果

时间有限时至少完成：

```text
1. 修复day27当前阻塞编译的问题
2. 建立tensor_metadata静态库target
3. 正确公开include目录
4. 正确声明C++17
5. 建立demo并链接库
6. 建立tests并注册CTest
7. 添加target级警告
8. 完成Debug构建和测试
9. 生成compile_commands.json
10. 使用clang-tidy从build目录检查一个源文件
```

最终要能够独立解释下面几行，而不只是复制：

```cmake
target_include_directories(
    tensor_metadata
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(
    tensor_metadata
    PUBLIC
    cxx_std_17
)

target_link_libraries(
    tensor_metadata_demo
    PRIVATE
    tensor_metadata
)
```

它们合起来表达的是：

> `tensor_metadata`公开自己的头文件位置和C++17要求；`tensor_metadata_demo`私有依赖这个库，并自动继承使用该库所必需的配置。
