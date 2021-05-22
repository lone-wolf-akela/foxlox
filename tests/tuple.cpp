#include <gtest/gtest.h>
import foxlox;

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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 0);
  }
  // this is not a tuple
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return ("a");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, "a");
  }
  // this is a tuple with 1 element
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return ("a",);
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 1);
    ASSERT_EQ(v[0], "a");
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return ("a", "b", "c");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 3);
    ASSERT_EQ(v[0], "a");
    ASSERT_EQ(v[1], "b");
    ASSERT_EQ(v[2], "c");
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return ("a", "b", "c",);
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 3);
    ASSERT_EQ(v[0], "a");
    ASSERT_EQ(v[1], "b");
    ASSERT_EQ(v[2], "c");
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
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 3);
    ASSERT_EQ(v[0], "a");
    ASSERT_EQ(v[1], "b");
    ASSERT_EQ(v[2], "c");
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return "a" + ("b", "c");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 3);
    ASSERT_EQ(v[0], "a");
    ASSERT_EQ(v[1], "b");
    ASSERT_EQ(v[2], "c");
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return ("a",) + ("b", "c");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 3);
    ASSERT_EQ(v[0], "a");
    ASSERT_EQ(v[1], "b");
    ASSERT_EQ(v[2], "c");
  }
}