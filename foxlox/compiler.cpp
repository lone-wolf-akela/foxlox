#include "scanner.h"
#include "parser.h"

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
    std::ignore = ast;
    return std::make_tuple(CompilerResult::COMPILE_ERROR, Chunk());
  }
}