#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <string_view>

namespace foxlox
{
  void dump_int64(std::ostream& strm, int64_t v);
  void dump_int32(std::ostream& strm, int32_t v);
  void dump_uint64(std::ostream& strm, uint64_t v);
  void dump_uint32(std::ostream& strm, uint32_t v);
  void dump_uint16(std::ostream& strm, uint16_t v);
  void dump_uint8(std::ostream& strm, uint8_t v);
  void dump_str(std::ostream& strm, std::string_view v);
  void dump_double(std::ostream& strm, double v);
  int64_t load_int64(std::istream& strm);
  int32_t load_int32(std::istream& strm);
  uint64_t load_uint64(std::istream& strm);
  uint32_t load_uint32(std::istream& strm);
  uint16_t load_uint16(std::istream& strm);
  uint8_t load_uint8(std::istream& strm);
  std::string load_str(std::istream& strm);
  double load_double(std::istream& strm);
}