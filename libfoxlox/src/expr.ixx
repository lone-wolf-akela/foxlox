module;
#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>
export module foxlox:expr;

import <memory>;
import <vector>;
import <variant>;
import <compare>;

import <gsl/gsl>;

import :except;
import :token;
import :compiletime_value;

namespace foxlox::stmt
{
  export class Function;
  export class Class;
  export class VarDeclareBase;
  export class VarDeclareListBase;
}

namespace foxlox
{
  export struct VarDeclareFromList // store function parameters, etc
  {
    stmt::VarDeclareListBase* list;
    gsl::index index;
    friend auto operator<=>(const VarDeclareFromList& l, const VarDeclareFromList& r) = default;
  };
  export struct ClassThisDeclare // store `this' ...
  {
    stmt::Class* klass;
    friend auto operator<=>(const ClassThisDeclare& l, const ClassThisDeclare& r) = default;
  };
  export using VarDeclareAt = std::variant<VarDeclareFromList, ClassThisDeclare>;
}

namespace foxlox::expr
{
  export class Expr
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

  export class Assign : public Expr
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

  export class Binary : public Expr
  {
  public:
    Binary(std::unique_ptr<Expr>&& l, Token&& tk, std::unique_ptr<Expr>&& r) noexcept;
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    std::unique_ptr<Expr> clone() final;
  };

  export class Logical : public Expr
  {
  public:
    Logical(std::unique_ptr<Expr>&& l, Token&& tk, std::unique_ptr<Expr>&& r) noexcept;
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    std::unique_ptr<Expr> clone() final;
  };
  export class Tuple : public Expr
  {
  public:
    Tuple(std::vector<std::unique_ptr<Expr>>&& es) noexcept;
    std::vector<std::unique_ptr<Expr>> exprs;

    std::unique_ptr<Expr> clone() final;
  };
  export class TupleUnpack : public Expr
  {
  public:
    TupleUnpack(std::unique_ptr<Expr>&& tpl, std::vector<std::unique_ptr<Expr>>&& list) noexcept;
    std::unique_ptr<Expr> tuple;
    std::vector<std::unique_ptr<Expr>> assignlist;

    std::unique_ptr<Expr> clone() final;
  };
  export class NoOP : public Expr
  {
  public:
    NoOP() = default;
    std::unique_ptr<Expr> clone() final;
  };
  export class Grouping : public Expr
  {
  public:
    Grouping(std::unique_ptr<Expr>&& expr) noexcept;
    std::unique_ptr<Expr> expression;

    std::unique_ptr<Expr> clone() final;
  };
  export class Literal : public Expr
  {
  public:
    Literal(CompiletimeValue&& v, Token&& tk) noexcept;
    Literal(const CompiletimeValue& v, Token tk) noexcept;
    CompiletimeValue value;

    // for error reporting
    Token token;

    std::unique_ptr<Expr> clone() final;
  };
  export class Unary : public Expr
  {
  public:
    Unary(Token&& tk, std::unique_ptr<Expr>&& r) noexcept;
    Token op;
    std::unique_ptr<Expr> right;

    std::unique_ptr<Expr> clone() final;
  };
  export class Call : public Expr
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
  export class Variable : public Expr
  {
  public:
    Variable(Token&& tk) noexcept;
    Token name;

    // to be filled by resolver 
    // pointed to where the value is declared
    VarDeclareAt declare;

    std::unique_ptr<Expr> clone() final;
  };
  export class Get : public Expr
  {
  public:
    Get(std::unique_ptr<Expr>&& o, Token&& tk) noexcept;
    std::unique_ptr<Expr> obj;
    Token name;

    std::unique_ptr<Expr> clone() final;
  };
  export class Set : public Expr
  {
  public:
    Set(std::unique_ptr<Expr>&& o, Token&& tk, std::unique_ptr<Expr>&& v) noexcept;
    std::unique_ptr<Expr> obj;
    Token name;
    std::unique_ptr<Expr> value;

    std::unique_ptr<Expr> clone() final;
  };
  export class Super : public Expr
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
  export class This : public Expr
  {
  public:
    This(Token&& tk) noexcept;
    Token keyword;

    // to be filled by resolver 
    // pointed to the corresponding class 
    VarDeclareAt declare;

    std::unique_ptr<Expr> clone() final;
  };

  export template<typename R>
    class IVisitor
  {
  public:
    virtual R visit_binary_expr(gsl::not_null<Binary*> expr) = 0;
    virtual R visit_tupleunpack_expr(gsl::not_null<TupleUnpack*> expr) = 0;
    virtual R visit_noop_expr(gsl::not_null<NoOP*> expr) = 0;
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
      if (auto p = dynamic_cast<TupleUnpack*>(expr); p != nullptr)
      {
        return visit_tupleunpack_expr(p);
      }
      if (auto p = dynamic_cast<NoOP*>(expr); p != nullptr)
      {
        return visit_noop_expr(p);
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

namespace foxlox::expr
{
  Assign::Assign(Token&& tk, std::unique_ptr<Expr>&& v) noexcept :
    name(std::move(tk)),
    value(std::move(v))
  {
  }
  std::unique_ptr<Expr> Assign::clone()
  {
    return std::make_unique<Assign>(Token(name), value->clone());
  }
  Binary::Binary(std::unique_ptr<Expr>&& l, Token&& tk, std::unique_ptr<Expr>&& r) noexcept :
    left(std::move(l)),
    op(std::move(tk)),
    right(std::move(r))
  {
  }
  std::unique_ptr<Expr> Binary::clone()
  {
    return std::make_unique<Binary>(left->clone(), Token(op), right->clone());
  }
  Logical::Logical(std::unique_ptr<Expr>&& l, Token&& tk, std::unique_ptr<Expr>&& r) noexcept :
    left(std::move(l)),
    op(std::move(tk)),
    right(std::move(r))
  {
  }
  std::unique_ptr<Expr> Logical::clone()
  {
    return std::make_unique<Logical>(left->clone(), Token(op), right->clone());
  }
  TupleUnpack::TupleUnpack(std::unique_ptr<Expr>&& tpl, std::vector<std::unique_ptr<Expr>>&& list) noexcept :
    tuple(std::move(tpl)),
    assignlist(std::move(list))
  {
  }
  std::unique_ptr<Expr> TupleUnpack::clone()
  {
    auto cloned_assignlist = assignlist
      | ranges::views::transform([](auto& e) {return e->clone(); })
      | ranges::to<std::vector<std::unique_ptr<Expr>>>;
    return std::make_unique<TupleUnpack>(tuple->clone(), std::move(cloned_assignlist));
  }
  std::unique_ptr<Expr> NoOP::clone()
  {
    return std::make_unique<NoOP>();
  }
  Grouping::Grouping(std::unique_ptr<Expr>&& expr) noexcept :
    expression(std::move(expr))
  {
  }
  std::unique_ptr<Expr> Grouping::clone()
  {
    return std::make_unique<Grouping>(expression->clone());
  }
  Literal::Literal(CompiletimeValue&& v, Token&& tk) noexcept :
    value(std::move(v)),
    token(std::move(tk))
  {
  }
  Literal::Literal(const CompiletimeValue& v, Token tk) noexcept :
    value(v),
    token(tk)
  {
  }
  std::unique_ptr<Expr> Literal::clone()
  {
    return std::make_unique<Literal>(value, token);
  }
  Call::Call(std::unique_ptr<Expr>&& ce, Token&& tk, std::vector<std::unique_ptr<Expr>>&& augs) noexcept :
    callee(std::move(ce)),
    paren(std::move(tk)),
    arguments(std::move(augs))
  {
  }
  std::unique_ptr<Expr> Call::clone()
  {
    auto augs = arguments
      | ranges::views::transform([](auto& arg) {return arg->clone(); })
      | ranges::to<std::vector<std::unique_ptr<Expr>>>;
    return std::make_unique<Call>(callee->clone(), Token(paren), std::move(augs));
  }
  Variable::Variable(Token&& tk) noexcept :
    name(std::move(tk))
  {
  }
  std::unique_ptr<Expr> Variable::clone()
  {
    return std::make_unique<Variable>(Token(name));
  }
  Get::Get(std::unique_ptr<Expr>&& o, Token&& tk) noexcept :
    obj(std::move(o)),
    name(std::move(tk))
  {
  }
  std::unique_ptr<Expr> Get::clone()
  {
    return std::make_unique<Get>(obj->clone(), Token(name));
  }
  Set::Set(std::unique_ptr<Expr>&& o, Token&& tk, std::unique_ptr<Expr>&& v) noexcept :
    obj(std::move(o)),
    name(std::move(tk)),
    value(std::move(v))
  {
  }
  std::unique_ptr<Expr> Set::clone()
  {
    return std::make_unique<Set>(obj->clone(), Token(name), value->clone());
  }
  Super::Super(Token&& key, Token&& mthd) noexcept :
    keyword(std::move(key)),
    method(std::move(mthd))
  {
  }
  std::unique_ptr<Expr> Super::clone()
  {
    return std::make_unique<Super>(Token(keyword), Token(method));
  }
  This::This(Token&& tk) noexcept :
    keyword(tk)
  {
  }
  std::unique_ptr<Expr> This::clone()
  {
    return std::make_unique<This>(Token(keyword));
  }
  Tuple::Tuple(std::vector<std::unique_ptr<Expr>>&& es) noexcept :
    exprs(std::move(es))
  {
  }
  std::unique_ptr<Expr> Tuple::clone()
  {
    auto es = exprs
      | ranges::views::transform([](auto& e) {return e->clone(); })
      | ranges::to<std::vector<std::unique_ptr<Expr>>>;
    return std::make_unique<Tuple>(std::move(es));
  }
  Unary::Unary(Token&& tk, std::unique_ptr<Expr>&& r) noexcept :
    op(std::move(tk)), right(std::move(r))
  {
  }
  std::unique_ptr<Expr> Unary::clone()
  {
    return std::make_unique<Unary>(Token(op), right->clone());
  }
}