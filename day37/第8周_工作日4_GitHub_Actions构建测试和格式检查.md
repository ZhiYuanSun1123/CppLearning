# 第8周工作日4：从零学习 GitHub Actions

> 今天的任务：让 GitHub 自动完成 C++ 代码格式检查、编译和测试。

## 1. 先看结论：今天到底要做什么

你以前是在自己的电脑上执行这些检查：

```bash
bash scripts/check-format.sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

今天要把它们交给 GitHub 自动执行：

```text
把代码推送到 GitHub
        ↓
GitHub 启动一台临时电脑
        ↓
下载你的代码
        ↓
检查格式
        ↓
编译项目
        ↓
运行 GoogleTest
        ↓
在网页上显示成功或失败
```

这种“提交代码后自动检查”的过程叫 **CI（持续集成）**。

今天不学习部署、Docker、GPU、服务器发布、Secret，也不要求你自己从空白写出整份配置。

## 2. 今天使用哪些文件

项目目录是：

```text
day37/
├── include/audio_ci/audio_metadata.hpp
├── src/audio_metadata.cpp
├── app/main.cpp
├── tests/audio_metadata_test.cpp
├── scripts/check-format.sh
├── scripts/format.sh
├── exercises/01_test_failure_experiment.md
├── exercises/02_format_failure_experiment.md
├── report/学习报告.md
├── .clang-format
├── CMakeLists.txt
└── CMakePresets.json
```

今天最重要的新文件是：

[打开 day37-ci.yml](</Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/.github/workflows/day37-ci.yml>)

它必须位于整个 Git 仓库根目录下的：

```text
.github/workflows/day37-ci.yml
```

GitHub 只会自动寻找 `.github/workflows` 中的工作流文件。把它放在 `day37` 里面不会自动运行。

## 3. 第一步：先在本地检查项目

### 3.1 进入 day37

```bash
cd /Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day37
```

后面的本地命令都在这个目录执行。

### 3.2 检查代码格式

```bash
bash scripts/check-format.sh
```

这条命令会检查 C++ 文件是否符合 `.clang-format` 中的格式规则，但不会修改代码。

运行后可以查看上一条命令是否成功：

```bash
echo $?
```

- 输出 `0`：成功；
- 输出非 `0`：失败。

如果格式检查失败，执行：

```bash
bash scripts/format.sh
```

这条命令会修改代码格式。修改后再次运行检查。

> `check-format.sh` 中包含一些 Bash 写法。今天只需要会运行它并理解用途，不要求掌握全部脚本语法。

### 3.3 编译并测试 Debug 版本

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

三条命令分别表示：

| 命令 | 作用 |
|---|---|
| `cmake --preset debug` | 生成 Debug 构建配置 |
| `cmake --build --preset debug` | 编译和链接程序 |
| `ctest --preset debug` | 运行测试 |

预期看到：

```text
100% tests passed, 0 tests failed out of 4
```

### 3.4 编译并测试 Release 版本

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release
```

Release 版本也应当通过全部测试。

为什么先在本地执行？因为本地发现问题速度更快。GitHub Actions 是再次验证，不应该替代本地检查。

## 4. GitHub Actions 的六个基本名词

先不要看完整配置，只理解下面六个名词。

### 4.1 Workflow：工作流

一整套自动化检查方案。本项目的工作流文件是 `day37-ci.yml`。

### 4.2 Event：触发事件

决定什么时候执行工作流，例如：

- 推送代码 `push`；
- 创建或更新合并请求 `pull_request`；
- 在网页上手动运行 `workflow_dispatch`。

### 4.3 Job：作业

一组在同一台临时电脑上执行的步骤。本项目有两类 Job：

- `format`：格式检查；
- `build-and-test`：编译和测试。

### 4.4 Step：步骤

Job 中的一步，例如下载代码、安装工具、编译、测试。

### 4.5 Action：别人封装好的步骤

例如：

```yaml
uses: actions/checkout@v7
```

它负责把 GitHub 仓库中的代码下载到临时电脑。

### 4.6 Runner：执行任务的电脑

`ubuntu-latest` 表示 Ubuntu 临时电脑，`macos-latest` 表示 macOS 临时电脑。

## 5. YAML 只学今天需要的语法

GitHub Actions 使用 YAML 文件。YAML 最重要的是缩进。

### 5.1 `名称: 内容`

```yaml
name: Day37 C++ CI
```

表示工作流名称是 `Day37 C++ CI`。

### 5.2 缩进表示包含关系

```yaml
jobs:
  format:
    runs-on: ubuntu-latest
```

含义是：

```text
jobs
└── format
    └── runs-on
```

不要使用 Tab，统一使用空格。

### 5.3 `-` 表示列表中的一项

```yaml
steps:
  - name: Check out repository
    uses: actions/checkout@v7
```

这里表示 `steps` 列表中有一个步骤。

### 5.4 `|` 和 `>-` 表示多行内容

```yaml
run: |
  sudo apt-get update
  sudo apt-get install --yes clang-format
```

两条命令会依次执行。当前文件中的 `>-` 也是为了把多行内容组合成一条命令。今天会读即可。

### 5.5 `${{ ... }}` 表示 GitHub 表达式

```yaml
${{ matrix.build_type }}
```

这不是 C++ 语法，而是 GitHub Actions 在运行时替换的值，例如 `Debug` 或 `Release`。

## 6. 从上到下读懂 day37-ci.yml

### 6.1 工作流名称

```yaml
name: Day37 C++ CI
```

这个名称会显示在 GitHub 的 Actions 页面中。

### 6.2 什么时候触发

```yaml
on:
  push:
    paths:
      - "day37/**"
      - ".github/workflows/day37-ci.yml"
  pull_request:
    paths:
      - "day37/**"
      - ".github/workflows/day37-ci.yml"
  workflow_dispatch:
```

含义：

- 推送了 `day37` 中的文件时运行；
- 合并请求修改了 `day37` 时运行；
- 修改工作流本身时运行；
- 也允许在网页上手动运行。

`day37/**` 中的 `**` 可以理解为“day37 目录以及它下面的所有内容”。

### 6.3 权限

```yaml
permissions:
  contents: read
```

表示工作流只需要读取仓库代码，不允许修改仓库内容。

### 6.4 取消旧任务

```yaml
concurrency:
  group: day37-${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true
```

同一分支连续推送多次时，新任务可以取消尚未完成的旧任务，避免重复占用资源。

这属于辅助配置，今天知道作用即可。

## 7. Job 一：格式检查

核心配置如下：

```yaml
format:
  name: Format check
  runs-on: ubuntu-latest
  steps:
    - name: Check out repository
      uses: actions/checkout@v7

    - name: Install clang-format
      run: |
        sudo apt-get update
        sudo apt-get install --yes clang-format

    - name: Check C++ formatting
      working-directory: day37
      run: bash scripts/check-format.sh
```

逐步解释：

1. 创建 Ubuntu 临时电脑；
2. 下载仓库代码；
3. 安装 `clang-format`；
4. 进入 `day37`；
5. 运行本地已经验证过的格式检查脚本。

`working-directory: day37` 非常重要。因为 Runner 默认位于仓库根目录，而脚本位于 `day37/scripts`。

## 8. Job 二：编译和测试

### 8.1 Matrix 是组合表

```yaml
matrix:
  os: [ubuntu-latest, macos-latest]
  build_type: [Debug, Release]
```

GitHub 会生成四种组合：

| 系统 | 构建类型 |
|---|---|
| Ubuntu | Debug |
| Ubuntu | Release |
| macOS | Debug |
| macOS | Release |

因此页面上会出现四个编译测试 Job。再加一个格式检查 Job，一共五个。

### 8.2 配置项目

```yaml
- name: Configure
  working-directory: day37
  run: >-
    cmake -S . -B build/ci
    -DCMAKE_BUILD_TYPE=${{ matrix.build_type }}
    -DBUILD_TESTING=ON
```

参数含义：

| 参数 | 含义 |
|---|---|
| `-S .` | 源代码目录是当前目录 |
| `-B build/ci` | 构建文件放在 `build/ci` |
| `CMAKE_BUILD_TYPE` | 选择 Debug 或 Release |
| `BUILD_TESTING=ON` | 构建测试程序 |

### 8.3 编译项目

```yaml
- name: Build
  working-directory: day37
  run: cmake --build build/ci --parallel 4
```

`--parallel 4` 表示最多并行执行四个编译任务。

### 8.4 运行测试

```yaml
- name: Test
  working-directory: day37
  run: >-
    ctest --test-dir build/ci
    --build-config ${{ matrix.build_type }}
    --output-on-failure
```

`--output-on-failure` 表示测试失败时输出详细信息，方便定位问题。

## 9. 推送代码并观察结果

先在仓库根目录查看改动：

```bash
cd /Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning
git status
```

确认文件没有问题后，再由你决定是否执行：

```bash
git add day37 .github/workflows/day37-ci.yml
git commit -m "Add CI for day37"
git push
```

> 本文档没有替你提交或推送代码。

推送后：

1. 打开 GitHub 仓库；
2. 点击顶部的 `Actions`；
3. 点击 `Day37 C++ CI`；
4. 打开本次运行记录；
5. 查看五个 Job；
6. 如果失败，展开红色步骤阅读日志。

## 10. 两个综合练习

### 练习一：故意制造测试失败

按照下面的文件操作：

[打开 01_test_failure_experiment.md](</Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day37/exercises/01_test_failure_experiment.md>)

目标：

1. 修改一个测试的预期值；
2. 本地运行测试并看到失败；
3. 根据失败信息找到测试名称和预期值；
4. 恢复正确代码；
5. 再次测试并确认通过。

### 练习二：故意制造格式失败

按照下面的文件操作：

[打开 02_format_failure_experiment.md](</Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day37/exercises/02_format_failure_experiment.md>)

目标：

1. 故意写出不符合规则的格式；
2. 运行 `check-format.sh` 并看到失败；
3. 运行 `format.sh` 自动修复；
4. 再次检查并确认成功。

这两个练习比背 YAML 更重要，因为它们训练你阅读 CI 错误信息。

## 11. 常见问题

### 11.1 本地成功，GitHub 失败

可能原因：

- 本地有依赖，但 Runner 没安装；
- 文件名大小写不一致；
- macOS 和 Ubuntu 行为不同；
- 本地存在没有提交的文件；
- 工作目录写错。

处理方法：先找到第一个红色步骤，再阅读它的第一条明确错误，不要只看最后一句 `exit code 1`。

### 11.2 GitHub 没有出现工作流

检查：

- 文件是否真的已经推送；
- 路径是否是 `.github/workflows/day37-ci.yml`；
- YAML 缩进是否正确；
- 本次修改是否命中 `paths` 条件。

### 11.3 显示 `No tests were found`

检查：

- 配置时是否使用 `-DBUILD_TESTING=ON`；
- `CMakeLists.txt` 是否启用了测试；
- 是否先成功编译测试程序；
- `ctest` 指向的构建目录是否正确。

## 12. CI 能保证什么、不能保证什么

它能证明：

- 在指定系统上能够编译；
- 当前自动化测试通过；
- 代码满足当前格式规则。

它不能证明：

- 程序完全没有错误；
- 未编写测试的情况也正确；
- 性能一定达标；
- GPU、驱动和线上环境一定可用；
- 业务设计一定合理。

CI 的含义是“自动执行已有检查”，不是“自动证明软件绝对正确”。

## 13. 今日验收标准

完成下面内容即可结束今天的任务：

- [ ] 能用自己的话解释 CI；
- [ ] 能区分 Workflow、Job、Step 和 Runner；
- [ ] 本地格式检查通过；
- [ ] 本地 Debug 构建和测试通过；
- [ ] 本地 Release 构建和测试通过；
- [ ] 能解释为什么工作流在 `.github/workflows`；
- [ ] 能解释 Matrix 为什么生成四种构建组合；
- [ ] 完成一次测试失败实验并恢复；
- [ ] 完成一次格式失败实验并恢复；
- [ ] 推送后能在 Actions 页面找到失败步骤和日志。

## 14. 完成后请回答

1. GitHub Actions 和你本地运行命令有什么关系？
2. Workflow、Job、Step 分别是什么？
3. 为什么当前页面会显示五个 Job？
4. `working-directory: day37` 有什么作用？
5. 为什么格式检查失败时 CI 应当失败？
6. 为什么本地测试通过后，还需要 GitHub 再测试一次？

如果这六个问题能自己说明白，今天就算真正掌握，而不是只复制了一份 YAML。

## 15. 相关文件

- [GitHub Actions 工作流](</Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/.github/workflows/day37-ci.yml>)
- [格式检查脚本](</Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day37/scripts/check-format.sh>)
- [自动格式化脚本](</Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day37/scripts/format.sh>)
- [学习报告](</Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day37/report/学习报告.md>)

## 16. 官方资料（今天不要求通读）

- [GitHub Actions 基本概念](https://docs.github.com/actions/get-started/understand-github-actions)
- [GitHub Actions 工作流语法](https://docs.github.com/actions/reference/workflows-and-actions/workflow-syntax)
- [GitHub Actions 表达式](https://docs.github.com/actions/reference/workflows-and-actions/expressions)
- [CMake 命令行工具](https://cmake.org/cmake/help/latest/manual/cmake.1.html)
- [CTest 命令行工具](https://cmake.org/cmake/help/latest/manual/ctest.1.html)
