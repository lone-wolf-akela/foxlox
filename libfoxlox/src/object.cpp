#include <fmt/format.h>

#include <foxlox/chunk.h>
#include <foxlox/except.h>

#include "object.h"

namespace foxlox
{
  GSL_SUPPRESS(type.6)
  String::String(size_t l) noexcept :
    SimpleObj(ObjType::STR, l)
  {
  }
  bool operator==(const String& l, const String& r) noexcept
  {
    return l.get_view() == r.get_view();
  }
  std::string_view String::get_view() const noexcept
  {
    return std::string_view(data(), size());
  }
  GSL_SUPPRESS(type.6)
  Tuple::Tuple(size_t l) noexcept : 
    SimpleObj(ObjType::TUPLE, l)
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
  Value Instance::get_property(gsl::not_null<String*> name)
  {
    if (auto func = klass->get_method(name); func.has_value())
    {
      GSL_SUPPRESS(lifetime.3)
      return Value(this, *func);
    }
    // return nil when the field is not found
    return fields.get_value(name).value_or(Value());
  }
  Value Instance::get_super_method(gsl::not_null<String*> name)
  {
    if (auto func = klass->get_super()->get_method(name); func.has_value())
    {
      GSL_SUPPRESS(lifetime.3)
      return Value(this, *func);
    }
    else
    {
      throw ValueError(fmt::format("Super class has no method with name `{}'", name->get_view()));
    }
  }
  HashTable<String*, Value>& Instance::get_hash_table() noexcept
  {
    return fields;
  }
  void Instance::set_property(gsl::not_null<String*> name, Value value)
  {
    if (klass->has_method(name))
    {
      throw ValueError("Attempt to rewrite class method. This is not allowed");
    }
    fields.set_entry(name, value);
  }
  bool Instance::is_marked() const noexcept
  { 
    return gc_mark;
  }
  void Instance::mark() noexcept
  {
    gc_mark = true;
  }
  void Instance::unmark() noexcept
  {
    gc_mark = false;
  }
  GSL_SUPPRESS(r.11) GSL_SUPPRESS(i.11)
  Class::Class(std::string_view name) : 
    ObjBase(ObjType::CLASS), 
    gc_mark(false),
    superclass(nullptr), 
    class_name(name),
    methods([](size_t l) {return new char[l]; }, [](char* p, size_t) {delete[] p; })
  {
  }
  void Class::add_method(String* name, Subroutine* func)
  {
    methods.set_entry(name, func);
  }
  void Class::set_super(gsl::not_null<Class*> super)
  {
    superclass = super;
    for(auto& entry : super->get_hash_table())
    {
      // if we already have a method with the same name,
      // do nothing (to shadow the base class method)
      methods.try_add_entry(entry.key, entry.value);
    }
  }
  Class* Class::get_super() noexcept
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
  HashTable<String*, Subroutine*>& Class::get_hash_table() noexcept
  {
    return methods;
  }
  bool Class::is_marked() const noexcept
  {
    return gc_mark;
  }
  void Class::mark() noexcept
  {
    gc_mark = true;
  }
  void Class::unmark() noexcept
  {
    gc_mark = false;
  }
  Value Dict::get(gsl::not_null<String*> name)
  {
    // return nil when the field is not found
    return fields.get_value(name).value_or(Value());
  }
  HashTable<Value, Value>& Dict::get_hash_table() noexcept
  {
    return fields;
  }
  void Dict::set(gsl::not_null<String*> name, Value value)
  {
    fields.set_entry(name, value);
  }
  bool Dict::is_marked() const noexcept
  {
    return gc_mark;
  }
  void Dict::mark() noexcept
  {
    gc_mark = true;
  }
  void Dict::unmark() noexcept
  {
    gc_mark = false;
  }
}
