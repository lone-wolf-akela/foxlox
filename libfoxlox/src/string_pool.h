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
      entries(HASH_TABLE_START_BUCKET)
    {
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
    std::vector<StringPoolEntry> entries;
  };
}