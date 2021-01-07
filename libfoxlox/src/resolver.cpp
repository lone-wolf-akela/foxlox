#include "resolver.h"

namespace foxlox
{
  Resolver::Resolver(AST&& a) : ast(std::move(a))
  {
    current_function = FunctionType::NONE;
    current_loop = LoopType::NONE;
    current_class = ClassType::NONE;
  }
  AST Resolver::resolve()
  {
    for (auto& stmt : ast)
    {
      resolve_stmt(stmt.get());
    }
    return std::move(ast);
  }
  void Resolver::resolve_expr(const expr::Expr* expr)
  {
    expr::IVisitor<void>::visit(expr);
  }
  void Resolver::resolve_stmt(const stmt::Stmt* stmt)
  {
    stmt::IVisitor<void>::visit(stmt);
  }
}