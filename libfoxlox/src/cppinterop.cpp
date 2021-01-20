#include <foxlox/except.h>

#include <foxlox/cppinterop.h>

namespace foxlox
{
  FoxValue to_variant(const Value& v)
  {
    switch (v.type)
    {
    case ValueType::BOOL:
      return v.v.b;
    case ValueType::F64:
      return v.v.f64;
    case ValueType::I64:
      return v.v.i64;
    case ValueType::FUNC:
      return v.v.func;
    case ValueType::CPP_FUNC:
      return v.v.cppfunc;
    case ValueType::METHOD:
      return std::make_pair(v.v.instance, v.method_func());
    case ValueType::OBJ:
    {
      if (v.v.obj == nullptr)
      {
        return nil;
      }
      switch (v.v.obj->type)
      {
      case ObjType::STR:
        return v.v.str->get_view();
      case ObjType::TUPLE:
        return v.v.tuple->get_span();
      case ObjType::CLASS:
        return v.v.klass;
      case ObjType::INSTANCE:
        return v.v.instance;
      default:
        throw FatalError("Unknown object type.");
      }
    }
    default:
      throw FatalError("Unknown value type.");
    }
  }

  bool operator==(const nil_t&, const nil_t&) noexcept 
  { 
    return true; 
  }
  TupleSpan::TupleSpan(const std::span<Value>& r) noexcept : std::span<Value>(r) {}
  TupleSpan::TupleSpan(std::span<Value>&& r) noexcept : std::span<Value>(std::move(r)) {}
  bool operator==(const TupleSpan& l, const TupleSpan& r) noexcept
  {
    return (l.data() == r.data()) && (l.size() == r.size());
  }
  FoxValue::FoxValue(const std::string& s) noexcept : FoxValueBase(std::string_view(s)) {}
  FoxValue::FoxValue(const char* s) noexcept : FoxValueBase(std::string_view(s)) {}
}