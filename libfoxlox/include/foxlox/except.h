#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace foxlox
{
  class FatalError : public std::exception
  {
  public:
    using std::exception::exception;
  };
  class UnimplementedError : public std::exception
  {
  public:
    using std::exception::exception;
  };
  class InternalRuntimeError : public std::exception
  {
  public:
    using std::exception::exception;
  };
  class RuntimeLibError : public std::exception
  {
  public:
    using std::exception::exception;
  };
  class RuntimeError : public std::exception
  {
  public:
    RuntimeError(std::string_view message, int line_num, std::string_view src_code);
    std::string msg;
    int line;
    std::string source;
  };
}
