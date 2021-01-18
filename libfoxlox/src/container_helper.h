#pragma once
#include <functional>

namespace foxlox
{
  template<typename T>
  class AllocatorWrapper
  {
  public:
    // typedefs
    using value_type = T;

    // constructor
    explicit AllocatorWrapper(
      std::function<char* (size_t)> alloc, 
      std::function<void(const char*, size_t)> dealloc) :
      allocator(alloc), deallocator(dealloc)
    {
    }

    template<typename U>
    explicit AllocatorWrapper(const AllocatorWrapper<U> & o)
    {
      allocator = o.allocator;
      deallocator = o.deallocator;
    }

    // memory allocation
    T* allocate(size_t n)
    {
      char* data = allocator(n * sizeof(T));
      return reinterpret_cast<T*>(data);
    }
    void deallocate(T* p, size_t n)
    {
      deallocator(reinterpret_cast<const char*>(p), n * sizeof(T));
    }

    bool operator==(const AllocatorWrapper & r)
    {
      return true; // I'm not sure if this is the right way to do this
    }
    bool operator!=(const AllocatorWrapper& r)
    {
      return !(*this == r);
    }

    template<typename U>
    friend class AllocatorWrapper;
  private:
    std::function<char*(size_t)> allocator;
    std::function<void(const char*, size_t)> deallocator;
  };
}