#pragma once

#include <istream>
#include <ostream>

namespace foxlox
{
  void dump_int64(std::ostream& strm, int64_t v);
  void dump_int32(std::ostream& strm, int32_t v);
  int64_t load_int64(std::istream& strm);
  int32_t load_int32(std::istream& strm);
}