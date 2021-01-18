#pragma once
#include <string>
#include <string_view>
#include <functional>
#include <vector>

#include "object.h"

namespace foxlox
{
  struct StringPoolEntry
  {
    uint32_t hash = 0;
    bool tombstone = false;
    String* str = nullptr;
  };

  class StringPool
  {
  public:
    template<Allocator A, Deallocator D>
    StringPool(A alloc, D dealloc) :
      allocator(alloc), 
      deallocator(dealloc), 
      count(0), 
      capacity_mask(HASH_TABLE_START_BUCKET - 1),
      entries(HASH_TABLE_START_BUCKET)
    {
      // make sure HASH_TABLE_START_BUCKET is power of 2
      // otherwise capacity_mask won't work
      static_assert((HASH_TABLE_START_BUCKET & (HASH_TABLE_START_BUCKET - 1)) == 0);
    }

    ~StringPool();
    String* add_string(std::string_view str);
    String* add_str_cat(std::string_view lhs, std::string_view rhs);
    void sweep();
  private:
    void grow_capacity();
    void delete_entry(StringPoolEntry& e);

    std::function<char* (size_t)> allocator;
    std::function<void(const char*, size_t)> deallocator;
    size_t count;
    size_t capacity_mask;
    std::vector<StringPoolEntry> entries;
  };
}