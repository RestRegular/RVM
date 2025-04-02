# RVM
<img src="https://github.com/user-attachments/assets/8ea29c7a-2f91-4e88-a389-97cfbe8850f4" height=80px></img>

**RVM**（Rio Virtual Machine），用于运行 RA 代码文件或 RSI 代码文件。

# 目录

1. **[功能概述](#1-功能概述)** 
2. **[开发环境配置](#2-开发环境配置)**
    - **[依赖项](#21-依赖项)**
    - **[编译步骤](#22-编译步骤)**
3. **[使用方法](#3-使用方法)**
4. **[参数说明](#4-参数说明)**
    - **[标志参数](#41-标志参数)**
    - **[选项参数](#42-选项参数)**
    - **[位置参数](#43-位置参数)**
5. **[代码结构](#5-代码结构)**
6. **[错误处理](#6-错误处理)**
7. **[RA 语言规范](#7-ra-语言规范)**

## 1. 功能概述

RVM 是一个虚拟机，主要用于运行 RA 代码文件或 RSI 代码文件。它支持多种操作，如编译、运行、版本检查等，并提供了详细的参数配置选项，方便用户根据需求进行操作。

## 2. 开发环境配置

### 2.1 依赖项

- CMake 版本 3.29 或更高
- C++ 20 标准支持

### 2.2 编译步骤

- **克隆代码仓库到本地：**
    ```bash
    git clone https://github.com/RestRegular/RVM.git
    cd RVM
    ```
- **创建构建目录并进入：**
    ```bash
    mkdir build
    cd build
    ```
- **使用 CMake 生成构建文件：**
    ```bash
    cmake ..
    ```
- **编译项目：**
    ```bash
    cmake --build .
    ```

- **作者的方法：**

  我在开发的时候使用了 bat 编写了一个编译脚本，编译后会直接运行，如果你使用 CMake 的话可以直接使用这个编译脚本。代码如下：
    ```bash
    # file: execute.bat
    @echo off
    cls
    color 0A
    set COMMAND_PREFIX="^>^>^>"
    set CMAKE_PATH=D:\soft\Clion\CLion-2024.2.3\bin\cmake\win\x64\bin\cmake.exe # 请将此路径替换为你的 CMake 路径

    set PROJECT_TARGET=RVM

    set PROJECT_DIR=D:\ClionProjects\%PROJECT_TARGET% # 请将此路径替换为你的项目路径
    set BUILD_DIR=%PROJECT_DIR%\cmake-build-debug # 在你的项目目录下创建一个名为 cmake-build-debug 的目录（或其他名称，注意修改 BUILD_DIR 变量）
    set EXECUTABLE_PATH=%BUILD_DIR%\%PROJECT_TARGET%.exe
    echo %COMMAND_PREFIX:"=% Building project...
    "%CMAKE_PATH%" --build %BUILD_DIR% --target %PROJECT_TARGET% -j 10 # 这里使用了 -j 10 参数，你可以根据自己的需求进行调整
    color 07
    cls
    echo %COMMAND_PREFIX:"=% %EXECUTABLE_PATH% %*
    %EXECUTABLE_PATH% %*
    ```
    **运行命令：**
    `execute.bat [options]`

## 3. 使用方法

编译完成后，可在 build 目录下找到生成的可执行文件 RVM。使用以下命令运行程序：

```bash
RVM.exe [options]
```

## 4. 参数说明

1. **标志参数**
    `--help (-h)`：显示帮助信息并退出。
    `--version (-v)`：显示版本信息并退出。
    `--vs-check (-vc)`：显示程序版本和指定 RSI 文件的版本信息并退出。
    `--run (-r)`：运行指定的可执行文件。
    `--comp (-c)`：编译指定的源文件。
    `--time-info (-ti)`：启用执行时间信息输出。
    `--debug (-d, -db)`：启用调试模式。
    `--rvm-work-directory (-rwd)`：指定 RVM 目录为程序的工作目录。
    `--precomp-link (-pcl, -pl)`：指定预编译 RSI 文件的路径。

2. **选项参数**
    `--help-option (-ho)`：指定帮助选项的名称。
    `--comp-level (-cl)`：指定编译级别。
    `--target (-tar, -t)`：指定操作的目标文件路径。
    `--archive (-arc, -a)`：指定生成文件的输出路径。
    `--working-dir (-wd)`：指定操作的工作目录。
    `--output-redirect (-or)`：指定输出重定向的位置。
    `--precomp-link-dir (-pcld, -pld)`：指定预编译 RSI 文件的路径。

3. **互斥和依赖关系**
    `--run` 和 `--comp` 互斥。
    `--rvm-work-directory` 和 `--working-dir` 互斥。
    `--help`、`--version` 与 `--run`、`--comp`、`--debug` 等选项互斥。
    `--help-option` 依赖于 `--help`。
    `--run` 和 `--comp` 依赖于 `--target`。
    `--debug` 依赖于 `--run`。
    `--vs-check` 依赖于 `--target`。

## 5. 代码结构

项目主要由以下几个部分组成：

- `rvm_main.cpp`：程序入口文件，负责初始化参数解析器和执行相应的操作。
- `ra_base.cpp/h`：基础类和工具的实现，如 `RVM_IO`、`RVM_Error` 等。
- `ra_core.cpp/h`：核心功能的实现，如文件操作、内存管理等。
- `ra_parser.cpp/h`：代码解析器的实现，包括代码预处理、指令解析等。
- `ra_utils.cpp/h`：工具函数的实现，如文件读取、字符串处理等。
- `rvm_ris.cpp/h`：指令集的实现，如乘法指令、文件读取指令等。

## 6. 错误处理

程序在运行过程中会捕获并处理各种异常，如参数错误、语法错误、文件读取错误等。错误信息会输出到控制台或指定的输出文件中，方便用户进行调试和排查问题。

## 7. RA 语言规范
详细的 RA 语言规范文档见项目中的 `document/RALanguage.md` 文件。
