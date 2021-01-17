#include <gtest/gtest.h>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>

using namespace foxlox;

TEST(temp_test, temp)
{
  auto [res, chunk] = compile(R"(
class CoffeeMaker {
  __init__(coffee) {
    this.coffee = coffee;
  }

  brew() {
    println("Enjoy your cup of {}", this.coffee);

    # No reusing the grounds!
    this.coffee = nil;
  }
}
class Child : CoffeeMaker
{
  __init__() {
    super.__init__("coffee and chicory");
  }
}
var maker = Child();
maker.brew();
maker.brew();
)");
  ASSERT_EQ(res, CompilerResult::OK);
  VM vm;
  auto v = vm.interpret(chunk);
  ASSERT_TRUE(v.is_nil());
}