cmake_minimum_required(VERSION 3.26)

project(RVM)

set(CMAKE_CXX_STANDARD 20)

# 查找 Python 3 环境（指定所需组件，如解释器、库、开发文件等）
#find_package(Python3
#        REQUIRED                  # 必须找到 Python，否则报错
#        COMPONENTS Interpreter    # 需包含解释器（对应 Python3_EXECUTABLE）
#        Development    # 需包含开发文件（头文件、库，用于嵌入或扩展）
#)

## 验证是否找到 Python
#if(Python3_FOUND)
#    message(STATUS "Found Python3: ${Python3_EXECUTABLE} (version ${Python3_VERSION})")
#    message(STATUS "Python3 include dirs: ${Python3_INCLUDE_DIRS}")
#    message(STATUS "Python3 libraries: ${Python3_LIBRARIES}")
#else()
#    message(FATAL_ERROR "Python3 not found!")
#endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

add_executable(RVM code/src/rvm_main.cpp
        code/src/ra_base.cpp
        code/include/ra_base.h
        code/src/ra_core.cpp
        code/include/ra_core.h
        code/src/ra_parser.cpp
        code/include/ra_parser.h
        code/src/lib/ra_utils.cpp
        code/include/lib/ra_utils.h
        code/src/rvm_ris.cpp
        code/include/rvm_ris.h
        code/src/lib/newrcc.cpp
        code/include/lib/newrcc.h
        code/include/rvm_version.h
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

# 将 Python 关联到你的目标（例如生成可执行文件或库）
#target_include_directories(RVM PRIVATE ${Python3_INCLUDE_DIRS})
#target_link_libraries(RVM PRIVATE ${Python3_LIBRARIES})