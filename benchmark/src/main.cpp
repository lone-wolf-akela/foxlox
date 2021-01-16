#include <cassert>

#include <foxlox/vm.h>
#include <foxlox/compiler.h>

constexpr auto fib = R"(
fun fib(n) {
  if (n < 2) return n;
  return fib(n - 2) + fib(n - 1);
}

var start = clock();
println(fib(35) == 9227465);
println(clock() - start);
)";
constexpr auto equality = R"(
var i = 0;

var loopStart = clock();

while (i < 10000000) {
  i = i + 1;

  1; 1; 1; 2; 1; nil; 1; "str"; 1; true;
  nil; nil; nil; 1; nil; "str"; nil; true;
  true; true; true; 1; true; false; true; "str"; true; nil;
  "str"; "str"; "str"; "stru"; "str"; 1; "str"; nil; "str"; true;
}

var loopTime = clock() - loopStart;

var start = clock();

i = 0;
while (i < 10000000) {
  i = i + 1;

  1 == 1; 1 == 2; 1 == nil; 1 == "str"; 1 == true;
  nil == nil; nil == 1; nil == "str"; nil == true;
  true == true; true == 1; true == false; true == "str"; true == nil;
  "str" == "str"; "str" == "stru"; "str" == 1; "str" == nil; "str" == true;
}

var elapsed = clock() - start;
println("loop: {}", loopTime);
println("elapsed: {}", elapsed);
println("equals: {}", elapsed - loopTime);
)";

constexpr auto bench_to_run = fib;
//constexpr auto bench_to_run = equality;

int main()
{
  auto [res, chunk] = foxlox::compile(bench_to_run);
  assert(res == foxlox::CompilerResult::OK);
  foxlox::VM vm;
  vm.interpret(chunk);
  return 0;
}