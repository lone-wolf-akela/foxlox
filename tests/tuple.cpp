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

TEST(tuple, unpack)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var a, b, c;
(a, b, c) = ("a", "b", "c");
return (a, b, c);
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

TEST(tuple, chain_unpack)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var a1, b1, c1, a2, b2, c2, r = (a1, b1, c1) = (a2, b2, c2) = ("a", "b", "c");
return r + (a1, b1, c1) + (a2, b2, c2);
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 9);
    ASSERT_EQ(v[0], "a");
    ASSERT_EQ(v[1], "b");
    ASSERT_EQ(v[2], "c");
    ASSERT_EQ(v[3], "a");
    ASSERT_EQ(v[4], "b");
    ASSERT_EQ(v[5], "c");
    ASSERT_EQ(v[6], "a");
    ASSERT_EQ(v[7], "b");
    ASSERT_EQ(v[8], "c");
  }
}

TEST(tuple, unpack_to_classmember)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
class A 
{
  __init__() {
    this.a = this.b = "b";
  }
}
var a = A(), c;
(a.a, c) = ("a", "c");
return (a.a, a.b, c);
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

TEST(tuple, unpack_wrong_size)
{
  {  
    VM vm;
    auto [res, chunk] = compile(R"(
var a, b;
(a, b) = ("a", "b", "c");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    ASSERT_THROW(vm.run(chunk), RuntimeError);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var a, b, c;
(a, b, c) = ("a", "b");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    ASSERT_THROW(vm.run(chunk), RuntimeError);
  }
}

TEST(tuple, unpack_nontuple)
{

  VM vm;
  auto [res, chunk] = compile(R"(
var a, b;
(a, b) = "str";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(tuple, order)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
# Unpack is from right to left
var a;
(a, a, a) = ("a", "b", "c");
return a;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v, "a");
  }
}

TEST(tuple, unpack_to_literal)
{
  {
    auto [res, chunk] = compile(R"CODE(
(1, 2) = (1, 2);
)CODE");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
  {
    auto [res, chunk] = compile(R"CODE(
("1", "2") = ("1", "2");
)CODE");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
}

TEST(tuple, recursion_unpack)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var a, b, c;
(a, (b, c)) = ("a", ("b", "c"));
return (a, b, c);
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
var a, b, c;
((a, b), c) = (("a", "b"), "c");
return (a, b, c);
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

TEST(tuple, deep_recursion_unpack)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var v1, v2, v3, v4, v5;
(((((v1,), v2), v3), v4), v5) = (((((1,), 2), 3), 4), 5);
return (v1, v2, v3, v4, v5);
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 5);
    ASSERT_EQ(v[0], 1);
    ASSERT_EQ(v[1], 2);
    ASSERT_EQ(v[2], 3);
    ASSERT_EQ(v[3], 4);
    ASSERT_EQ(v[4], 5);
  }
}

TEST(tuple, recursion_unpack_wrong_size)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var a, b, c;
((a, b), c) = (("a", "b", "?"), "c");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    ASSERT_THROW(vm.run(chunk), RuntimeError);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var a, b, c;
(a, (b, c)) = ("a", ("b",));
)");
    ASSERT_EQ(res, CompilerResult::OK);
    ASSERT_THROW(vm.run(chunk), RuntimeError);
  }
}

TEST(tuple, recursion_unpack_nontuple)
{

  VM vm;
  auto [res, chunk] = compile(R"(
var a, b;
(a, (b,)) = ("a", "b");
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(tuple, declare_unpack)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var (a, b, c) = ("a", "b", "c");
return (a, b, c);
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

TEST(tuple, declare_chain_unpack)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var (a2, b2, c2) = ("a", "b", "c"), (a1, b1, c1) = (a2, b2, c2), r = (a1, b1, c1);
return r + (a1, b1, c1) + (a2, b2, c2);
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 9);
    ASSERT_EQ(v[0], "a");
    ASSERT_EQ(v[1], "b");
    ASSERT_EQ(v[2], "c");
    ASSERT_EQ(v[3], "a");
    ASSERT_EQ(v[4], "b");
    ASSERT_EQ(v[5], "c");
    ASSERT_EQ(v[6], "a");
    ASSERT_EQ(v[7], "b");
    ASSERT_EQ(v[8], "c");
  }
}

TEST(tuple, declare_unpack_to_classmember)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
class A 
{
  __init__() {
    this.a = "b";
  }
}
var a = A();
var (a.a, c) = ("a", "c");
)");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
}

TEST(tuple, declare_unpack_wrong_size)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var (a, b) = ("a", "b", "c");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    ASSERT_THROW(vm.run(chunk), RuntimeError);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var (a, b, c) = ("a", "b");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    ASSERT_THROW(vm.run(chunk), RuntimeError);
  }
}

TEST(tuple, declare_unpack_nontuple)
{

  VM vm;
  auto [res, chunk] = compile(R"(
var (a, b) = "str";
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}

TEST(tuple, declare_order)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
# Unpack is from right to left
var a = 1, (b, c, d) = (++a, ++a, ++a);
return (a, b, c, d);
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_EQ(v.ssize(), 4);
    ASSERT_EQ(v[0], 4);
    ASSERT_EQ(v[1], 2);
    ASSERT_EQ(v[2], 3);
    ASSERT_EQ(v[3], 4);
  }
}

TEST(tuple, declare_unpack_to_literal)
{
  {
    auto [res, chunk] = compile(R"CODE(
var (1, 2) = (1, 2);
)CODE");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
  {
    auto [res, chunk] = compile(R"CODE(
var ("1", "2") = ("1", "2");
)CODE");
    ASSERT_EQ(res, CompilerResult::COMPILE_ERROR);
  }
}

TEST(tuple, declare_recursion_unpack)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var (a, (b, c)) = ("a", ("b", "c"));
return (a, b, c);
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
var ((a, b), c) = (("a", "b"), "c");
return (a, b, c);
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

TEST(tuple, declare_deep_recursion_unpack)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var (((((v1,), v2), v3), v4), v5) = (((((1,), 2), 3), 4), 5);
return (v1, v2, v3, v4, v5);
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = FoxValue(vm.run(chunk));
    ASSERT_TRUE(v.is<TupleSpan>());
    ASSERT_EQ(v.ssize(), 5);
    ASSERT_EQ(v[0], 1);
    ASSERT_EQ(v[1], 2);
    ASSERT_EQ(v[2], 3);
    ASSERT_EQ(v[3], 4);
    ASSERT_EQ(v[4], 5);
  }
}

TEST(tuple, declare_recursion_unpack_wrong_size)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var ((a, b), c) = (("a", "b", "?"), "c");
)");
    ASSERT_EQ(res, CompilerResult::OK);
    ASSERT_THROW(vm.run(chunk), RuntimeError);
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
var (a, (b, c)) = ("a", ("b",));
)");
    ASSERT_EQ(res, CompilerResult::OK);
    ASSERT_THROW(vm.run(chunk), RuntimeError);
  }
}

TEST(tuple, declare_recursion_unpack_nontuple)
{

  VM vm;
  auto [res, chunk] = compile(R"(
var (a, (b,)) = ("a", "b");
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_THROW(vm.run(chunk), RuntimeError);
}