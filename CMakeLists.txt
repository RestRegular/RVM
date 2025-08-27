cmake_minimum_required(VERSION 3.29)

project(RVM)

set(CMAKE_CXX_STANDARD 20)

# 查找 Python 3 环境（指定所需组件，如解释器、库、开发文件等）
find_package(Python3
        REQUIRED                  # 必须找到 Python，否则报错
        COMPONENTS Interpreter    # 需包含解释器（对应 Python3_EXECUTABLE）
        Development    # 需包含开发文件（头文件、库，用于嵌入或扩展）
)

# 验证是否找到 Python
if(Python3_FOUND)
    message(STATUS "Found Python3: ${Python3_EXECUTABLE} (version ${Python3_VERSION})")
    message(STATUS "Python3 include dirs: ${Python3_INCLUDE_DIRS}")
    message(STATUS "Python3 libraries: ${Python3_LIBRARIES}")
else()
    message(FATAL_ERROR "Python3 not found!")
endif()

add_executable(RVM src/rvm_main.cpp
        src/ra_base.cpp
        include/ra_base.h
        src/ra_core.cpp
        include/ra_core.h
        src/ra_parser.cpp
        include/ra_parser.h
        src/lib/ra_utils.cpp
        include/lib/ra_utils.h
        src/rvm_ris.cpp
        include/rvm_ris.h
        src/lib/newrcc.cpp
        include/lib/newrcc.h
        include/rvm_version.h
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

# 将 Python 关联到你的目标（例如生成可执行文件或库）
target_include_directories(RVM PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(RVM PRIVATE ${Python3_LIBRARIES})