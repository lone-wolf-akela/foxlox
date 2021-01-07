#ifndef FOXLOX_VALUE_H
#define FOXLOX_VALUE_H

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <compare>

#include <fmt/format.h>

namespace foxlox
{
  struct String
  {
    size_t length;
    char str[sizeof(length)];

    static String* alloc(size_t l)
    {
      std::byte* data = new std::byte[sizeof(String) + (l - sizeof(String::str))];
      assert(data != nullptr);
      return reinterpret_cast<String*>(data);
    }
    static void free(String* p)
    {
      delete[] reinterpret_cast<std::byte*>(p);
    }
  };

  struct Value
  {
    enum : uint8_t
    {
      NIL, BOOL, F64, I64, STR,
    } type;
    union
    {
      bool b;
      double f64;
      int64_t i64;
      String* str;
    } v;

    Value();
    Value(String* str);
    Value(bool b);
    Value(double f64);
    Value(int64_t i64);

    void cast_double();
    void cast_int64();
    double get_double() const;
    int64_t get_int64() const;

    Value neg();
    Value add(Value r);
    Value sub(Value r);
    Value mul(Value r);
    Value div(Value r);

    friend int64_t intdiv(const Value& l, const Value& r);
    friend std::partial_ordering operator<=>(const Value& l, const Value& r);
    friend bool operator==(const Value& l, const Value& r);

    std::string to_string() const;
  };
  using ValueArray = std::vector<Value>;

  bool num_is_double(Value v);

  template<typename ... Args>
  bool num_have_double(Args ... args)
  {
    return (num_is_double(args) || ...);
  }
}

#endif // FOXLOX_VALUE_H