# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.16)

project(foxlox)

# 包含子项目。
add_subdirectory("libfoxlox")
add_subdirectory("fox")
add_subdirectory("tests")