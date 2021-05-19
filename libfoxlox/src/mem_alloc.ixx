export module foxlox:mem_alloc;

#ifdef FOXLOX_USE_MIMALLOC
import <mimalloc.h>;
namespace foxlox
{
  void* MALLOC(std::size_t size)
  {
    return mi_malloc(size);
  }
  void FREE(void* ptr)
  {
    mi_free(ptr);
  }
}
#else
import <cstdlib>;
namespace foxlox
{
  void* MALLOC(std::size_t size)
  {
    return std::malloc(size);
  }
  void FREE(void* ptr)
  {
    std::free(ptr);
  }
}
#endif