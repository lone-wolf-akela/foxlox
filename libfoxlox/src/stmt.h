#pragma once

#include <memory>
#include <vector>

#include <cassert>

#include <gsl/gsl>

#include <foxlox/except.h>
#include "expr.h"
#include "token.h"

namespace foxlox::stmt
{
  enum class VarStoreType
  {
    Stack, // to be stored on stack, and destroyed when function returns 
    Static, // to be stored in the value pool of the closure, and still exists after function exits
  };

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

    // to be filled by resolver
    VarStoreType store_type;
  };

  class While : public Stmt
  {
  public:
    While(std::unique_ptr<expr::Expr>&& cond, std::unique_ptr<Stmt>&& bd, Token&& r_paren);
    std::unique_ptr<expr::Expr> condition;
    std::unique_ptr<Stmt> body;

    // for error reporting
    Token right_paren;
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
      std::unique_ptr<Stmt>&& elseb,
      Token&& r_paren
    );
    std::unique_ptr<expr::Expr> condition;
    std::unique_ptr<Stmt> then_branch;
    std::unique_ptr<Stmt> else_branch;

    // for error reporting
    Token right_paren;
  };

  class Function : public Stmt
  {
  public:
    Function(Token&& tk, std::vector<Token>&& par, std::vector<std::unique_ptr<Stmt>>&& bd);
    Token name;
    std::vector<Token> param;
    std::vector<std::unique_ptr<Stmt>> body;

    // to be filled by resolver
    VarStoreType name_store_type;
    std::vector<VarStoreType> param_store_types;
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
    Class(Token&& tk, std::unique_ptr<expr::Variable>&& super, std::vector<std::unique_ptr<Function>>&& ms);
    // for error reporting
    Token name;
    std::unique_ptr<expr::Variable> superclass;
    std::vector<std::unique_ptr<Function>> methods;

    // to be filled by resolver, this is refer to the class name as a variable
    VarStoreType name_store_type;
    // to be filled by resolver, this is refer to the `this' variable
    VarStoreType this_store_type;
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
      std::unique_ptr<Stmt>&& bd,
      Token&& r_paren
    );
    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<expr::Expr> condition;
    std::unique_ptr<expr::Expr> increment;
    std::unique_ptr<Stmt> body;

    // for error reporting
    Token right_paren;
  };

  template<typename R>
  class IVisitor
  {
  public:
    virtual R visit_expression_stmt(gsl::not_null<Expression*> stmt) = 0;
    virtual R visit_var_stmt(gsl::not_null<Var*> stmt) = 0;
    virtual R visit_block_stmt(gsl::not_null<Block*> stmt) = 0;
    virtual R visit_if_stmt(gsl::not_null<If*> stmt) = 0;
    virtual R visit_while_stmt(gsl::not_null<While*> stmt) = 0;
    virtual R visit_function_stmt(gsl::not_null<Function*> stmt) = 0;
    virtual R visit_return_stmt(gsl::not_null<Return*> stmt) = 0;
    virtual R visit_break_stmt(gsl::not_null<Break*> stmt) = 0;
    virtual R visit_continue_stmt(gsl::not_null<Continue*> stmt) = 0;
    virtual R visit_class_stmt(gsl::not_null<Class*> stmt) = 0;
    virtual R visit_for_stmt(gsl::not_null<For*> stmt) = 0;

    virtual ~IVisitor() = default;

    R visit(Stmt* stmt)
    {
      if (stmt == nullptr) 
      {
        return R(); 
      }
      if (auto p = dynamic_cast<Expression*>(stmt); p != nullptr)
      {
        return visit_expression_stmt(p);
      }
      if (auto p = dynamic_cast<Var*>(stmt); p != nullptr)
      {
        return visit_var_stmt(p);
      }
      if (auto p = dynamic_cast<Block*>(stmt); p != nullptr)
      {
        return visit_block_stmt(p);
      }
      if (auto p = dynamic_cast<If*>(stmt); p != nullptr)
      {
        return visit_if_stmt(p);
      }
      if (auto p = dynamic_cast<While*>(stmt); p != nullptr)
      {
        return visit_while_stmt(p);
      }
      if (auto p = dynamic_cast<Function*>(stmt); p != nullptr)
      {
        return visit_function_stmt(p);
      }
      if (auto p = dynamic_cast<Return*>(stmt); p != nullptr)
      {
        return visit_return_stmt(p);
      }
      if (auto p = dynamic_cast<Break*>(stmt); p != nullptr)
      {
        return visit_break_stmt(p);
      }
      if (auto p = dynamic_cast<Continue*>(stmt); p != nullptr)
      {
        return visit_continue_stmt(p);
      }
      if (auto p = dynamic_cast<Class*>(stmt); p != nullptr)
      {
        return visit_class_stmt(p);
      }
      if (auto p = dynamic_cast<For*>(stmt); p != nullptr)
      {
        return visit_for_stmt(p);
      }
      throw FatalError("Unknown stmt type");
    }
  };
}
