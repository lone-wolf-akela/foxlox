#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>

using namespace foxlox;

TEST(temp_test, temp)
{
  auto [res, chunk] = compile(R"(
var a = 123;
fun first(param) {
  var b = param;
  a = a + b;
}
first(234);
return a;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::I64);
  ASSERT_EQ(v.get_int64(), 123 + 234);
}