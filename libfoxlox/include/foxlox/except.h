#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace foxlox
{
  class FatalError : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };
  class UnimplementedError : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };
  class InternalRuntimeError : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };
  class RuntimeLibError : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };
  class RuntimeError : public std::runtime_error
  {
  public:
    RuntimeError(std::string_view message, int line_num, std::string_view src_code);
    std::string msg;
    int line;
    std::string source;
  };
}
