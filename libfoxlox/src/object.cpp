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
}