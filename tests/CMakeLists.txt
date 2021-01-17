# CMakeList.txt: ${PROJECT_NAME}lox 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.16)

project(tests)

enable_testing()

# 将源代码添加到此项目的可执行文件。
add_executable(${PROJECT_NAME})

add_dependencies(${PROJECT_NAME} libfoxlox)

target_sources(${PROJECT_NAME} PRIVATE
   "src/temp.cpp" 
   "src/variable.cpp" 
   "src/if.cpp" 
   "src/block.cpp" 
   "src/bool.cpp" 
   "src/nil.cpp" 
   "src/while.cpp" 
   "src/for.cpp" 
   "src/logical_operator.cpp" 
   "src/tuple.cpp"
   "src/closure.cpp" 
   "src/basic.cpp" 
   "src/class.cpp" 
   "src/regression.cpp")

target_link_libraries(${PROJECT_NAME} PRIVATE libfoxlox)

find_package(magic_enum CONFIG REQUIRED)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GTest::gtest GTest::gtest_main)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

add_test(NAME run_test COMMAND ${PROJECT_NAME})