# CMakeList.txt: foxlox 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.16)

# 将源代码添加到此项目的可执行文件。
add_executable (foxlox 
  "foxlox.cpp"
  "mimalloc-new-delete.cpp"
  "debug.cpp" "chunk.cpp" "value.cpp" "util.h")
target_compile_features(foxlox PRIVATE cxx_std_20)

find_package(Microsoft.GSL CONFIG REQUIRED)
target_link_libraries(foxlox PRIVATE Microsoft.GSL::GSL)

find_package(mimalloc 1.6 CONFIG REQUIRED)
target_link_libraries(foxlox PRIVATE mimalloc)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(foxlox PUBLIC fmt::fmt)

find_package(range-v3 CONFIG REQUIRED)
target_link_libraries(foxlox PRIVATE range-v3)

if(MSVC)
  target_compile_options(foxlox PRIVATE /utf-8)
  target_compile_options(foxlox PRIVATE /W4 /WX)
else()
  target_compile_options(foxlox PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
