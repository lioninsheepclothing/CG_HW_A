cmake_minimum_required(VERSION 3.10)
project(CG_ZBuffer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(third_party)
include_directories(src)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(CG_ZBuffer ${SOURCES} "src/Renderer.h" "src/Renderer.cpp" "src/SceneTree.h" "src/SceneTree.cpp")

target_compile_definitions(CG_ZBuffer PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}")

if(EXISTS "${CMAKE_SOURCE_DIR}/models")
    file(COPY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()