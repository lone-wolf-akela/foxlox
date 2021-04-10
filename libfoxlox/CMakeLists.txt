# CMakeList.txt: ${PROJECT_NAME}lox 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.16)

project(libfoxlox)

# 将源代码添加到此项目的可执行文件。
add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
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
  "src/codegen.cpp"
  "src/resolver.cpp"
  "src/common.cpp"
  "src/except.cpp"
  "src/object.cpp"
  "src/runtimelib.h"
  "src/runtimelib.cpp"
  "src/cppinterop.cpp"
  "src/hash_table.cpp"
  "src/mem_alloc.cpp" 
  "src/serialization.cpp")

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

OPTION(FOXLOX_USE_MIMALLOC "Use Mimalloc lib for VM heap allocator" ON)
if(FOXLOX_USE_MIMALLOC)
  target_compile_definitions(${PROJECT_NAME} PRIVATE FOXLOX_USE_MIMALLOC)
  find_package(mimalloc 1.6 CONFIG REQUIRED)
  target_link_libraries(${PROJECT_NAME} PUBLIC mimalloc)
endif()

OPTION(FOXLOX_DEBUG_STRESS_GC "Trigger GC after every INST for debugging" OFF)
if(FOXLOX_DEBUG_STRESS_GC)
  target_compile_definitions(${PROJECT_NAME} PRIVATE FOXLOX_DEBUG_STRESS_GC)
endif()

OPTION(FOXLOX_ENABLE_ASAN "Use addressSanitizer" OFF)

find_package(Microsoft.GSL CONFIG REQUIRED)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt-header-only)

find_package(range-v3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC range-v3)

find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC magic_enum::magic_enum)

find_package(ICU REQUIRED COMPONENTS uc dt)
target_link_libraries(${PROJECT_NAME} PUBLIC ICU::uc ICU::dt)

find_package(Boost REQUIRED COMPONENTS filesystem)
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::filesystem)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /nologo /utf-8)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
  target_compile_options(${PROJECT_NAME} PRIVATE /wd4068) #  warning C4068: 未知的杂注“GCC” 
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
  target_compile_options(${PROJECT_NAME} PUBLIC -Wno-unknown-pragmas -Wno-pedantic -Wno-attributes)
endif()

if(FOXLOX_ENABLE_ASAN)
  STRING (REGEX REPLACE "/RTC[^ ]*" "" CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}") 
  target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
  if(NOT MSVC OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
  endif()
  message(WARNING "Enable Asan. Only use this build for tests!")
endif()