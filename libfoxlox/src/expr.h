#pragma once

#include <memory>
#include <vector>
#include <variant>
#include <compare>

#include <gsl/gsl>

#include <foxlox/foxexcept.h>
#include "token.h"
#include "compiletime_value.h"

namespace foxlox::stmt
{
  class Var;
  class Function;
  class Class;
}

namespace foxlox
{
  struct VarDeclareAtFunc
  {
    stmt::Function* func;
    int param_index;
    friend auto operator<=>(const VarDeclareAtFunc& l, const VarDeclareAtFunc& r) = default;
  };
  using VarDeclareAt = std::variant<stmt::Var*, stmt::Class*, stmt::Function*, VarDeclareAtFunc>;
}

namespace foxlox::expr
{
  class Expr
  {
  public:
    virtual std::unique_ptr<Expr> clone() = 0;
    virtual ~Expr() = default;
  };

  class Assign : public Expr
  {
  public:
    Assign(Token&& tk, std::unique_ptr<Expr>&& v);
    Token name;
    std::unique_ptr<Expr> value;

    // to be filled by resolver 
    // pointed to where the value is declared
    VarDeclareAt declare;

    virtual std::unique_ptr<Expr> clone() override;
  };

  class Binary : public Expr
  {
  public:
    Binary(std::unique_ptr<Expr>&& l, Token&& tk, std::unique_ptr<Expr>&& r);
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    virtual std::unique_ptr<Expr> clone() override;
  };

  class Logical : public Expr
  {
  public:
    Logical(std::unique_ptr<Expr>&& l, Token&& tk, std::unique_ptr<Expr>&& r);
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    virtual std::unique_ptr<Expr> clone() override;
  };
  class Tuple : public Expr
  {
  public:
    Tuple(std::vector<std::unique_ptr<Expr>>&& es);
    std::vector<std::unique_ptr<Expr>> exprs;

    virtual std::unique_ptr<Expr> clone() override;
  };
  class Grouping : public Expr
  {
  public:
    Grouping(std::unique_ptr<Expr>&& expr);
    std::unique_ptr<Expr> expression;

    virtual std::unique_ptr<Expr> clone() override;
  };
  class Literal : public Expr
  {
  public:
    Literal(CompiletimeValue&& v);
    Literal(const CompiletimeValue& v);
    CompiletimeValue value;

    virtual std::unique_ptr<Expr> clone() override;
  };
  class Unary : public Expr
  {
  public:
    Unary(Token&& tk, std::unique_ptr<Expr>&& r);
    Token op;
    std::unique_ptr<Expr> right;

    virtual std::unique_ptr<Expr> clone() override;
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

    virtual std::unique_ptr<Expr> clone() override;
  };
  class Variable : public Expr
  {
  public:
    Variable(Token&& tk);
    Token name;

    // to be filled by resolver 
    // pointed to where the value is declared
    VarDeclareAt declare;

    virtual std::unique_ptr<Expr> clone() override;
  };
  class Get : public Expr
  {
  public:
    Get(std::unique_ptr<Expr>&& o, Token&& tk);
    std::unique_ptr<Expr> obj;
    Token name;

    virtual std::unique_ptr<Expr> clone() override;
  };
  class Set : public Expr
  {
  public:
    Set(std::unique_ptr<Expr>&& o, Token&& tk, std::unique_ptr<Expr>&& v);
    std::unique_ptr<Expr> obj;
    Token name;
    std::unique_ptr<Expr> value;

    virtual std::unique_ptr<Expr> clone() override;
  };
  class Super : public Expr
  {
  public:
    Super(Token&& key, Token&& mthd);
    Token keyword;
    Token method;

    // to be filled by resolver 
    // pointed to the corresponding method 
    VarDeclareAt declare;

    virtual std::unique_ptr<Expr> clone() override;
  };
  class This : public Expr
  {
  public:
    This(Token&& tk);
    Token keyword;

    // to be filled by resolver 
    // pointed to the corresponding method 
    VarDeclareAt declare;

    virtual std::unique_ptr<Expr> clone() override;
  };

  template<typename R>
  class IVisitor
  {
  public:
    virtual R visit_binary_expr(gsl::not_null<Binary*> expr) = 0;
    virtual R visit_grouping_expr(gsl::not_null<Grouping*> expr) = 0;
    virtual R visit_tuple_expr(gsl::not_null<Tuple*> expr) = 0;
    virtual R visit_literal_expr(gsl::not_null<Literal*> expr) = 0;
    virtual R visit_unary_expr(gsl::not_null<Unary*> expr) = 0;
    virtual R visit_variable_expr(gsl::not_null<Variable*> expr) = 0;
    virtual R visit_assign_expr(gsl::not_null<Assign*> expr) = 0;
    virtual R visit_logical_expr(gsl::not_null<Logical*> expr) = 0;
    virtual R visit_call_expr(gsl::not_null<Call*> expr) = 0;
    virtual R visit_get_expr(gsl::not_null<Get*> expr) = 0;
    virtual R visit_set_expr(gsl::not_null<Set*> expr) = 0;
    virtual R visit_this_expr(gsl::not_null<This*> expr) = 0;
    virtual R visit_super_expr(gsl::not_null<Super*> expr) = 0;

    virtual ~IVisitor() = default;

    R visit(Expr* expr)
    {
      if (expr == nullptr)
      {
        return R();
      }
      if (auto p = dynamic_cast<Binary*>(expr); p != nullptr)
      {
        return visit_binary_expr(p);
      }
      if (auto p = dynamic_cast<Grouping*>(expr); p != nullptr)
      {
        return visit_grouping_expr(p);
      }
      if (auto p = dynamic_cast<Tuple*>(expr); p != nullptr)
      {
        return visit_tuple_expr(p);
      }
      if (auto p = dynamic_cast<Literal*>(expr); p != nullptr)
      {
        return visit_literal_expr(p);
      }
      if (auto p = dynamic_cast<Unary*>(expr); p != nullptr)
      {
        return visit_unary_expr(p);
      }
      if (auto p = dynamic_cast<Variable*>(expr); p != nullptr)
      {
        return visit_variable_expr(p);
      }
      if (auto p = dynamic_cast<Assign*>(expr); p != nullptr)
      {
        return visit_assign_expr(p);
      }
      if (auto p = dynamic_cast<Logical*>(expr); p != nullptr)
      {
        return visit_logical_expr(p);
      }
      if (auto p = dynamic_cast<Call*>(expr); p != nullptr)
      {
        return visit_call_expr(p);
      }
      if (auto p = dynamic_cast<Get*>(expr); p != nullptr)
      {
        return visit_get_expr(p);
      }
      if (auto p = dynamic_cast<Set*>(expr); p != nullptr)
      {
        return visit_set_expr(p);
      }
      if (auto p = dynamic_cast<This*>(expr); p != nullptr)
      {
        return visit_this_expr(p);
      }
      if (auto p = dynamic_cast<Super*>(expr); p != nullptr)
      {
        return visit_super_expr(p);
      }
      throw FatalError("Unknown expr type");
    }
  };
}
