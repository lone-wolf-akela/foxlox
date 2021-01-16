#include <foxlox/chunk.h>

#include "object.h"

namespace foxlox
{
  String::TStrViewComp operator<=>(const String& l, const String& r)
  {
#if __cpp_lib_three_way_comparison >= 201907L
#pragma message("We get __cpp_lib_three_way_comparison support!")
    return l.get_view() <=> r.get_view();
#else
#pragma message("No __cpp_lib_three_way_comparison support!")
    const auto res = l.get_view().compare(r.get_view());
    if (res < 0) { return std::weak_ordering::less; }
    if (res == 0) { return std::weak_ordering::equivalent; }
    return std::weak_ordering::greater;
#endif
  }
  bool operator==(const String& l, const String& r)
  {
    return l.get_view() == r.get_view();
  }
  std::string_view String::get_view() const noexcept
  {
    return std::string_view(data(), size());
  }
  std::span<const Value> Tuple::get_span() const noexcept
  {
    GSL_SUPPRESS(bounds.3)
      return std::span{ data(), size() };
  }
  std::span<Value> Tuple::get_span() noexcept
  {
    GSL_SUPPRESS(bounds.3)
      return std::span{ data(), size() };
  }
  Class* Instance::get_class() const noexcept { return klass; }
  Value Instance::get_property(std::string_view name, Chunk& chunk)
  {
    if (auto [got, func_idx] = klass->try_get_method_idx(name); got)
    {
      auto func = &chunk.get_subroutines()[func_idx];
      return Value(this, func);
    }
    // TODO: error handling
    return fields.at(name);
  }
  void Instance::set_property(std::string_view name, Value value)
  {
    // TODO: look at klass
    // TODO: error handling
    fields.emplace(name, value);
  }
  Class::Class(std::string_view name) : ObjBase(ObjType::CLASS), class_name(name)
  {
  }
  void Class::add_method(std::string_view name, uint16_t func_idx)
  {
    methods[name] = func_idx;
  }
  std::pair<bool, uint16_t> Class::try_get_method_idx(std::string_view name)
  {
    const auto found = methods.find(name);
    if (found == methods.end())
    {
      return std::make_pair(false, uint16_t{});
    }
    return std::make_pair(true, found->second);
  }
}