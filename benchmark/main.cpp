import <fstream>;
import <iostream>;
import <filesystem>;
import <vector>;
import <string>;
import <chrono>;
import <iostream>;
import <format>;

#include <range/v3/view/enumerate.hpp>

import foxlox;

int main(int argc, const char*)
{
  if (argc != 1)
  {
    std::cerr << "Unknown commandline arguments.\n";
    return 0;
  }
  while (true)
  {
    auto bench_path = std::filesystem::path(FOXLOX_CURRENT_SOURCE_DIR) / "bench";
    if (!is_directory(bench_path))
    {
      bench_path = std::filesystem::current_path() / "bench";
    }
    if (!is_directory(bench_path))
    {
      std::cerr << "Cannot find bench folder.\n";
      return 1;
    }
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
    std::cout << std::format("\t{}. {}\n", 0, "[ALL]");
    for (auto&& [i, name] : bench_names | ranges::views::enumerate)
    {
      std::cout << std::format("\t{}. {}\n", i + 1, name);
    }
    std::cout << std::format("\t{}. {}\n", bench_names.size() + 1, "[EXIT]");
    std::cout << "Please select: ";
    int selected;
    std::cin >> selected;
    if (selected == ssize(bench_names) + 1)
    {
      // exit
      return 0;
    }
    if (selected < 0 || selected >= ssize(bench_names) + 2)
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
        std::cout << std::format("=== {{Benchmark: {}}} ===\n", bench_names.at(selected - 1));
      }
      else
      {
        std::cout << std::format("Failed to open source file: {}.\n", bench_files.at(selected - 1).string());
      }
      std::string src{ std::istreambuf_iterator<char>(ifs),  std::istreambuf_iterator<char>() };

      const auto time_compile_start = std::chrono::system_clock::now().time_since_epoch();
      auto [res, chunk] = foxlox::compile(src);
      const auto time_compile_end = std::chrono::system_clock::now().time_since_epoch();
      const long long compile_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        time_compile_end - time_compile_start).count();
      std::cout << std::format("Compile used {}ms.\n", compile_time);

      std::cout << "Begin...\n";
      if (res == foxlox::CompilerResult::OK)
      {
        foxlox::VM vm;
        vm.run(chunk);
        std::cout << "Finished.\n\n";
      }
      else
      {
        std::cout << "Compilation failed.\n";
        return 0;
      }
    } while (run_all && ++selected <= ssize(bench_files));
    const auto time_end = std::chrono::system_clock::now().time_since_epoch();
    const long long total_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      time_end - time_start).count();
    std::cout << std::format("Total time used {}ms.\n", total_time);
  }
}