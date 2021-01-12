#include <gtest/gtest.h>

#include <vm.h>
#include <compiler.h>

using namespace foxlox;

TEST(single_expr, int_add)
{
  auto [res, chunk] = compile(R"(
return 123 + 234;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::I64);
  ASSERT_EQ(v.get_int64(), 123 + 234);
}

TEST(single_expr, int_double_add)
{
  auto [res, chunk] = compile(R"(
return 123 + 234.5;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::F64);
  ASSERT_EQ(v.get_double(), 123 + 234.5);
}

TEST(single_expr, double_int_add)
{
  auto [res, chunk] = compile(R"(
return 234.5 + 123;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::F64);
  ASSERT_EQ(v.get_double(), 234.5 + 123);
}

TEST(single_expr, double_double_add)
{
  auto [res, chunk] = compile(R"(
return 123.3 + 234.5;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::F64);
  ASSERT_EQ(v.get_double(), 123.3 + 234.5);
}

TEST(single_expr, str_str_add)
{
  auto [res, chunk] = compile(R"(
return "Hello, " + "World!";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::STR);
  ASSERT_EQ(v.get_strview(), "Hello, World!");
}

TEST(single_expr, double_double_intdiv)
{
  auto [res, chunk] = compile(R"(
return 200.5 // 100.3;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::I64);
  ASSERT_EQ(v.get_int64(), int64_t(200.5 / 100.3));
}

TEST(single_expr, double_double_div)
{
  auto [res, chunk] = compile(R"(
return 200.5 / 100.3;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_EQ(v.type, Value::F64);
  ASSERT_EQ(v.get_double(), 200.5 / 100.3);
}