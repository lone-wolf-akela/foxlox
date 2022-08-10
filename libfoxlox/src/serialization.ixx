module;
#include <gsl/gsl>
export module foxlox:serialization;

import <istream>;
import <ostream>;
import <string>;
import <string_view>;
import <climits>;
import <concepts>;
import <bit>;


import :except;

namespace foxlox
{
  void dump_uint(std::ostream& strm, std::unsigned_integral auto v)
  {
    for (size_t i = 0; i < sizeof(v); i++)
    {
      strm.put((v >> (i * CHAR_BIT)) & 0xff);
    }
  }

  template<std::unsigned_integral T>
  T load_uint(std::istream& strm)
  {
    T v = 0;
    for (size_t i = 0; i < sizeof(v); i++)
    {
      T byte = gsl::narrow_cast<uint8_t>(strm.get());
      v |= (byte << (i * CHAR_BIT));
    }
    return v;
  }

  export void dump_uint64(std::ostream& strm, uint64_t v)
  {
    dump_uint(strm, v);
  }

  export void dump_uint32(std::ostream& strm, uint32_t v)
  {
    dump_uint(strm, v);
  }

  export void dump_int64(std::ostream& strm, int64_t v)
  {
    dump_uint64(strm, std::bit_cast<uint64_t>(v));
  }

  export void dump_int32(std::ostream& strm, int32_t v)
  {
    dump_uint32(strm, std::bit_cast<uint32_t>(v));
  }

  export void dump_uint16(std::ostream& strm, uint16_t v)
  {
    dump_uint(strm, v);
  }

  export void dump_uint8(std::ostream& strm, uint8_t v)
  {
    dump_uint(strm, v);
  }

  export void dump_str(std::ostream& strm, std::string_view v)
  {
    dump_int64(strm, ssize(v));
    strm.write(v.data(), v.size());
  }

  export void dump_double(std::ostream& strm, double v)
  {
    char* bytes = reinterpret_cast<char*>(&v);
    for (size_t i = 0; i < sizeof(v); i++)
    {
      if constexpr (std::endian::native == std::endian::little)
      {
        strm.put(bytes[i]);
      }
      else
      {
        strm.put(bytes[sizeof(v) - i - 1]);
      }
    }
  }

  export uint64_t load_uint64(std::istream& strm)
  {
    return load_uint<uint64_t>(strm);
  }

  export uint32_t load_uint32(std::istream& strm)
  {
    return load_uint<uint32_t>(strm);
  }

  export int64_t load_int64(std::istream& strm)
  {
    return std::bit_cast<int64_t>(load_uint64(strm));
  }

  export int32_t load_int32(std::istream& strm)
  {
    return std::bit_cast<int32_t>(load_uint32(strm));
  }

  export uint16_t load_uint16(std::istream& strm)
  {
    return load_uint<uint16_t>(strm);
  }
  export uint8_t load_uint8(std::istream& strm)
  {
    return load_uint<uint8_t>(strm);
  }
  export std::string load_str(std::istream& strm)
  {
    int64_t len = load_int64(strm);
    std::string str(len, '\0');
    strm.read(str.data(), len);
    return str;
  }
  export double load_double(std::istream& strm)
  {
    double v = 0;
    char* bytes = reinterpret_cast<char*>(&v);
    for (size_t i = 0; i < sizeof(v); i++)
    {
      if constexpr (std::endian::native == std::endian::little)
      {
        bytes[i] = gsl::narrow_cast<char>(strm.get());
      }
      else
      {
        bytes[sizeof(v) - i - 1] = gsl::narrow_cast<char>(strm.get());
      }
    }
    return v;
  }
}
