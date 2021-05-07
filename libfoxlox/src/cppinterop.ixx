module;
export module foxlox:cppinterop;

import <variant>;
import <string>;
import <string_view>;
import <utility>;

import <gsl/gsl>;

import :value;
import :chunk;
import :except;

namespace foxlox
{
  export struct nil_t 
  {
    friend bool operator==(const nil_t&, const nil_t&) noexcept;
  };
  export inline nil_t nil;

  export struct TupleSpan : public std::span<Value>
  {
    TupleSpan(const std::span<Value>& r) noexcept;
    TupleSpan(std::span<Value>&& r) noexcept;
    friend bool operator==(const TupleSpan& l, const TupleSpan& r) noexcept;
  };

  export constexpr int64_t operator"" _i64(unsigned long long int i)
  {
    return i;
  }

  export using FoxValueBase = std::variant<
    decltype(nil), // NIL
    bool, // BOOL
    int64_t, // I64
    double, // F64
    Subroutine*, // FUNC,
    CppFunc*, // CPP_FUNC
    std::pair<Instance*, Subroutine*>, //METHOD
    std::string_view, // STR
    TupleSpan, // TUPLE
    Class*, //CLASS
    Instance* //INSTANCE
  >;

  export class FoxValue : public FoxValueBase
  {
  public:
    using FoxValueBase::FoxValueBase;
    FoxValue(const std::string& s) noexcept;
    FoxValue(const char* s) noexcept;
  };

  export FoxValue to_variant(const Value& v);
}

namespace foxlox
{
  FoxValue to_variant(const Value& v)
  {
    switch (v.type)
    {
    case ValueType::NIL:
      return nil;
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
      Expects(v.v.obj != nullptr);
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