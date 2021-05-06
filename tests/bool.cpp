#include <foxlox/vm.h>
#include <foxlox/compiler.h>
#include <foxlox/cppinterop.h>

#include <gtest/gtest.h>

using namespace foxlox;

TEST(bool_, equality)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true == true;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(true));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true == false;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(false));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false == true; 
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(false));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false == false;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(true));
  }
  // Not equal to other types.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true == 1;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(false));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false == 0;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(false));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true == 1.0;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(false));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false == 0.0;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(false));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true == "true";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(false));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false == "false";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(false));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false == "";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(false));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false == nil;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(false));
  }

  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true != true;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(false));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true != false;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(true));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false != true; 
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(true));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false != false;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(false));
  }
  // Not equal to other types.
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true != 1;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(true));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false != 0;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(true));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true != 1.0;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(true));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false != 0.0;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(true));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return true != "true";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(true));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false != "false";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(true));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false != "";
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(true));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return false != nil;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(true));
  }
}

TEST(bool_, not_)
{
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return !true;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(false));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return !false;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(true));
  }
  {
    VM vm;
    auto [res, chunk] = compile(R"(
return !!true;
)");
    ASSERT_EQ(res, CompilerResult::OK);
    auto v = to_variant(vm.run(chunk));
    ASSERT_EQ(v, FoxValue(true));
  }
}