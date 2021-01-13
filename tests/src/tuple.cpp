#include <gtest/gtest.h>

#include <vm.h>
#include <compiler.h>

using namespace foxlox;

TEST(tuple, creation)
{
  VM vm;
  // empty tuple
  {
    auto [res, chunk] = compile(R"(
return ();
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 0);
  }
  // this is not a tuple
  {
    auto [res, chunk] = compile(R"(
return ("a");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::STR);
    ASSERT_EQ(v.get_strview(), "a");
  }
  // this is a tuple with 1 element
  {
    auto [res, chunk] = compile(R"(
return ("a",);
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 1);
    ASSERT_EQ(s[0].type, Value::STR);
    ASSERT_EQ(s[0].get_strview(), "a");
  }
  {
    auto [res, chunk] = compile(R"(
return ("a", "b", "c");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 3);
    ASSERT_EQ(s[0].type, Value::STR);
    ASSERT_EQ(s[0].get_strview(), "a");
    ASSERT_EQ(s[1].type, Value::STR);
    ASSERT_EQ(s[1].get_strview(), "b");
    ASSERT_EQ(s[2].type, Value::STR);
    ASSERT_EQ(s[2].get_strview(), "c");
  }
  {
    auto [res, chunk] = compile(R"(
return ("a", "b", "c",);
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 3);
    ASSERT_EQ(s[0].type, Value::STR);
    ASSERT_EQ(s[0].get_strview(), "a");
    ASSERT_EQ(s[1].type, Value::STR);
    ASSERT_EQ(s[1].get_strview(), "b");
    ASSERT_EQ(s[2].type, Value::STR);
    ASSERT_EQ(s[2].get_strview(), "c");
  }
}

TEST(tuple, add)
{
  VM vm;
  {
    auto [res, chunk] = compile(R"(
return ("a", "b") + "c";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 3);
    ASSERT_EQ(s[0].type, Value::STR);
    ASSERT_EQ(s[0].get_strview(), "a");
    ASSERT_EQ(s[1].type, Value::STR);
    ASSERT_EQ(s[1].get_strview(), "b");
    ASSERT_EQ(s[2].type, Value::STR);
    ASSERT_EQ(s[2].get_strview(), "c");
  }
  {
    auto [res, chunk] = compile(R"(
return "a" + ("b", "c");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 3);
    ASSERT_EQ(s[0].type, Value::STR);
    ASSERT_EQ(s[0].get_strview(), "a");
    ASSERT_EQ(s[1].type, Value::STR);
    ASSERT_EQ(s[1].get_strview(), "b");
    ASSERT_EQ(s[2].type, Value::STR);
    ASSERT_EQ(s[2].get_strview(), "c");
  }
  {
    auto [res, chunk] = compile(R"(
return ("a",) + ("b", "c");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.interpret(chunk);
    ASSERT_EQ(v.type, Value::TUPLE);
    auto s = v.get_tuplespan();
    ASSERT_EQ(s.size(), 3);
    ASSERT_EQ(s[0].type, Value::STR);
    ASSERT_EQ(s[0].get_strview(), "a");
    ASSERT_EQ(s[1].type, Value::STR);
    ASSERT_EQ(s[1].get_strview(), "b");
    ASSERT_EQ(s[2].type, Value::STR);
    ASSERT_EQ(s[2].get_strview(), "c");
  }
}