#include <fmt/format.h>

#include <foxlox/chunk.h>
#include <foxlox/except.h>

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
    if (auto found = fields.find(name); found != fields.end())
    {
      return found->second;
    }
    else
    {
      // return nil when the field is not found
      return Value();
    }
  }
  Value Instance::get_super_method(std::string_view name, Chunk& chunk)
  {
    if (auto [got, func_idx] = klass->get_super()->try_get_method_idx(name); got)
    {
      auto func = &chunk.get_subroutines()[func_idx];
      return Value(this, func);
    }
    else
    {
      throw ValueError(fmt::format("Super class has no method with name `{}'", name).c_str());
    }
  }
  Instance::Fields& Instance::get_all_fields()
  {
    return fields;
  }
  void Instance::set_property(std::string_view name, Value value)
  {
    if (klass->has_method(name))
    {
      throw ValueError("Attempt to rewrite class method. This is not allowed");
    }
    fields.insert_or_assign(name, value);
  }
  bool Instance::is_marked() 
  { 
    return gc_mark;
  }
  void Instance::mark()
  {
    gc_mark = true;
  }
  void Instance::unmark()
  {
    gc_mark = false;
  }
  Class::Class(std::string_view name) : 
    ObjBase(ObjType::CLASS), superclass(nullptr), class_name(name), gc_mark(false)
  {
  }
  void Class::add_method(std::string_view name, uint16_t func_idx)
  {
    methods.emplace(name, func_idx);
  }
  void Class::set_super(Class* super)
  {
    superclass = super;
    for (auto& [key, val] : super->methods)
    {
      // if we already have a method with the same name,
      // do nothing (to shadow the base class method)
      methods.try_emplace(key, val);
    }
  }
  Class* Class::get_super()
  {
    return superclass;
  }
  bool Class::has_method(std::string_view name)
  {
    return methods.contains(name);
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
  std::unordered_map<std::string_view, uint16_t>& Class::get_all_methods()
  {
    return methods;
  }
  bool Class::is_marked()
  {
    return gc_mark;
  }
  void Class::mark()
  {
    gc_mark = true;
  }
  void Class::unmark()
  {
    gc_mark = false;
  }
}