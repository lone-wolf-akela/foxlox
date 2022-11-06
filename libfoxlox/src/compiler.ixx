module;
export module foxlox:compiler;

import <tuple>;
import <string_view>;
import <vector>;
import <filesystem>;
import <sstream>;
import <fstream>;

import :config;
import :codegen;
import :scanner;
import :parser;
import :resolver;
import :value;

namespace foxlox
{
  export enum class CompilerResult
  {
    OK,
    COMPILE_ERROR
  };
  export std::tuple<CompilerResult, std::vector<char>> compile(std::string_view source);
  export std::tuple<CompilerResult, std::vector<char>> compile_file(const std::filesystem::path& path);
}

namespace
{
  using namespace foxlox;
  std::tuple<CompilerResult, std::vector<char>> compile_impl(std::string_view source, std::string_view src_path, std::string_view src_name)
  {
    Scanner scanner(u8_to_u32(source));
    auto [tokens, src_per_line] = scanner.scan_tokens();

    Parser parser(std::move(tokens));
    auto ast = parser.parse();
    if (parser.get_had_error())
    {
      return std::make_tuple(CompilerResult::COMPILE_ERROR, std::vector<char>{});
    }

    Resolver resolver(std::move(ast));
    auto resolved_ast = resolver.resolve();
    if (resolver.get_had_error())
    {
      return std::make_tuple(CompilerResult::COMPILE_ERROR, std::vector<char>{});
    }

    CodeGen codegen(std::move(resolved_ast));
    auto chunk = codegen.gen(src_name);
    if (codegen.get_had_error())
    {
      return std::make_tuple(CompilerResult::COMPILE_ERROR, std::vector<char>{});
    }

    chunk.set_src_path(src_path);
    chunk.set_source(std::move(src_per_line));
    std::ostringstream strm;
    strm.write(BINARY_HEADER.data(), BINARY_HEADER.size());
    chunk.dump(strm);
    
    std::vector<char> result(strm.view().size());
    std::ranges::copy(strm.view(), begin(result));
    return std::make_tuple(CompilerResult::OK, result);
  }
}

namespace foxlox
{
  std::tuple<CompilerResult, std::vector<char>> compile(std::string_view source)
  {
    return compile_impl(source, ".", "script");
  }

  std::tuple<CompilerResult, std::vector<char>> compile_file(const std::filesystem::path& path)
  {
    std::ifstream ifs(path);
    if (!ifs)
    {
      return std::make_tuple(CompilerResult::COMPILE_ERROR, std::vector<char>{});
    }
    std::string str(std::istreambuf_iterator<char>{ifs}, {});
    ifs.close();
    return compile_impl(str, path.string(), path.stem().string());
  }
}