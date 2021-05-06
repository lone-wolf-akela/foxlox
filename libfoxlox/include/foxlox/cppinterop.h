#pragma once

import <variant>;
import <string>;
import <string_view>;
import <utility>;

#include "../../src/value.h"
#include "../../src/object.h"

namespace foxlox
{
  struct nil_t 
  {
    friend bool operator==(const nil_t&, const nil_t&) noexcept;
  };
  inline nil_t nil;

  struct TupleSpan : public std::span<Value>
  {
    TupleSpan(const std::span<Value>& r) noexcept;
    TupleSpan(std::span<Value>&& r) noexcept;
    friend bool operator==(const TupleSpan& l, const TupleSpan& r) noexcept;
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
    FoxValue(const std::string& s) noexcept;
    FoxValue(const char* s) noexcept;
  };

  FoxValue to_variant(const Value& v);
}