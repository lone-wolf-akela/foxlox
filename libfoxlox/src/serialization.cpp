#include <foxlox/except.h>

#include "serialization.h"

void foxlox::dump_int64(std::ostream& /*strm*/, int64_t /*v*/)
{
  throw UnimplementedError("");
}

void foxlox::dump_int32(std::ostream& /*strm*/, int32_t /*v*/)
{
  throw UnimplementedError("");
}

int64_t foxlox::load_int64(std::istream& /*strm*/)
{
  throw UnimplementedError("");
}

int32_t foxlox::load_int32(std::istream& /*strm*/)
{
  throw UnimplementedError("");
}
