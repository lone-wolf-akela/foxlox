#include <gtest/gtest.h>
import <fstream>;

import foxlox;
using namespace foxlox;

// SWITCH ON DEBUG_STRESS_GC TO TEST THIS
TEST(regression, gc_declared_but_not_used)
{
  VM vm;
  auto [res, chunk] = compile(R"(
# r is only declared but not used in global subroutine
# but it should still be in global subroutine's reference list
# otherwise, r will be recycled before it is accessed in inFoo()
var r = ();
class Foo {
  inFoo() {
    r += "in foo";
  }
}
var foo = Foo();
foo.inFoo();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_NO_THROW(vm.run(chunk));
}

TEST(regression, wrong_jump_length)
{
  VM vm;
  auto [res, chunk] = compile(R"(
# I was making some stupid bug...
if(true)
{
  1 + 1;
  2 + 2;
}
else
{
  2 + 2;
  1 + 1;
}
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_NO_THROW(vm.run(chunk));
}

TEST(regression, init_not_found_after_rehash)
{
  VM vm;
  auto [res, chunk] = compile(R"(
# there was a bug
# it hapens when there are 7 entries in string pool
# which trigers the rehash of the pool
# and the pool wrongly use the old capacity mask instead of the new one
# during rehashing
class C {
  __init__(a) {
    this.one = 1;
    this.two = 2;
    this.three = 3;
    this.four = 4;
    this.five = 5;
    # __init__ is the 6th
    # and the vm will try add another `__init__' (the 7th entry) into the pool
    # which triger the rehash
  }
}
C(0);
)");
  ASSERT_EQ(res, CompilerResult::OK);
  ASSERT_NO_THROW(vm.run(chunk));
}

TEST(regression, class_pool_vector_pusback_invalid)
{
  // the class pool in vm was once a vector
  // pointer to class would bu invalid
  // if we call push_back on that vector
  {
    std::ofstream ofs("lib.fox");
    ASSERT_TRUE(!!ofs);
    ofs << R"(
export import fox.io;
class World {
  __init__(id) {
    this.id = id;
  }
  print()	{}
}
export class Program : World {
  __init__(name) {
    super.__init__(42);
    this.name = name;
  }
  print() {
    super.print();
  }
}
export fun fib(n) {
  var first = 1;
  var second = 0;
  for(var i = 0; i < n; ++i) {
    var t = second;
    second = first + second;
  first = t;
  }
  return second;
})";
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
import lib;
from lib import fib;
fun main() {
  var prog = lib.Program("A_Name");
  prog.print();
}
main();
)");
    ASSERT_EQ(res, CompilerResult::OK);
    ASSERT_NO_THROW(vm.run(chunk));
  }
  std::filesystem::remove("lib.fox");
}


TEST(regression, false_not_equal_to_false)
{
  // this only happens in clang
  // as `2 < 1' will set the bool bit in Value to 0
  // but left the full value to 1024
  // which makes it not equal to a plain false (0)
  VM vm;
  auto [res, chunk] = compile(R"(
return false == 2 < 1;
)");
  ASSERT_EQ(res, CompilerResult::OK);
  auto v = FoxValue(vm.run(chunk));
  ASSERT_EQ(v, true);
}