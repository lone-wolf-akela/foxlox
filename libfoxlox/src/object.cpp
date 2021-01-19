#include <fmt/format.h>

#include <foxlox/chunk.h>
#include <foxlox/except.h>

#include "object.h"

namespace foxlox
{
  String::String(size_t l) noexcept :
    SimpleObj(ObjType::STR, l), m_data{}
  {
  }
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
  Tuple::Tuple(size_t l) noexcept : 
    SimpleObj(ObjType::TUPLE, l), m_data{} 
  {
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
  Value Instance::get_property(String* name)
  {
    if (auto func = klass->get_method(name); func.has_value())
    {
      return Value(this, *func);
    }
    if(auto value = fields.get_value(name); value.has_value())
    {
      return *value;
    }
    else
    {
      // return nil when the field is not found
      return Value();
    }
  }
  Value Instance::get_super_method(String* name)
  {
    if (auto func = klass->get_super()->get_method(name); func.has_value())
    {
      return Value(this, *func);
    }
    else
    {
      throw ValueError(fmt::format("Super class has no method with name `{}'", name->get_view()));
    }
  }
  HashTable<Value>& Instance::get_hash_table()
  {
    return fields;
  }
  void Instance::set_property(String* name, Value value)
  {
    if (klass->has_method(name))
    {
      throw ValueError("Attempt to rewrite class method. This is not allowed");
    }
    fields.try_add_entry(name, value);
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
    ObjBase(ObjType::CLASS), 
    superclass(nullptr), 
    class_name(name),
    methods([](size_t l) {return new char[l]; }, [](char* p, size_t) {delete[] p; }),
    gc_mark(false)
  {
  }
  void Class::add_method(String* name, Subroutine* func)
  {
    methods.set_entry(name, func);
  }
  void Class::set_super(Class* super)
  {
    superclass = super;
    for (
      auto entry = super->get_hash_table().first_entry(); 
      entry!=nullptr;
      entry = super->get_hash_table().next_entry(entry)
      )
    {
      // if we already have a method with the same name,
      // do nothing (to shadow the base class method)
      methods.try_add_entry(entry->str, entry->value);
    }
  }
  Class* Class::get_super()
  {
    return superclass;
  }
  bool Class::has_method(String* name)
  {
    return methods.get_value(name).has_value();
  }
  std::optional<Subroutine*> Class::get_method(String* name)
  {
    return methods.get_value(name);
  }
  HashTable<Subroutine*>& Class::get_hash_table()
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
