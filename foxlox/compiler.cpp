#include "scanner.h"

#include "compiler.h"

namespace foxlox
{
  std::tuple<CompilerResult, Chunk> compile(std::string_view source)
  {
    Scanner scanner(u8_to_u32(source));
    auto tokens = scanner.scan_tokens();
    std::ignore = tokens;
    return std::tuple<CompilerResult, Chunk>();
  }
}