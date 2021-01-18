#pragma once

#include <variant>
#include <string>
#include <string_view>
#include <utility>

#include "../../src/value.h"
#include "../../src/object.h"

namespace foxlox
{
  struct nil_t 
  {
    friend bool operator==(const nil_t&, const nil_t&) { return true; }
  };
  inline nil_t nil;

  struct TupleSpan : public std::span<Value>
  {
    TupleSpan(const std::span<Value>& r) : std::span<Value>(r) {}
    TupleSpan(std::span<Value>&& r) noexcept : std::span<Value>(std::move(r)) {}

    friend bool operator==(const TupleSpan& l, const TupleSpan& r)
    {
      return (l.data() == r.data()) && (l.size() == r.size());
    }
  };

  constexpr int64_t operator"" _i64(unsigned long long int i)
  {
    return i;
  }

  using FoxValueBase = std::variant<
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

  class FoxValue : public FoxValueBase
  {
  public:
    using FoxValueBase::FoxValueBase;
    FoxValue(const std::string& s) : FoxValueBase(std::string_view(s)) {}
    FoxValue(const char* s) : FoxValueBase(std::string_view(s)) {}
  };

  FoxValue to_variant(const Value& v);
}