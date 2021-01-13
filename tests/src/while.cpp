#include <gtest/gtest.h>

#include <vm.h>
#include <compiler.h>

using namespace foxlox;

TEST(while_, syntax)
{
  VM vm;
  {
    // Single-expression body.
    auto [res, chunk] = compile(R"(
var r = ();
var c = 0;
while (c < 3) r = r + (c = c + 1);
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 3);
    for (int i = 0; i < 3; i++)
    {
      ASSERT_EQ(s[i].type, Value::I64);
      ASSERT_EQ(s[i].get_int64(), i + 1);
    }
  }
  {
    // Block body.
    auto [res, chunk] = compile(R"(
var r = ();
var a = 0;
while (a < 3) {
  r = r + a;
  a = a + 1;
}
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 3);
    for (int i = 0; i < 3; i++)
    {
      ASSERT_EQ(s[i].type, Value::I64);
      ASSERT_EQ(s[i].get_int64(), i);
    }
  }
  {
    // Statement bodies.
    auto [res, chunk] = compile(R"(
while (false) if (true) 1; else 2;
while (false) while (true) 1;
while (false) for (;;) 1;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::NIL);
  }
}