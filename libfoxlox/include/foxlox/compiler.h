#pragma once
import <tuple>;
import <string_view>;
import <vector>;
import <filesystem>;

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

