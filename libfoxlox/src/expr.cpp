#include <gsl/gsl>

#include "expr.h"

namespace foxlox::expr
{
  Assign::Assign(Token&& tk, std::unique_ptr<Expr>&& v) : 
    name(std::move(tk)), 
    value(std::move(v)) 
  {
  }
  std::unique_ptr<Expr> Assign::clone()
  {
    return std::make_unique<Assign>(Token(name), value->clone());
  }
  Binary::Binary(std::unique_ptr<Expr>&& l, Token&& tk, std::unique_ptr<Expr>&& r) :
    left(std::move(l)), 
    op(std::move(tk)), 
    right(std::move(r))
  {
  }
  std::unique_ptr<Expr> Binary::clone()
  {
    return std::make_unique<Binary>(left->clone(), Token(op), right->clone());
  }
  Logical::Logical(std::unique_ptr<Expr>&& l, Token&& tk, std::unique_ptr<Expr>&& r) :
    left(std::move(l)), 
    op(std::move(tk)), 
    right(std::move(r))
  {
  }
  std::unique_ptr<Expr> Logical::clone()
  {
    return std::make_unique<Logical>(left->clone(), Token(op), right->clone());
  }
  Grouping::Grouping(std::unique_ptr<Expr>&& expr) 
    : expression(std::move(expr)) 
  {
  }
  std::unique_ptr<Expr> Grouping::clone()
  {
    return std::make_unique<Grouping>(expression->clone());
  }
  Literal::Literal(CompiletimeValue&& v) : 
    value(std::move(v)) 
  {
  }
  Literal::Literal(const CompiletimeValue& v) : 
    value(v) 
  {
  }
  std::unique_ptr<Expr> Literal::clone()
  {
    return std::make_unique<Literal>(value);
  }
  Call::Call(std::unique_ptr<Expr>&& ce, Token&& tk, std::vector<std::unique_ptr<Expr>>&& augs) :
    callee(std::move(ce)), 
    paren(std::move(tk)), 
    arguments(std::move(augs))
  {
  }
  std::unique_ptr<Expr> Call::clone()
  {
    std::vector<std::unique_ptr<Expr>> augs(arguments.size());
    for (gsl::index i = 0; i < ssize(arguments); i++)
    {
      augs[i] = arguments[i]->clone();
    }
    return std::make_unique<Call>(callee->clone(), Token(paren), std::move(augs));
  }
  Variable::Variable(Token&& tk) : 
    name(std::move(tk)) 
  {
  }
  std::unique_ptr<Expr> Variable::clone()
  {
    return std::make_unique<Variable>(Token(name));
  }
  Get::Get(std::unique_ptr<Expr>&& o, Token&& tk) : 
    obj(std::move(o)), 
    name(std::move(tk)) 
  {
  }
  std::unique_ptr<Expr> Get::clone()
  {
    return std::make_unique<Get>(obj->clone(), Token(name));
  }
  Set::Set(std::unique_ptr<Expr>&& o, Token&& tk, std::unique_ptr<Expr>&& v) :
    obj(std::move(o)), 
    name(std::move(tk)), 
    value(std::move(v))
  {
  }
  std::unique_ptr<Expr> Set::clone()
  {
    return std::make_unique<Set>(obj->clone(), Token(name), value->clone());
  }
  Super::Super(Token&& key, Token&& mthd) : 
    keyword(std::move(key)), 
    method(std::move(mthd)) 
  {
  }
  std::unique_ptr<Expr> Super::clone()
  {
    return std::make_unique<Super>(Token(keyword), Token(method));
  }
  This::This(Token&& tk) : 
    keyword(tk) 
  {
  }
  std::unique_ptr<Expr> This::clone()
  {
    return std::make_unique<This>(Token(keyword));
  }
  Tuple::Tuple(std::vector<std::unique_ptr<Expr>>&& es):
    exprs(std::move(es))
  {
  }
  std::unique_ptr<Expr> Tuple::clone()
  {
    std::vector<std::unique_ptr<Expr>> es(exprs.size());
    for (gsl::index i = 0; i < ssize(exprs); i++)
    {
      es[i] = exprs[i]->clone();
    }
    return std::make_unique<Tuple>(std::move(es));
  }
  Unary::Unary(Token&& tk, std::unique_ptr<Expr>&& r) : 
    op(std::move(tk)), right(std::move(r)) 
  {
  }
  std::unique_ptr<Expr> Unary::clone()
  {
    return std::make_unique<Unary>(Token(op), right->clone());
  }
}