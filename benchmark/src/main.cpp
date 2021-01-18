#include <cassert>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <chrono>

#include <fmt/format.h>
#include <range/v3/all.hpp>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>


int main()
{
  while (true)
  {
    const auto bench_path = std::filesystem::path(__FILE__).parent_path() / "bench";
    std::vector<std::string> bench_names;
    std::vector<std::filesystem::path> bench_files;

    for (auto& p : std::filesystem::directory_iterator(bench_path))
    {
      if (p.is_regular_file() && p.path().extension() == ".fox")
      {
        bench_names.emplace_back(p.path().stem().string());
        bench_files.emplace_back(p.path());
      }
    }

    std::cout << "Available benches:\n";
    std::cout << fmt::format("\t{}. {}\n", 0, "[ALL]");
    for (auto&& [i, name] : bench_names | ranges::views::enumerate)
    {
      std::cout << fmt::format("\t{}. {}\n", i + 1, name);
    }
    std::cout << fmt::format("\t{}. {}\n", bench_names.size() + 1, "[EXIT]");
    std::cout << "Please select: ";
    int selected;
    std::cin >> selected;
    if (selected == bench_names.size() + 1)
    {
      return 0;
    }
    if (selected < 0 || selected >= bench_names.size() + 2)
    {
      continue;
    }
    const bool run_all = selected == 0;
    if (run_all)
    {
      selected = 1;
    }
    const auto time_start = std::chrono::system_clock::now().time_since_epoch();
    do
    {
      std::ifstream ifs(bench_files.at(selected - 1));
      if (ifs)
      {
        std::cout << fmt::format("=== {{Benchmark: {}}} ===\n", bench_names.at(selected - 1));
      }
      else
      {
        std::cout << fmt::format("Failed to open source file: {}.\n", bench_files.at(selected - 1).string());
      }
      std::string src{ std::istreambuf_iterator<char>(ifs),  std::istreambuf_iterator<char>() };

      const auto time_compile_start = std::chrono::system_clock::now().time_since_epoch();
      auto [res, chunk] = foxlox::compile(src);
      const auto time_compile_end = std::chrono::system_clock::now().time_since_epoch();
      const long long compile_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        time_compile_end - time_compile_start).count();
      std::cout << fmt::format("Compile used {}ms.\n", compile_time);

      std::cout << "Begin...\n";
      if (res == foxlox::CompilerResult::OK)
      {
        foxlox::VM vm;
        vm.interpret(chunk);
        std::cout << "Finished.\n\n";
      }
      else
      {
        std::cout << "Compilation failed.\n";
        return 0;
      }
    } while (run_all && ++selected <= bench_files.size());
    const auto time_end = std::chrono::system_clock::now().time_since_epoch();
    const long long total_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      time_end - time_start).count();
    std::cout << fmt::format("Total time used {}ms.\n", total_time);
  }
}