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

OPTION(FOXLOX_ENABLE_ASAN "Use addressSanitizer" OFF)

find_package(Microsoft.GSL CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Microsoft.GSL::GSL)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

find_package(range-v3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE range-v3)

find_package(magic_enum CONFIG REQUIRED)

find_package(ICU REQUIRED COMPONENTS uc)
target_link_libraries(${PROJECT_NAME} PRIVATE ICU::uc)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
if(MSVC)
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 
      "/O2 /DNDEBUG /Zi /Gy /Ob2" 
      CACHE STRING "compile flags for release build" 
      FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE 
      "/O2 /DNDEBUG /Zi /Gy /Ob2" 
      CACHE STRING "compile flags for release build" 
      FORCE)
  else()
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 
      "/O2 /DNDEBUG /Zi /Gy /Ob3" 
      CACHE STRING "compile flags for release build" 
      FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE 
      "/O2 /DNDEBUG /Zi /Gy /Ob3" 
      CACHE STRING "compile flags for release build" 
      FORCE)
  endif()
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO 
    "/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF" 
    CACHE STRING "link flags for release build" 
    FORCE)
  
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE 
    "/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF" 
    CACHE STRING "link flags for release build" 
    FORCE)
  target_compile_options(${PROJECT_NAME} PRIVATE /nologo /utf-8)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
  target_compile_options(${PROJECT_NAME} PRIVATE /wd4068) #  warning C4068: 未知的杂注“GCC” 
else()
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 
    "-O3 -g -DNDEBUG" 
    CACHE STRING "compile flags for release build" 
    FORCE)
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
  target_compile_options(${PROJECT_NAME} PUBLIC -Wno-unknown-pragmas -Wno-pedantic -Wno-attributes)
endif()

if(FOXLOX_ENABLE_ASAN)
  target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
  target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
  message(WARNING "Enable Asan. Only use this build for tests!")
endif()