# CMakeList.txt: ${PROJECT_NAME}lox 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.16)

project(libfoxlox)

# 将源代码添加到此项目的可执行文件。
add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
  "src/mimalloc-new-delete.cpp"
  "src/config.cpp"
  "src/debug.cpp" 
  "src/chunk.cpp" 
  "src/value.cpp" 
  "src/vm.cpp" 
  "src/compiler.cpp" 
  "src/scanner.cpp" 
  "src/token.cpp"
  "src/util.cpp" 
  "src/compiletime_value.cpp"
  "src/stmt.cpp" 
  "src/expr.cpp" 
  "src/parser.cpp" 
  "src/opcode.cpp" 
  "src/codegen.cpp")

target_include_directories(${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-config
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(EXPORT ${PROJECT_NAME}-config
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

find_package(Microsoft.GSL CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft.GSL::GSL)

find_package(mimalloc 1.6 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE mimalloc)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

find_package(range-v3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE range-v3)

find_package(magic_enum CONFIG REQUIRED)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
