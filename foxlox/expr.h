#ifndef FOXLOX_EXPR_H
#define FOXLOX_EXPR_H

#include <memory>
#include <vector>

#include "token.h"
#include "compiletime_value.h"

namespace foxlox::expr
{
  class Expr
  {
  public:
    virtual ~Expr() = default;
  };

  class Assign : public Expr
  {
  public:
    Assign(Token&& tk, std::unique_ptr<Expr>&& v);
    Token name;
    std::unique_ptr<Expr> value;
  };

  class Binary : public Expr
  {
  public:
    Binary(std::unique_ptr<Expr>&& l, Token&& tk, std::unique_ptr<Expr>&& r);
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
  };

  class Logical : public Expr
  {
  public:
    Logical(std::unique_ptr<Expr>&& l, Token&& tk, std::unique_ptr<Expr>&& r);
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
  };

  class Grouping : public Expr
  {
  public:
    Grouping(std::unique_ptr<Expr>&& expr);
    std::unique_ptr<Expr> expression;
  };
  class Literal : public Expr
  {
  public:
    Literal(CompiletimeValue&& v);
    Literal(const CompiletimeValue& v);
    CompiletimeValue value;
  };
  class Unary : public Expr
  {
  public:
    Unary(Token&& tk, std::unique_ptr<Expr>&& r) : op(std::move(tk)), right(std::move(r)) {}
    Token op;
    std::unique_ptr<Expr> right;
  };
  class Call : public Expr
  {
  public:
    Call(std::unique_ptr<Expr>&& ce, Token&& tk, std::vector<std::unique_ptr<Expr>>&& augs);
    std::unique_ptr<Expr> callee;
      // stores the token for the closing parenthesis.
      // its location is used when we report a runtime error caused by a function call.
    Token paren;
    std::vector<std::unique_ptr<Expr>> arguments;
  };
  class Variable : public Expr
  {
  public:
    Variable(Token&& tk);
    Token name;
  };
  class Get : public Expr
  {
  public:
    Get(std::unique_ptr<Expr>&& o, Token&& tk);
    std::unique_ptr<Expr> obj;
    Token name;
  };
  class Set : public Expr
  {
  public:
    Set(std::unique_ptr<Expr>&& o, Token&& tk, std::unique_ptr<Expr>&& v);
    std::unique_ptr<Expr> obj;
    Token name;
    std::unique_ptr<Expr> value;
  };
  class Super : public Expr
  {
  public:
    Super(Token&& key, Token&& mthd);
    Token keyword;
    Token method;
  };
  class This : public Expr
  {
  public:
    This(Token&& tk);
    Token keyword;
  };

  template<typename R>
  class IVisitor
  {
  public:
    virtual R visit_binary_expr(const Binary* expr) = 0;
    virtual R visit_grouping_expr(const Grouping* expr) = 0;
    virtual R visit_literal_expr(const Literal* expr) = 0;
    virtual R visit_unary_expr(const Unary* expr) = 0;
    virtual R visit_variable_expr(const Variable* expr) = 0;
    virtual R visit_assign_expr(const Assign* expr) = 0;
    virtual R visit_logical_expr(const Logical* expr) = 0;
    virtual R visit_call_expr(const Call* expr) = 0;
    virtual R visit_get_expr(const Get* expr) = 0;
    virtual R visit_set_expr(const Set* expr) = 0;
    virtual R visit_this_expr(const This* expr) = 0;
    virtual R visit_super_expr(const Super* expr) = 0;

    virtual ~IVisitor() = default;

    R visit(const Expr* stmt)
    {
      if (auto p = dynamic_cast<const Binary*>(stmt); p != nullptr)
      {
        return visit_binary_expr(p);
      }
      if (auto p = dynamic_cast<const Grouping*>(stmt); p != nullptr)
      {
        return visit_grouping_expr(p);
      }
      if (auto p = dynamic_cast<const Literal*>(stmt); p != nullptr)
      {
        return visit_literal_expr(p);
      }
      if (auto p = dynamic_cast<const Unary*>(stmt); p != nullptr)
      {
        return visit_unary_expr(p);
      }
      if (auto p = dynamic_cast<const Variable*>(stmt); p != nullptr)
      {
        return visit_variable_expr(p);
      }
      if (auto p = dynamic_cast<const Assign*>(stmt); p != nullptr)
      {
        return visit_assign_expr(p);
      }
      if (auto p = dynamic_cast<const Logical*>(stmt); p != nullptr)
      {
        return visit_logical_expr(p);
      }
      if (auto p = dynamic_cast<const Call*>(stmt); p != nullptr)
      {
        return visit_call_expr(p);
      }
      if (auto p = dynamic_cast<const Get*>(stmt); p != nullptr)
      {
        return visit_get_expr(p);
      }
      if (auto p = dynamic_cast<const Set*>(stmt); p != nullptr)
      {
        return visit_set_expr(p);
      }
      if (auto p = dynamic_cast<const This*>(stmt); p != nullptr)
      {
        return visit_this_expr(p);
      }
      if (auto p = dynamic_cast<const Super*>(stmt); p != nullptr)
      {
        return visit_super_expr(p);
      }
    }
  };
}

#endif // FOXLOX_EXPR_H