cmake_minimum_required(VERSION 3.10)
project(CG_ZBuffer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/third_party)
include_directories(${PROJECT_SOURCE_DIR}/src)

set(SOURCES
    src/main.cpp
    src/Mesh.cpp
    src/stb_impl.cpp
)

# 生成可执行文件
add_executable(CG_ZBuffer ${SOURCES})

# 自动化优化：编译时自动将项目根目录下的 models 文件夹复制到 build 目录下
file(COPY ${PROJECT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})