#pragma once
#include <tuple>
#include <string_view>
#include <vector>
#include <filesystem>

namespace foxlox
{
  enum class CompilerResult
  {
    OK,
    COMPILE_ERROR
  };
  std::tuple<CompilerResult, std::vector<char>> compile(std::string_view source);
  std::tuple<CompilerResult, std::vector<char>> compile_file(const std::filesystem::path& path);
}

