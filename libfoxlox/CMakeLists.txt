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
  "src/container_helper.cpp" 
  "src/cppinterop.cpp" 
  "src/string_pool.cpp")

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

find_package(Microsoft.GSL CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Microsoft.GSL::GSL)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

find_package(range-v3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE range-v3)

find_package(magic_enum CONFIG REQUIRED)

find_package(ICU REQUIRED COMPONENTS uc)
target_link_libraries(${PROJECT_NAME} PRIVATE ICU::uc)

find_package(mimalloc 1.6 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC mimalloc)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
if(MSVC)
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 
    "/O2 /DNDEBUG /Zi /Gy /DNDEBUG /Ob3" 
    CACHE STRING "compile flags for release build" 
    FORCE)
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO 
    "/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF" 
    CACHE STRING "link flags for release build" 
    FORCE)
  set(CMAKE_CXX_FLAGS_RELEASE 
    "/O2 /DNDEBUG /Zi /Gy /DNDEBUG /Ob3" 
    CACHE STRING "compile flags for release build" 
    FORCE)
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE 
    "/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF" 
    CACHE STRING "link flags for release build" 
    FORCE)
  target_compile_options(${PROJECT_NAME} PRIVATE /nologo /utf-8)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
  target_compile_options(${PROJECT_NAME} PUBLIC -Wno-unknown-pragmas -Wno-pedantic)
endif()
