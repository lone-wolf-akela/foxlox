#ifndef FOXLOX_STMT_H
#define FOXLOX_STMT_H

#include <memory>
#include <vector>

#include <cassert>

#include "expr.h"
#include "token.h"

namespace foxlox::stmt
{
  class Stmt
  {
  public:
    virtual ~Stmt() = default;
  };

  class Expression : public Stmt
  {
  public:
    Expression(std::unique_ptr<expr::Expr>&& expr);
    std::unique_ptr<expr::Expr> expression;
  };
  
  class Var : public Stmt
  {
  public:
    Var(Token&& tk, std::unique_ptr<expr::Expr>&& init);
    Token name;
    std::unique_ptr<expr::Expr> initializer;
  };

  class While : public Stmt
  {
  public:
    While(std::unique_ptr<expr::Expr>&& cond, std::unique_ptr<Stmt>&& bd);
    std::unique_ptr<expr::Expr> condition;
    std::unique_ptr<Stmt> body;
  };

  class Block : public Stmt
  {
  public:
    Block(std::vector<std::unique_ptr<Stmt>>&& stmts);
    std::vector<std::unique_ptr<Stmt>> statements;
  };

  class If : public Stmt
  {
  public:
    If(
      std::unique_ptr<expr::Expr>&& cond, 
      std::unique_ptr<Stmt>&& thenb, 
      std::unique_ptr<Stmt>&& elseb
    );
    std::unique_ptr<expr::Expr> condition;
    std::unique_ptr<Stmt> then_branch;
    std::unique_ptr<Stmt> else_branch;
  };

  class Function : public Stmt
  {
  public:
    Function(Token&& tk, std::vector<Token>&& par, std::vector<std::unique_ptr<Stmt>>&& bd);
    Token name;
    std::vector<Token> param;
    std::vector<std::unique_ptr<Stmt>> body;
  };

  class Return : public Stmt
  {
  public:
    Return(Token&& tk, std::unique_ptr<expr::Expr>&& v);
    // for error reporting
    Token keyword;
    std::unique_ptr<expr::Expr> value;
  };

  class Class : public Stmt
  {
  public:
    Class(Token&& tk, expr::Variable&& super, std::vector<std::unique_ptr<Function>>&& ms);
    // for error reporting
    Token name;
    expr::Variable superclass;
    std::vector<std::unique_ptr<Function>> methods;
  };

  class Break : public Stmt
  {
  public:
    Break(Token&& tk);
    // for error reporting
    Token keyword;
  };

  class Continue : public Stmt
  {
  public:
    Continue(Token&& tk);
    // for error reporting
    Token keyword;
  };

  class For : public Stmt
  {
  public:
    For(
      std::unique_ptr<Stmt>&& init, 
      std::unique_ptr<expr::Expr>&& cond, 
      std::unique_ptr<expr::Expr>&& incre, 
      std::unique_ptr<Stmt>&& bd
    );
    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<expr::Expr> condition;
    std::unique_ptr<expr::Expr> increment;
    std::unique_ptr<Stmt> body;
  };

  template<typename R>
  class IVisitor
  {
  public:
    virtual R visit_expression_stmt(const Expression* stmt) = 0;
    virtual R visit_var_stmt(const Var* stmt) = 0;
    virtual R visit_block_stmt(const Block* stmt) = 0;
    virtual R visit_if_stmt(const If* stmt) = 0;
    virtual R visit_while_stmt(const While* stmt) = 0;
    virtual R visit_function_stmt(const Function* stmt) = 0;
    virtual R visit_return_stmt(const Return* stmt) = 0;
    virtual R visit_break_stmt(const Break* stmt) = 0;
    virtual R visit_continue_stmt(const Continue* stmt) = 0;
    virtual R visit_class_stmt(const Class* stmt) = 0;
    virtual R visit_for_stmt(const For* stmt) = 0;

    virtual ~IVisitor() = default;

    R visit(const Stmt* stmt)
    {
      if (auto p = dynamic_cast<const Expression*>(stmt); p != nullptr)
      {
        return visit_expression_stmt(p);
      }
      if (auto p = dynamic_cast<const Var*>(stmt); p != nullptr)
      {
        return visit_var_stmt(p);
      }
      if (auto p = dynamic_cast<const Block*>(stmt); p != nullptr)
      {
        return visit_block_stmt(p);
      }
      if (auto p = dynamic_cast<const If*>(stmt); p != nullptr)
      {
        return visit_if_stmt(p);
      }
      if (auto p = dynamic_cast<const While*>(stmt); p != nullptr)
      {
        return visit_while_stmt(p);
      }
      if (auto p = dynamic_cast<const Function*>(stmt); p != nullptr)
      {
        return visit_function_stmt(p);
      }
      if (auto p = dynamic_cast<const Return*>(stmt); p != nullptr)
      {
        return visit_return_stmt(p);
      }
      if (auto p = dynamic_cast<const Break*>(stmt); p != nullptr)
      {
        return visit_break_stmt(p);
      }
      if (auto p = dynamic_cast<const Continue*>(stmt); p != nullptr)
      {
        return visit_continue_stmt(p);
      }
      if (auto p = dynamic_cast<const Class*>(stmt); p != nullptr)
      {
        return visit_class_stmt(p);
      }
      if (auto p = dynamic_cast<const For*>(stmt); p != nullptr)
      {
        return visit_for_stmt(p);
      }
      assert(false);
      return {};
    }
  };
}

#endif