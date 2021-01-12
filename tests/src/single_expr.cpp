#include <gtest/gtest.h>

#include <vm.h>
#include <compiler.h>

using namespace foxlox;

TEST(Single_Expr, IntAdd)
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

TEST(Single_Expr, Int_Double_Add)
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

TEST(Single_Expr, Double_Int_Add)
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

TEST(Single_Expr, Double_Double_Add)
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

TEST(Single_Expr, Str_Str_Add)
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

TEST(Single_Expr, Double_Double_IntDiv)
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

TEST(Single_Expr, Double_Double_Div)
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