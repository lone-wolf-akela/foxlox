module;
export module foxlox:cppinterop;

import <cstdint>;
import <variant>;
import <string>;
import <string_view>;
import <utility>;
import <variant>;
import <concepts>;

import <gsl/gsl>;

import :value;
import :object;
import :chunk;
import :except;

namespace foxlox
{
  export struct nil_t
  {
    friend bool operator==(const nil_t&, const nil_t&) noexcept
    {
      return true;
    }
  };
  export const nil_t nil;

  export struct TupleSpan : public std::span<Value>
  {
    TupleSpan(const std::span<Value>& r) noexcept : std::span<Value>(r) {}
    TupleSpan(std::span<Value>&& r) noexcept : std::span<Value>(std::move(r)) {}
    friend bool operator==(const TupleSpan& l, const TupleSpan& r) noexcept
    {
      return (l.data() == r.data()) && (l.size() == r.size());
    }
  };

  using FoxValueVariant = std::variant<
    nil_t, // NIL
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

  export class FoxValue
  {
  public:
    FoxValue() noexcept : value(nil) {}
    FoxValue(const char* s) noexcept : value(std::string_view(s)) {}
    FoxValue(const std::string& s) noexcept : value(std::string_view(s)) {}
    template<typename T>
    FoxValue(const T& v) noexcept : value(v) {}
    template<>
    FoxValue(const Value& v)
    {
      switch (v.type)
      {
      case ValueType::NIL:
        value = nil;
        return;
      case ValueType::BOOL:
        value = v.v.b;
        return;
      case ValueType::F64:
        value = v.v.f64;
        return;
      case ValueType::I64:
        value = v.v.i64;
        return;
      case ValueType::FUNC:
        value = v.v.func;
        return;
      case ValueType::CPP_FUNC:
        value = v.v.cppfunc;
        return;
      case ValueType::METHOD:
        value = std::make_pair(v.v.instance, v.method_func());
        return;
      case ValueType::OBJ:
      {
        Expects(v.v.obj != nullptr);
        switch (v.v.obj->type)
        {
        case ObjType::STR:
          value = v.v.str->get_view();
          return;
        case ObjType::TUPLE:
          value = v.v.tuple->get_span();
          return;
        case ObjType::CLASS:
          value = v.v.klass;
          return;
        case ObjType::INSTANCE:
          value = v.v.instance;
          return;
        default:
          throw FatalError("Unknown object type.");
        }
      }
      default:
        throw FatalError("Unknown value type.");
      }
    }
    template<typename T>
    bool is() const
    {
      return std::holds_alternative<T>(value);
    }
    friend auto operator==(const FoxValue& l, const FoxValue& r)
    {
      return l.value == r.value;
    }
    template<typename T>
    requires !std::same_as<T, FoxValue>
    friend bool operator==(const FoxValue& l, const T& r)
    {
      return l == FoxValue(r);
    }
    FoxValue operator[](size_t idx) const
    {
      if (is<TupleSpan>())
      {
        return FoxValue(std::get<TupleSpan>(value)[idx]);
      }
      throw FatalError("Not an indexable type.");
    }

    template<typename T>
    auto get()
    {
      return std::get<T>(value);
    }

    size_t size() const
    {
      if (is<TupleSpan>())
      {
        return std::get<TupleSpan>(value).size();
      }
      else if (is<std::string_view>())
      {
        return std::get<std::string_view>(value).size();
      }
      throw FatalError("Not a sizable type.");
    }

    auto ssize() const
    {
      return std::ssize(*this);
    }
  private:
    FoxValueVariant value;
  };
}