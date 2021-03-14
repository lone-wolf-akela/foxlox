#include <sstream>

#include <range/v3/all.hpp>

#include "common.h"
#include "scanner.h"
#include "parser.h"
#include "codegen.h"
#include "resolver.h"
#include "object.h"

#include <foxlox/compiler.h>

namespace foxlox
{
  std::tuple<CompilerResult, std::vector<char>> compile(std::string_view source)
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
    auto chunk = codegen.gen();
    if (codegen.get_had_error())
    {
      return std::make_tuple(CompilerResult::COMPILE_ERROR, std::vector<char>{});
    }

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