#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>
#include <foxlox/cppinterop.h>

using namespace foxlox;

TEST(assignment, associativity)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
var a = "a";
var b = "b";
var c = "c";
# Assignment is right-associative.
a = b = c;
r += a; # expect: c
r += b; # expect: c
r += c; # expect: c
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = to_variant(vm.interpret(chunk));
  ASSERT_TRUE(std::holds_alternative<TupleSpan>(v));
  auto s = std::get<TupleSpan>(v);
  int i = 0;
  ASSERT_EQ(to_variant(s[i++]), FoxValue("c"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("c"));
  ASSERT_EQ(to_variant(s[i++]), FoxValue("c"));
}