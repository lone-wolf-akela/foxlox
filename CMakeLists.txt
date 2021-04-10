# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file")
set(VCPKG_FEATURE_FLAGS versions)
project(foxlox)

OPTION(FOXLOX_MSVC_PROFILING "Slower but easy to do profiling mode" OFF)
OPTION(FOXLOX_ENABLE_ASAN "Use addressSanitizer" OFF)

if(MSVC)
  if(FOXLOX_MSVC_PROFILING)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 
      "/O1 /DNDEBUG /Zi /Gy /Ob1" 
      CACHE STRING "compile flags for release build" 
      FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE 
      "/O1 /DNDEBUG /Zi /Gy /Ob1" 
      CACHE STRING "compile flags for release build" 
      FORCE)
  else() # !FOXLOX_MSVC_PROFILING
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
      set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 
        "/O2 /DNDEBUG /Zi /Gy /Ob2" 
        CACHE STRING "compile flags for release build" 
        FORCE)
      set(CMAKE_CXX_FLAGS_RELEASE 
        "/O2 /DNDEBUG /Zi /Gy /Ob2" 
        CACHE STRING "compile flags for release build" 
        FORCE)
    else() # !Clang
      set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 
        "/O2 /DNDEBUG /Zi /Gy /Ob3" 
        CACHE STRING "compile flags for release build" 
        FORCE)
      set(CMAKE_CXX_FLAGS_RELEASE 
        "/O2 /DNDEBUG /Zi /Gy /Ob3" 
        CACHE STRING "compile flags for release build" 
        FORCE)
    endif() # Clang
  endif() # FOXLOX_MSVC_PROFILING
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO 
    "/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF" 
    CACHE STRING "link flags for release build" 
    FORCE)
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE 
    "/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF" 
    CACHE STRING "link flags for release build" 
    FORCE)
else()
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 
    "-O3 -g -DNDEBUG" 
    CACHE STRING "compile flags for release build" 
    FORCE)
endif()

if(FOXLOX_ENABLE_ASAN)
  STRING (REGEX REPLACE "/RTC[^ ]*" "" CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}") 
endif()

# 包含子项目。
add_subdirectory("libfoxlox")
add_subdirectory("fox")
add_subdirectory("tests")
add_subdirectory("benchmark")