#include "scanner.h"
#include "parser.h"
#include "codegen.h"
#include "resolver.h"

#include <foxlox/compiler.h>

namespace foxlox
{
  std::tuple<CompilerResult, Chunk> compile(std::string_view source)
  {
    Scanner scanner(u8_to_u32(source));
    auto [tokens, src_per_line] = scanner.scan_tokens();

    Parser parser(std::move(tokens));
    auto ast = parser.parse();
    if (parser.get_had_error())
    {
      return std::make_tuple(CompilerResult::COMPILE_ERROR, Chunk());
    }

    Resolver resolver(std::move(ast));
    auto resolved_ast = resolver.resolve();
    if (resolver.get_had_error())
    {
      return std::make_tuple(CompilerResult::COMPILE_ERROR, Chunk());
    }

    CodeGen codegen(std::move(resolved_ast));
    auto chunk = codegen.gen();
    if (codegen.get_had_error())
    {
      return std::make_tuple(CompilerResult::COMPILE_ERROR, Chunk());
    }

    chunk.set_source(std::move(src_per_line));
    return std::make_tuple(CompilerResult::OK, std::move(chunk));
  }
}