#include <gtest/gtest.h>
import foxlox;

//TODO

using namespace foxlox;

TEST(tuple, creation)
{
  // empty tuple
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return ();
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_tuple());
    auto s = v.get_tuplespan();
    ASSERT_EQ(ssize(s), 0);
  }
  // this is not a tuple
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return ("a");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_str());
    ASSERT_EQ(v.get_strview(), "a");
  }
  // this is a tuple with 1 element
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return ("a",);
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_tuple());
    auto s = v.get_tuplespan();
    ASSERT_EQ(ssize(s), 1);
    ASSERT_TRUE(s[0].is_str());
    ASSERT_EQ(s[0].get_strview(), "a");
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return ("a", "b", "c");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_tuple());
    auto s = v.get_tuplespan();
    ASSERT_EQ(ssize(s), 3);
    ASSERT_TRUE(s[0].is_str());
    ASSERT_EQ(s[0].get_strview(), "a");
    ASSERT_TRUE(s[1].is_str());
    ASSERT_EQ(s[1].get_strview(), "b");
    ASSERT_TRUE(s[2].is_str());
    ASSERT_EQ(s[2].get_strview(), "c");
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return ("a", "b", "c",);
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_tuple());
    auto s = v.get_tuplespan();
    ASSERT_EQ(ssize(s), 3);
    ASSERT_TRUE(s[0].is_str());
    ASSERT_EQ(s[0].get_strview(), "a");
    ASSERT_TRUE(s[1].is_str());
    ASSERT_EQ(s[1].get_strview(), "b");
    ASSERT_TRUE(s[2].is_str());
    ASSERT_EQ(s[2].get_strview(), "c");
  }
}

TEST(tuple, add)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return ("a", "b") + "c";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_tuple());
    auto s = v.get_tuplespan();
    ASSERT_EQ(ssize(s), 3);
    ASSERT_TRUE(s[0].is_str());
    ASSERT_EQ(s[0].get_strview(), "a");
    ASSERT_TRUE(s[1].is_str());
    ASSERT_EQ(s[1].get_strview(), "b");
    ASSERT_TRUE(s[2].is_str());
    ASSERT_EQ(s[2].get_strview(), "c");
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return "a" + ("b", "c");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_tuple());
    auto s = v.get_tuplespan();
    ASSERT_EQ(ssize(s), 3);
    ASSERT_TRUE(s[0].is_str());
    ASSERT_EQ(s[0].get_strview(), "a");
    ASSERT_TRUE(s[1].is_str());
    ASSERT_EQ(s[1].get_strview(), "b");
    ASSERT_TRUE(s[2].is_str());
    ASSERT_EQ(s[2].get_strview(), "c");
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return ("a",) + ("b", "c");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = vm.run(chunk);
    ASSERT_TRUE(v.is_tuple());
    auto s = v.get_tuplespan();
    ASSERT_EQ(ssize(s), 3);
    ASSERT_TRUE(s[0].is_str());
    ASSERT_EQ(s[0].get_strview(), "a");
    ASSERT_TRUE(s[1].is_str());
    ASSERT_EQ(s[1].get_strview(), "b");
    ASSERT_TRUE(s[2].is_str());
    ASSERT_EQ(s[2].get_strview(), "c");
  }
}