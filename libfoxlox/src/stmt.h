#pragma once

#include <memory>
#include <vector>

#include <cassert>

#include <gsl/gsl>

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

    // to be filled by resolver, this is refer to the `this' variable
    VarStoreType store_type;
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
    virtual R visit_expression_stmt(Expression* stmt) = 0;
    virtual R visit_var_stmt(Var* stmt) = 0;
    virtual R visit_block_stmt(Block* stmt) = 0;
    virtual R visit_if_stmt(If* stmt) = 0;
    virtual R visit_while_stmt(While* stmt) = 0;
    virtual R visit_function_stmt(Function* stmt) = 0;
    virtual R visit_return_stmt(Return* stmt) = 0;
    virtual R visit_break_stmt(Break* stmt) = 0;
    virtual R visit_continue_stmt(Continue* stmt) = 0;
    virtual R visit_class_stmt(Class* stmt) = 0;
    virtual R visit_for_stmt(For* stmt) = 0;

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
      assert(false);
      return R();
    }
  };
}
