#include "scanner.h"
#include "parser.h"
#include "codegen.h"

#include "compiler.h"

namespace foxlox
{
  std::tuple<CompilerResult, Chunk> compile(std::string_view source)
  {
    Scanner scanner(u8_to_u32(source));
    Parser parser(scanner.scan_tokens());
    auto ast = parser.parse();
    if (parser.get_had_error())
    {
      return std::make_tuple(CompilerResult::COMPILE_ERROR, Chunk());
    }
    CodeGen codegen(std::move(ast));
    auto chunk = codegen.gen();
    return std::make_tuple(CompilerResult::OK, std::move(chunk));
  }
}