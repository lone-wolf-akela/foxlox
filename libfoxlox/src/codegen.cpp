#include <cassert>

#include "codegen.h"

namespace foxlox
{
  CodeGen::CodeGen(AST&& a):
    ast(std::move(a))
  {
    current_line = 0;
    closure_stack.push_back(chunk.add_closure());
  }
  Chunk CodeGen::gen()
  {
    for (auto& stmt : ast)
    {
      compile_stmt(stmt.get());
    }
    return std::move(chunk);
  }
  Closure& CodeGen::current_closure()
  {
    return chunk.get_closures()[closure_stack.back()];
  }
  void CodeGen::compile_expr(const expr::Expr* expr)
  {
    if (expr == nullptr) { return; }
    expr::IVisitor<void>::visit(expr);
  }
  void CodeGen::compile_stmt(const stmt::Stmt* stmt)
  {
    if (stmt == nullptr) { return; }
    stmt::IVisitor<void>::visit(stmt);
  }
  void CodeGen::visit_binary_expr(const expr::Binary* expr)
  {
    compile_expr(expr->left.get());
    compile_expr(expr->right.get());
    switch (expr->op.type)
    {
    case TokenType::MINUS:
      emit(OpCode::OP_SUBTRACT);
      break;
    case TokenType::SLASH:
      emit(OpCode::OP_DIVIDE);
      break;
    case TokenType::STAR:
      emit(OpCode::OP_MULTIPLY);
      break;
    case TokenType::PLUS:
      emit(OpCode::OP_ADD);
      break;
    case TokenType::SLASH_SLASH:
      emit(OpCode::OP_INTDIV);
      break;
    case TokenType::GREATER:
      emit(OpCode::OP_GT);
      break;
    case TokenType::GREATER_EQUAL:
      emit(OpCode::OP_GE);
      break;
    case TokenType::LESS:
      emit(OpCode::OP_LT);
      break;
    case TokenType::LESS_EQUAL:
      emit(OpCode::OP_LE);
      break;
    case TokenType::BANG_EQUAL:
      emit(OpCode::OP_NE);
      break;
    case TokenType::EQUAL_EQUAL: 
      emit(OpCode::OP_EQ);
      break;
    default:
      assert(false);
    }
  }
  void CodeGen::visit_literal_expr(const expr::Literal* expr)
  {
    auto& v = expr->value.v;
    if (std::holds_alternative<std::monostate>(v))
    {
      emit(OpCode::OP_NIL);
      return;
    }
    if (std::holds_alternative<double>(v))
    {
      const auto constant = chunk.add_constant(Value(std::get<double>(v)));
      emit(OpCode::OP_CONSTANT, constant);
      return;
    }
    if (std::holds_alternative<int64_t>(v))
    {
      const auto i64 = std::get<int64_t>(v);
      if (INST_IA_MIN <= i64 && i64 <= INST_IA_MAX)
      {
        emit(OpCode::OP_INT, gsl::narrow_cast<int32_t>(i64));
      }
      else
      {
        const auto constant = chunk.add_constant(Value(i64));
        emit(OpCode::OP_CONSTANT, constant);
      }
      return;
    }
    if (std::holds_alternative<std::string>(v))
    {
      const auto str_index = chunk.add_string(std::get<std::string>(v));
      emit(OpCode::OP_STRING, str_index);
      return;
    }
    if (std::holds_alternative<bool>(v))
    {
      emit(OpCode::OP_BOOL, static_cast<int32_t>(std::get<bool>(v)));
      return;
    }
    assert(false);
  }
}