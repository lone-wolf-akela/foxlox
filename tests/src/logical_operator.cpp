#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>

using namespace foxlox;

TEST(logical_operator, and_)
{
  // Note: These tests implicitly depend on ints being truthy.
  VM vm;
  // Return the first non-true argument.
  {
    auto [res, chunk] = compile(R"(
return false and 1;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::BOOL);
    ASSERT_EQ(v.get_bool(), false);
  }
  {
    auto [res, chunk] = compile(R"(
return true and 1;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::I64);
    ASSERT_EQ(v.get_int64(), 1);
  }
  {
    auto [res, chunk] = compile(R"(
return 1 and 2 and false;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::BOOL);
    ASSERT_EQ(v.get_bool(), false);
  }
  // Return the last argument if all are true.
  {
    auto [res, chunk] = compile(R"(
return 1 and true;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::BOOL);
    ASSERT_EQ(v.get_bool(), true);
  }
  {
    auto [res, chunk] = compile(R"(
return 1 and 2 and 3;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::I64);
    ASSERT_EQ(v.get_int64(), 3);
  }
  // Short-circuit at the first false argument.
  {
    auto [res, chunk] = compile(R"(
var r = ();
var a = "before";
var b = "before";
(a = true) and (b = false) and (a = "bad");
r += a; # expect: true
r += b; # expect: false
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_tuple());
    auto s = v.get_tuplespan();
    ASSERT_EQ(ssize(s), 2);
    ASSERT_EQ(s[0].type, ValueType::BOOL);
    ASSERT_EQ(s[0].get_bool(), true);
    ASSERT_EQ(s[1].type, ValueType::BOOL);
    ASSERT_EQ(s[1].get_bool(), false);
  }
}

TEST(logical_operator, and_truth)
{
  VM vm;
  // False and nil are false.
  {
    auto [res, chunk] = compile(R"(
return false and "bad";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::BOOL);
    ASSERT_EQ(v.get_bool(), false);
  }
  {
    auto [res, chunk] = compile(R"(
return nil and "bad";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_nil());
  }
  // Everything else is true.
  {
    auto [res, chunk] = compile(R"(
return true and "ok";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "ok");
  }
  {
    auto [res, chunk] = compile(R"(
return 0 and "ok";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "ok");
  }
  {
    auto [res, chunk] = compile(R"(
return "" and "ok";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "ok");
  }
}

TEST(logical_operator, or_)
{
  // Note: These tests implicitly depend on ints being truthy.
  VM vm;
  // Return the first true argument.
  {
    auto [res, chunk] = compile(R"(
return 1 or true;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::I64);
    ASSERT_EQ(v.get_int64(), 1);
  }
  {
    auto [res, chunk] = compile(R"(
return false or 1;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::I64);
    ASSERT_EQ(v.get_int64(), 1);
  }
  {
    auto [res, chunk] = compile(R"(
return false or false or true;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::BOOL);
    ASSERT_EQ(v.get_bool(), true);
  }
  // Return the last argument if all are false.
  {
    auto [res, chunk] = compile(R"(
return false or false;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::BOOL);
    ASSERT_EQ(v.get_bool(), false);
  }
  {
    auto [res, chunk] = compile(R"(
return  false or false or false;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::BOOL);
    ASSERT_EQ(v.get_bool(), false);
  }
  // Short-circuit at the first true argument.
  {
    auto [res, chunk] = compile(R"(
var r = ();
var a = "before";
var b = "before";
(a = false) or (b = true) or (a = "bad");
r += a; # expect: false
r += b; # expect: true
return r;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_tuple());
    auto s = v.get_tuplespan();
    ASSERT_EQ(ssize(s), 2);
    ASSERT_EQ(s[0].type, ValueType::BOOL);
    ASSERT_EQ(s[0].get_bool(), false);
    ASSERT_EQ(s[1].type, ValueType::BOOL);
    ASSERT_EQ(s[1].get_bool(), true);
  }
}

TEST(logical_operator, or_truth)
{
  VM vm;
  // False and nil are false.
  {
    auto [res, chunk] = compile(R"(
return false or "ok";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "ok");
  }
  {
    auto [res, chunk] = compile(R"(
return nil or "ok";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "ok");
  }
  // Everything else is true.
  {
    auto [res, chunk] = compile(R"(
return true or "ok";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::BOOL);
    ASSERT_EQ(v.get_bool(), true);
  }
  {
    auto [res, chunk] = compile(R"(
return 0 or "ok";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, ValueType::I64);
    ASSERT_EQ(v.get_int64(), 0);
  }
  {
    auto [res, chunk] = compile(R"(
return "s" or "ok";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "s");
  }
}