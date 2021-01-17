#pragma once
#include <string>
#include <span>
#include <functional>

#include "object.h"

namespace foxlox
{
  class StringPool
  {
  public:
    template<Allocator A, Deallocator D>
    StringPool(A alloc, D dealloc) :
      allocator(alloc), deallocator(dealloc)
    {
    }

    ~StringPool();
    String* add_string(std::string_view str);
    String* add_str_cat(std::string_view lhs, std::string_view rhs);
    std::span<String*> get_all_strings() const noexcept;
    void sweep();
  private:
    std::function<char* (size_t)> allocator;
    std::function<void(const char*, size_t)> deallocator;
  };
}