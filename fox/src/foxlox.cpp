#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <fmt/format.h>

#include <foxlox/debug.h>
#include <foxlox/chunk.h>
#include <foxlox/vm.h>
#include <foxlox/compiler.h>
#include <foxlox/foxexcept.h>

using namespace foxlox;

void run(std::string_view source, VM& vm)
{
  auto [compile_result, chunk] = compile(source);
  if (compile_result != CompilerResult::OK)
  {
    std::exit(65);
  }
  try
  {
    vm.interpret(chunk);
  }
  catch (RuntimeError& e)
  {
    fmt::print(stderr, "{}\n", e.what());
    std::exit(70);
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
    run(line, vm);
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
  run(content, vm);
}

int main(int argc, const char* argv[])
{
  VM vm;

#ifdef _DEBUG
  std::ignore = argc;
  std::ignore = argv;
  const auto code = R"(
var a = 1;
)";
  run(code, vm);
  return 0;
#else
  if (argc == 2)
  {
    run_file(argv[1], vm);
  }
  else if (argc == 1)
  {
    run_prompt(vm);
  }
  else
  {
    fmt::print(stderr, "Usage: fox [script]\n");
    std::exit(64);
  }
  return 0;
#endif
}
