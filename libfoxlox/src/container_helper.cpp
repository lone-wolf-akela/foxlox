#include <memory>
#include <type_traits>
#include <concepts>

#include "container_helper.h"

namespace foxlox
{
  using IntAllocator = AllocatorWrapper<int>;
  using AllocatorWrapperTraits = std::allocator_traits<IntAllocator>;
  static_assert(std::same_as<AllocatorWrapperTraits::value_type, int>);
  static_assert(std::same_as<AllocatorWrapperTraits::size_type, size_t>);

  // TODO: wait for msvc to support use require directly
  template<typename A>
  concept Allocatable = requires (A& alloc) {
    {std::allocator_traits<A>::allocate(alloc, size_t{})} ->
      std::same_as<typename std::allocator_traits<A>::pointer>;
  };
  static_assert(Allocatable<IntAllocator>);

  template<typename A>
  concept DeAalocatable = requires (A& alloc) {
    {std::allocator_traits<A>::deallocate(alloc, typename std::allocator_traits<A>::pointer{}, size_t{})} ->
      std::same_as<void>;
  };
  static_assert(DeAalocatable<IntAllocator>);

  static_assert(std::copyable<IntAllocator>);
  static_assert(std::constructible_from<IntAllocator, AllocatorWrapper<float>>);
}