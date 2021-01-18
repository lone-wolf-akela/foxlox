#include <fmt/format.h>

#include <foxlox/except.h>

namespace foxlox
{
  RuntimeError::RuntimeError(std::string_view message, int line_num, std::string_view src_code) :
    std::runtime_error(fmt::format("[line {}] at \"{}\"\n{}", line_num, src_code, message)),
    msg(message),
    line(line_num),
    source(src_code)
  {
  }
}
