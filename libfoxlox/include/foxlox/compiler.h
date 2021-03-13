#pragma once
#include <tuple>
#include <string_view>
#include <vector>

namespace foxlox
{
  enum class CompilerResult
  {
    OK,
    COMPILE_ERROR
  };
  std::tuple<CompilerResult, std::vector<char>> compile(std::string_view source);
}

