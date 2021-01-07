#ifndef FOXLOX_VALUE_H
#define FOXLOX_VALUE_H

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <compare>
#include <string_view>
#include <version>

namespace foxlox
{
  struct String
  {
    size_t length;
    char str[sizeof(length)];

    static String* alloc(size_t l)
    {
      char* data = new char[sizeof(String) + (l - sizeof(String::str))];
      assert(data != nullptr);
      String* str = reinterpret_cast<String*>(data);
      str->length = l;
      return str;
    }
    static void free(String* p)
    {
      delete[] reinterpret_cast<std::byte*>(p);
    }

    std::string_view get_view() const;

#if __cpp_lib_three_way_comparison >= 201907L
    using TStrViewComp = decltype(std::string_view{} <=> std::string_view{});
#else
    using TStrViewComp = std::weak_ordering;
#endif
    friend TStrViewComp operator<=>(const String& l, const String& r);
    friend bool operator==(const String& l, const String& r);
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

    double get_double() const;
    int64_t get_int64() const;

    friend double operator/(const Value& l, const Value& r);
    friend Value operator*(const Value& l, const Value& r);
    friend Value operator+(const Value& l, const Value& r);
    friend Value operator-(const Value& l, const Value& r);
    friend Value operator-(const Value& val);
    friend int64_t intdiv(const Value& l, const Value& r);
    friend std::partial_ordering operator<=>(const Value& l, const Value& r);
    friend bool operator==(const Value& l, const Value& r);

    std::string to_string() const;
  };
  using ValueArray = std::vector<Value>;
}

#endif // FOXLOX_VALUE_H