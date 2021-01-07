#ifndef FOXLOX_EXCEPTION_H
#define FOXLOX_EXCEPTION_H

#include <stdexcept>

namespace foxlox
{
  class FatalError : public std::exception
  {
    using std::exception::exception;
  };
}
#endif