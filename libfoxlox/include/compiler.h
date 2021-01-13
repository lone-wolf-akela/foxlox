#pragma once

#include <tuple>
#include <string_view>

#include <chunk.h>

namespace foxlox
{
  enum class CompilerResult
  {
    OK,
    COMPILE_ERROR
  };
  std::tuple<CompilerResult, Chunk> compile(std::string_view source);
}

