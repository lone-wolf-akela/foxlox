#include <gsl/gsl>
#include <range/v3/all.hpp>

#include "expr.h"

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