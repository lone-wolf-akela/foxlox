#include <cassert>

#include "codegen.h"

namespace foxlox
{
  CodeGen::CodeGen(AST&& a):
    ast(std::move(a))
  {
    current_line = 0;
    closure_stack.push_back(chunk.add_closure());
    current_stack_size = 0;
  }
  Chunk CodeGen::gen()
  {
    for (auto& stmt : ast)
    {
      compile(stmt.get());
    }
    return std::move(chunk);
  }
  Closure& CodeGen::current_closure()
  {
    return chunk.get_closures()[closure_stack.back()];
  }
  void CodeGen::push_stack()
  {
    current_stack_size++;
  }
  void CodeGen::pop_stack()
  {
    current_stack_size--;
  }
  int16_t CodeGen::idx_cast(int16_t idx)
  {
    return idx < 0 ? idx : current_stack_size - idx - 1;
  }
  void CodeGen::compile(expr::Expr* expr)
  {
    if (expr == nullptr) { return; }
    expr::IVisitor<void>::visit(expr);
  }
  void CodeGen::compile(stmt::Stmt* stmt)
  {
    if (stmt == nullptr) { return; }
    stmt::IVisitor<void>::visit(stmt);
  }
  void CodeGen::visit_binary_expr(expr::Binary* expr)
  {
    current_line = expr->op.line;

    compile(expr->left.get());
    compile(expr->right.get());
    switch (expr->op.type)
    {
    case TokenType::MINUS:
      emit(OP_SUBTRACT);
      break;
    case TokenType::SLASH:
      emit(OP_DIVIDE);
      break;
    case TokenType::STAR:
      emit(OP_MULTIPLY);
      break;
    case TokenType::PLUS:
      emit(OP_ADD);
      break;
    case TokenType::SLASH_SLASH:
      emit(OP_INTDIV);
      break;
    case TokenType::GREATER:
      emit(OP_GT);
      break;
    case TokenType::GREATER_EQUAL:
      emit(OP_GE);
      break;
    case TokenType::LESS:
      emit(OP_LT);
      break;
    case TokenType::LESS_EQUAL:
      emit(OP_LE);
      break;
    case TokenType::BANG_EQUAL:
      emit(OP_NE);
      break;
    case TokenType::EQUAL_EQUAL: 
      emit(OP_EQ);
      break;
    default:
      assert(false);
    }
    pop_stack();
  }
  void CodeGen::visit_grouping_expr(expr::Grouping* expr)
  {
    compile(expr->expression.get());
  }
  void CodeGen::visit_literal_expr(expr::Literal* expr)
  {
    auto& v = expr->value.v;
    if (std::holds_alternative<std::monostate>(v))
    {
      emit(OP_NIL);
    }
    else if (std::holds_alternative<double>(v))
    {
      const uint16_t constant = chunk.add_constant(Value(std::get<double>(v)));
      emit(OP_CONSTANT, constant);
    }
    else if (std::holds_alternative<int64_t>(v))
    {
      const int64_t i64 = std::get<int64_t>(v);
      const uint16_t constant = chunk.add_constant(Value(i64));
      emit(OP_CONSTANT, constant);
    }
    else if (std::holds_alternative<std::string>(v))
    {
      const uint16_t str_index = chunk.add_string(std::get<std::string>(v));
      emit(OP_STRING, str_index);
    }
    else if (std::holds_alternative<bool>(v))
    {
      emit(OP_BOOL, std::get<bool>(v));
    }
    else
    {
      assert(false);
    }
    push_stack();
  }
  void CodeGen::visit_unary_expr(expr::Unary* expr)
  {
    current_line = expr->op.line;

    compile(expr->right.get());
    switch (expr->op.type)
    {
    case TokenType::MINUS:
      emit(OP_NEGATE);
    case TokenType::BANG:
      emit(OP_NOT);
    default:
      assert(false);
    }
  }
  void CodeGen::visit_variable_expr(expr::Variable* expr)
  {
    current_line = expr->name.line;

    const int16_t idx = idx_cast(value_idxs.at(expr->declare));
    emit(OP_LOAD, idx);
    push_stack();
  }
  void CodeGen::visit_assign_expr(expr::Assign* expr)
  {
    current_line = expr->name.line;

    compile(expr->value.get());
    const int16_t idx = idx_cast(value_idxs.at(expr->declare));
    emit(OP_STORE, idx);
  }
}