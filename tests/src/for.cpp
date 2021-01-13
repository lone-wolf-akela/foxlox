#include <gtest/gtest.h>

#include <vm.h>
#include <compiler.h>

using namespace foxlox;

TEST(for_, scope)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
var r = ();
{
  var i = "before";

  # New variable is in inner scope.
  for (var i = 0; i < 1; i = i + 1) {
    r = r + i; # expect: 0

    # Loop body is in second inner scope.
    var i = -1;
    r = r + i; # expect: -1
  }
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 2);
    ASSERT_EQ(s[0].type, Value::I64);
    ASSERT_EQ(s[0].get_int64(), 0);
    ASSERT_EQ(s[1].type, Value::I64);
    ASSERT_EQ(s[1].get_int64(), -1);
  }
  {
    auto [res, chunk] = compile(R"(
var r = ();
{
  # New variable shadows outer variable.
  for (var i = 0; i > 0; i = i + 1) {}

  # Goes out of scope after loop.
  var i = "after";
  r = r + i; # expect: after

  # Can reuse an existing variable.
  for (i = 0; i < 1; i = i + 1) {
    r = r + i; # expect: 0
  }
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 2);
    ASSERT_EQ(s[0].type, Value::STR);
    ASSERT_EQ(s[0].get_strview(), "after");
    ASSERT_EQ(s[1].type, Value::I64);
    ASSERT_EQ(s[1].get_int64(), 0);
  }
}