import <cstdio>;
import <cstdlib>;
import <string>;
import <string_view>;
import <iostream>;
import <fstream>;
import <filesystem>;

import foxlox.except;

#include <foxlox/debug.h>
#include <foxlox/chunk.h>
#include <foxlox/vm.h>
#include <foxlox/compiler.h>

#include <fmt/format.h>

using namespace foxlox;

void run(std::string_view source, VM& vm, bool exit_on_error)
{
  auto [compile_result, chunk] = compile(source);
  if (compile_result != CompilerResult::OK)
  {
    if (exit_on_error)
    {
      std::exit(65);
    }
    else
    {
      return;
    }
  }
  try
  {
    vm.run(chunk);
  }
  catch (RuntimeError& e)
  {
    fmt::print(stderr, "{}\n", e.what());
    if (exit_on_error)
    {
      std::exit(70);
    }
    else
    {
      return;
    }
  }
}

void run_prompt(VM& vm)
{
  while (true)
  {
    fmt::print("> ");
    std::string line;
    std::getline(std::cin, line);
    if (std::cin.eof())
    {
      break;
    }
    run(line, vm, false);
  }
}

void run_file(const std::filesystem::path& path, VM& vm)
{
  std::ifstream ifs(path);
  if (!ifs)
  {
    fmt::print(stderr, "Could not open file \"{}\".\n", path.string());
    std::exit(74);
  }
  std::string content
  {
    std::istreambuf_iterator<char>(ifs),
    std::istreambuf_iterator<char>()
  };
  ifs.close();
  run(content, vm, true);
}

int main(int argc, const char* argv[])
{
  VM vm;
  if (argc == 2)
  {
    run_file(argv[1], vm);
  }
  else if (argc == 1)
  {
    fmt::print("prompt mode not implemented.\n");
    std::exit(64);
    // run_prompt(vm);
  }
  else
  {
    fmt::print(stderr, "Usage: fox [script]\n");
    std::exit(64);
  }
  return 0;
}
