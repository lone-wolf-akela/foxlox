#include <gtest/gtest.h>

#include <vm.h>
#include <compiler.h>

using namespace foxlox;

TEST(temp_test, temp)
{
  auto [res, chunk] = compile(R"(
var a = 123;
fun first() {
  a = a + 234;
}
first();
return a;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::I64);
  ASSERT_EQ(v.get_int64(), 123 + 234);
}