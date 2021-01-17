#include <algorithm>
#include <bit>

#include "string_pool.h"

namespace
{
  using namespace foxlox;

  bool str_equal(String* l, String* r)
  {
    return *l == *r;
  }
  bool str_equal(String* l, std::string_view r)
  {
    return l->get_view() == r;
  }
  bool str_equal(String* l, std::string_view r1, std::string_view r2)
  {
    return l->size() == (r1.size() + r2.size()) &&
      std::equal(r1.begin(), r1.end(), l->data()) &&
      std::equal(r2.begin(), r2.end(), l->data() + r1.size());
  }

  // FNV-1a hash
  // http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-1a
  uint64_t str_hash(std::string_view str)
  {
    uint64_t hash = 14695981039346656037ull;
    for(auto c: str)
    {
      hash ^= std::bit_cast<uint8_t>(c);
      hash *= 1099511628211ull;
    }
    return hash;
  }

  uint64_t str_hash(String* str)
  {
    return str_hash(str->get_view());
  }

  uint64_t str_hash(std::string_view str1, std::string_view str2)
  {
    uint64_t hash = 14695981039346656037ull;
    for (auto c : str1)
    {
      hash ^= std::bit_cast<uint8_t>(c);
      hash *= 1099511628211ull;
    }
    for (auto c : str2)
    {
      hash ^= std::bit_cast<uint8_t>(c);
      hash *= 1099511628211ull;
    }
    return hash;
  }
}
namespace foxlox
{
#ifdef DEBUG_LOG_GC
  std::cout << fmt::format("sweeping {} [{}]: {}\n", static_cast<const void*>(str), str->is_marked() ? "is_marked" : "not_marked", str->get_view());
#endif
  StringPool::~StringPool()
  {
    assert(false);
  }
}