#include <gtest/gtest.h>
import foxlox;

using namespace foxlox;

TEST(logical_operator, and_)
{
  // Note: These tests implicitly depend on ints being truthy.
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
# Return the first non-true argument.
r += false and 1; 
r += true and 1; 
r += 1 and 2 and false; 

# Return the last argument if all are true.
r += 1 and true; 
r += 1 and 2 and 3; 

# Short-circuit at the first false argument.
var a = "before", b = "before";
(a = true) and
    (b = false) and
    (a = "bad");
r += a; 
r += b; 
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 7);
  ASSERT_EQ(v[0], false);
  ASSERT_EQ(v[1], 1);
  ASSERT_EQ(v[2], false);
  ASSERT_EQ(v[3], true);
  ASSERT_EQ(v[4], 3);
  ASSERT_EQ(v[5], true);
  ASSERT_EQ(v[6], false);
}

TEST(logical_operator, and_truth)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
# False and nil are false.
r += false and "bad";
r += nil and "bad"; 
# Everything else is true.
r += true and "ok"; 
r += 0 and "ok"; 
r += "" and "ok"; 
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 5);
  ASSERT_EQ(v[0], false);
  ASSERT_EQ(v[1], nil);
  ASSERT_EQ(v[2], "ok");
  ASSERT_EQ(v[3], "ok");
  ASSERT_EQ(v[4], "ok");
}

TEST(logical_operator, or_)
{
  // Note: These tests implicitly depend on ints being truthy.
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
# Return the first true argument.
r += 1 or true; 
r += false or 1; 
r += false or false or true;

# Return the last argument if all are false.
r += false or false; 
r += false or false or false;

# Short-circuit at the first true argument.
var a = "before", b = "before";
(a = false) or
    (b = true) or
    (a = "bad");
r += a; 
r += b; 
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 7);
  ASSERT_EQ(v[0], 1);
  ASSERT_EQ(v[1], 1);
  ASSERT_EQ(v[2], true);
  ASSERT_EQ(v[3], false);
  ASSERT_EQ(v[4], false);
  ASSERT_EQ(v[5], false);
  ASSERT_EQ(v[6], true);
}

TEST(logical_operator, or_truth)
{
  VM vm;
  auto [res, chunk] = compile(R"(
var r = ();
# False and nil are false.
r += false or "ok"; 
r += nil or "ok";

# Everything else is true.
r += true or "ok";
r += 0 or "ok";
r += "s" or "ok"; 
return r;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_TRUE(v.is<TupleSpan>());
  ASSERT_EQ(v.ssize(), 5);
  ASSERT_EQ(v[0], "ok");
  ASSERT_EQ(v[1], "ok");
  ASSERT_EQ(v[2], true);
  ASSERT_EQ(v[3], 0);
  ASSERT_EQ(v[4], "s");
}