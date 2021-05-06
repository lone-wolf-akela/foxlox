import <sstream>;
import <fstream>;

#include "common.h"
#include "codegen.h"

#include <range/v3/all.hpp>

#include "scanner.h"
#include "parser.h"
#include "resolver.h"
#include "object.h"

#include <foxlox/compiler.h>

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
#if !defined(_MSC_VER) && (__GNUC__ <= 10)
#pragma message("using g++ <= 10, there's no std::ostringstream.view()")
    return std::make_tuple(CompilerResult::OK, strm.str() | ranges::to<std::vector<char>>);
#else
    return std::make_tuple(CompilerResult::OK, strm.view() | ranges::to<std::vector<char>>);
#endif
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