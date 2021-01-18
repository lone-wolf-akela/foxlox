#include <algorithm>
#include <bit>

#include <gsl/gsl>

#include "config.h"

#include "string_pool.h"

namespace
{
  using namespace foxlox;

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
  uint32_t str_hash(std::string_view str)
  {
    uint32_t hash = 2166136261u;
    for(auto c: str)
    {
      hash ^= std::bit_cast<uint8_t>(c);
      hash *= 16777619u;
    }
    return hash;
  }

  uint32_t str_hash(std::string_view str1, std::string_view str2)
  {
    uint32_t hash = 2166136261u;
    for (auto c : str1)
    {
      hash ^= std::bit_cast<uint8_t>(c);
      hash *= 16777619u;
    }
    for (auto c : str2)
    {
      hash ^= std::bit_cast<uint8_t>(c);
      hash *= 16777619u;
    }
    return hash;
  }
}
namespace foxlox
{
  StringPool::~StringPool()
  {
    for (auto& e : entries)
    {
      if (e.str != nullptr && !e.tombstone)
      {
        delete_entry(e);
      }
    }
  }
  String* StringPool::add_string(std::string_view str)
  {
    if (count + 1 > entries.size() * STRING_POOL_MAX_LOAD)
    {
      grow_capacity();
    }
    const auto hash = str_hash(str);
    gsl::index idx = hash % entries.size();
    StringPoolEntry* first_tombstone = nullptr;
    while (true)
    {
      if (entries.at(idx).tombstone)
      {
        if (first_tombstone == nullptr) { first_tombstone = &entries.at(idx); }
      }
      else if (entries.at(idx).str == nullptr)
      {
        String* p = String::alloc(allocator, str.size());
        std::copy(str.begin(), str.end(), p->data());
       
        if (first_tombstone == nullptr) { count++; }
        StringPoolEntry* entry_to_insert = first_tombstone ? first_tombstone : &entries.at(idx);
        entry_to_insert->hash = hash;
        entry_to_insert->str = p;
        entry_to_insert->tombstone = false;
        return p;
      }
      else if (str_equal(entries.at(idx).str, str))
      {
        return entries.at(idx).str;
      }
      idx = (idx + 1) % entries.size();
    }
  }
  String* StringPool::add_str_cat(std::string_view lhs, std::string_view rhs)
  {
    if (count + 1 > entries.size() * STRING_POOL_MAX_LOAD)
    {
      grow_capacity();
    }
    const auto hash = str_hash(lhs, rhs);
    gsl::index idx = hash % entries.size();
    StringPoolEntry* first_tombstone = nullptr;
    while (true)
    {
      if (entries.at(idx).tombstone)
      {
        if (first_tombstone == nullptr) { first_tombstone = &entries.at(idx); }
      }
      else if (entries.at(idx).str == nullptr)
      {
        String* p = String::alloc(allocator, lhs.size() + rhs.size());
        const auto it = std::copy(lhs.begin(), lhs.end(), p->data());
        std::copy(rhs.begin(), rhs.end(), it);

        if (first_tombstone == nullptr) { count++; }
        StringPoolEntry* entry_to_insert = first_tombstone ? first_tombstone : &entries.at(idx);
        entry_to_insert->hash = hash;
        entry_to_insert->str = p;
        entry_to_insert->tombstone = false;
        return p;
      }
      else if (str_equal(entries.at(idx).str, lhs, rhs))
      {
        return entries.at(idx).str;
      }
      idx = (idx + 1) % entries.size();
    }
  }
  void StringPool::sweep()
  {
    for (auto& e : entries)
    {
#ifdef DEBUG_LOG_GC
      if (e.str != nullptr && !e.tombstone)
      {
        std::cout << fmt::format("sweeping {} [{}]: {}\n", static_cast<const void*>(e.str), e.str->is_marked() ? "is_marked" : "not_marked", e.str->get_view());
      }
#endif
      if (e.str != nullptr && !e.tombstone && !e.str->is_marked())
      {
        delete_entry(e);
      }
    }
  }
  void StringPool::grow_capacity()
  {
    size_t new_count = 0;
    const size_t new_capacity = std::max<size_t>(HASH_TABLE_START_BUCKET, count * 2);
    std::vector<StringPoolEntry> new_entries(new_capacity);
    for (auto& e : entries)
    {
      if (e.tombstone || e.str == nullptr)
      {
        continue;
      }
      new_count++;
      gsl::index idx = e.hash % new_capacity;
      while (true)
      {
        if (new_entries.at(idx).str == nullptr)
        {
          new_entries.at(idx) = e;
          break;
        }
        idx = (idx + 1) % new_capacity;
      }
    }
    entries.swap(new_entries);
    count = new_count;
  }
  void StringPool::delete_entry(StringPoolEntry& e)
  {
    assert(e.str != nullptr && !e.tombstone);
    String::free(deallocator, e.str);
    e.tombstone = true;
    // tombstone still counts in count, so we do not count-- here
  }
}