#pragma once

#include <memory>
#include <vector>
#include <variant>
#include <compare>

#include <gsl/gsl>

#include <foxlox/except.h>
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
  struct VarDeclareAtFunc // store function parameters ...
  {
    stmt::Function* func;
    int param_index;
    friend auto operator<=>(const VarDeclareAtFunc& l, const VarDeclareAtFunc& r) = default;
  };
  struct VarDeclareAtClass // store `this' ...
  {
    stmt::Class* klass;
    friend auto operator<=>(const VarDeclareAtClass& l, const VarDeclareAtClass& r) = default;
  };
  using VarDeclareAt = std::variant<stmt::Var*, stmt::Class*, stmt::Function*, VarDeclareAtFunc, VarDeclareAtClass>;
}

namespace foxlox::expr
{
  class Expr
  {
  public:
    virtual std::unique_ptr<Expr> clone() = 0;

    Expr() = default;
    Expr(const Expr&) = delete;
    Expr(Expr&&) = delete;
    Expr& operator=(const Expr&) = delete;
    Expr& operator=(Expr&&) = delete;
    virtual ~Expr() = default;
  };

  class Assign : public Expr
  {
  public:
    Assign(Token&& tk, std::unique_ptr<Expr>&& v) noexcept;
    Token name;
    std::unique_ptr<Expr> value;

    // to be filled by resolver 
    // pointed to where the value is declared
    VarDeclareAt declare;

    std::unique_ptr<Expr> clone() final;
  };

  class Binary : public Expr
  {
  public:
    Binary(std::unique_ptr<Expr>&& l, Token&& tk, std::unique_ptr<Expr>&& r) noexcept;
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    std::unique_ptr<Expr> clone() final;
  };

  class Logical : public Expr
  {
  public:
    Logical(std::unique_ptr<Expr>&& l, Token&& tk, std::unique_ptr<Expr>&& r) noexcept;
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    std::unique_ptr<Expr> clone() final;
  };
  class Tuple : public Expr
  {
  public:
    Tuple(std::vector<std::unique_ptr<Expr>>&& es) noexcept;
    std::vector<std::unique_ptr<Expr>> exprs;

    std::unique_ptr<Expr> clone() final;
  };
  class Grouping : public Expr
  {
  public:
    Grouping(std::unique_ptr<Expr>&& expr) noexcept;
    std::unique_ptr<Expr> expression;

    std::unique_ptr<Expr> clone() final;
  };
  class Literal : public Expr
  {
  public:
    Literal(CompiletimeValue&& v, Token&& tk) noexcept;
    Literal(const CompiletimeValue& v, Token tk) noexcept;
    CompiletimeValue value;

    // for error reporting
    Token token;

    std::unique_ptr<Expr> clone() final;
  };
  class Unary : public Expr
  {
  public:
    Unary(Token&& tk, std::unique_ptr<Expr>&& r) noexcept;
    Token op;
    std::unique_ptr<Expr> right;

    std::unique_ptr<Expr> clone() final;
  };
  class Call : public Expr
  {
  public:
    Call(std::unique_ptr<Expr>&& ce, Token&& tk, std::vector<std::unique_ptr<Expr>>&& augs) noexcept;
    std::unique_ptr<Expr> callee;
      // stores the token for the closing parenthesis.
      // its location is used when we report a runtime error caused by a function call.
    Token paren;
    std::vector<std::unique_ptr<Expr>> arguments;

    std::unique_ptr<Expr> clone() final;
  };
  class Variable : public Expr
  {
  public:
    Variable(Token&& tk) noexcept;
    Token name;

    // to be filled by resolver 
    // pointed to where the value is declared
    VarDeclareAt declare;

    std::unique_ptr<Expr> clone() final;
  };
  class Get : public Expr
  {
  public:
    Get(std::unique_ptr<Expr>&& o, Token&& tk) noexcept;
    std::unique_ptr<Expr> obj;
    Token name;

    std::unique_ptr<Expr> clone() final;
  };
  class Set : public Expr
  {
  public:
    Set(std::unique_ptr<Expr>&& o, Token&& tk, std::unique_ptr<Expr>&& v) noexcept;
    std::unique_ptr<Expr> obj;
    Token name;
    std::unique_ptr<Expr> value;

    std::unique_ptr<Expr> clone() final;
  };
  class Super : public Expr
  {
  public:
    Super(Token&& key, Token&& mthd) noexcept;
    Token keyword;
    Token method;

    // to be filled by resolver 
    // pointed to the corresponding method 
    VarDeclareAt declare;

    std::unique_ptr<Expr> clone() final;
  };
  class This : public Expr
  {
  public:
    This(Token&& tk) noexcept;
    Token keyword;

    // to be filled by resolver 
    // pointed to the corresponding class 
    VarDeclareAt declare;

    std::unique_ptr<Expr> clone() final;
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

    GSL_SUPPRESS(c.21)
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
