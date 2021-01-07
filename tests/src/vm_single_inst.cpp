#include <gtest/gtest.h>

#include <vm.h>
#include <chunk.h>
#include "../../libfoxlox/src/opcode.h"

using namespace foxlox;

TEST(VM_SingleInst, NOP)
{
  Chunk chunk;
  chunk.add_closure();
  VM vm;
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_NOP), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_RETURN), 0);
  ASSERT_EQ(vm.interpret(chunk), InterpretResult::OK);
  ASSERT_EQ(vm.get_stack_size(), 0);
}

TEST(VM_SingleInst, Int)
{
  Chunk chunk;
  chunk.add_closure();
  VM vm;
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_INT, 123), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_RETURN), 0);
  ASSERT_EQ(vm.interpret(chunk), InterpretResult::OK);
  ASSERT_EQ(vm.get_stack_size(), 1);
  ASSERT_EQ(vm.top()->type, Value::I64);
  ASSERT_EQ(vm.top()->v.i64, 123);
}

TEST(VM_SingleInst, IntAdd)
{
  Chunk chunk;
  chunk.add_closure();
  VM vm;
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_INT, 123), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_INT, 234), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_ADD), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_RETURN), 0);
  ASSERT_EQ(vm.interpret(chunk), InterpretResult::OK);
  ASSERT_EQ(vm.get_stack_size(), 1);
  ASSERT_EQ(vm.top()->type, Value::I64);
  ASSERT_EQ(vm.top()->v.i64, 123 + 234);
}

TEST(VM_SingleInst, Int_Double_Add)
{
  Chunk chunk;
  chunk.add_closure();
  chunk.add_constant(Value(234.5));
  VM vm;
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_INT, 123), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_CONSTANT, 0u), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_ADD), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_RETURN), 0);
  ASSERT_EQ(vm.interpret(chunk), InterpretResult::OK);
  ASSERT_EQ(vm.get_stack_size(), 1);
  ASSERT_EQ(vm.top()->type, Value::F64);
  ASSERT_EQ(vm.top()->v.f64, 123 + 234.5);
}

TEST(VM_SingleInst, Double_Int_Add)
{
  Chunk chunk;
  chunk.add_closure();
  chunk.add_constant(Value(234.5));
  VM vm;
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_CONSTANT, 0u), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_INT, 123), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_ADD), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_RETURN), 0);
  ASSERT_EQ(vm.interpret(chunk), InterpretResult::OK);
  ASSERT_EQ(vm.get_stack_size(), 1);
  ASSERT_EQ(vm.top()->type, Value::F64);
  ASSERT_EQ(vm.top()->v.f64, 123 + 234.5);
}

TEST(VM_SingleInst, Double_Double_Add)
{
  Chunk chunk;
  chunk.add_closure();
  chunk.add_constant(Value(123.3));
  chunk.add_constant(Value(234.5));
  VM vm;
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_CONSTANT, 0u), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_CONSTANT, 1u), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_ADD), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_RETURN), 0);
  ASSERT_EQ(vm.interpret(chunk), InterpretResult::OK);
  ASSERT_EQ(vm.get_stack_size(), 1);
  ASSERT_EQ(vm.top()->type, Value::F64);
  ASSERT_EQ(vm.top()->v.f64, 123.3 + 234.5);
}

TEST(VM_SingleInst, Str_Str_Add)
{
  Chunk chunk;
  chunk.add_closure();
  chunk.add_string("Hello, ");
  chunk.add_string("World!");
  VM vm;
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_STRING, 0u), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_STRING, 1u), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_ADD), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_RETURN), 0);
  ASSERT_EQ(vm.interpret(chunk), InterpretResult::OK);
  ASSERT_EQ(vm.get_stack_size(), 1);
  ASSERT_EQ(vm.top()->type, Value::STR);
  ASSERT_EQ(vm.top()->v.str->get_view(), "Hello, World!");
}

TEST(VM_SingleInst, Double_Double_IntDiv)
{
  Chunk chunk;
  chunk.add_closure();
  chunk.add_constant(Value(200.5));
  chunk.add_constant(Value(100.3));
  VM vm;
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_CONSTANT, 0u), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_CONSTANT, 1u), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_INTDIV), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_RETURN), 0);
  ASSERT_EQ(vm.interpret(chunk), InterpretResult::OK);
  ASSERT_EQ(vm.get_stack_size(), 1);
  ASSERT_EQ(vm.top()->type, Value::I64);
  ASSERT_EQ(vm.top()->v.i64, int64_t(200.5 / 100.3));
}

TEST(VM_SingleInst, Double_Double_Div)
{
  Chunk chunk;
  chunk.add_closure();
  chunk.add_constant(Value(200.5));
  chunk.add_constant(Value(100.3));
  VM vm;
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_CONSTANT, 0u), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_CONSTANT, 1u), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_DIVIDE), 0);
  chunk.get_closures()[0].add_code(Inst(OpCode::OP_RETURN), 0);
  ASSERT_EQ(vm.interpret(chunk), InterpretResult::OK);
  ASSERT_EQ(vm.get_stack_size(), 1);
  ASSERT_EQ(vm.top()->type, Value::F64);
  ASSERT_EQ(vm.top()->v.f64, 200.5 / 100.3);
}