export module foxlox:stmt;

import <memory>;
import <vector>;

import <gsl/gsl>;

import :except;
import :token;
import :expr;

namespace foxlox::stmt
{
  export enum class VarStoreType
  {
    Stack, // to be stored on stack, and destroyed when function returns 
    Static, // to be stored in the value pool of the closure, and still exists after function exits
  };

  export class Stmt
  {
  public:
    Stmt() = default;
    Stmt(const Stmt&) = delete;
    Stmt(Stmt&&) = delete;
    Stmt& operator=(const Stmt&) = delete;
    Stmt& operator=(Stmt&&) = delete;
    virtual ~Stmt() = default;
  };

  export class VarDeclareBase : public virtual Stmt
  {
  public:
    VarDeclareBase(Token&& nm) noexcept;
    Token name;
    VarStoreType store_type; // to be filled by resolver
    virtual ~VarDeclareBase() = default;
  };

  export class VarDeclareListBase : public virtual Stmt
  {
  public:
    VarDeclareListBase(std::vector<Token>&& names) noexcept;
    std::vector<Token> var_names{};
    std::vector<VarStoreType> store_type_list{}; // to be filled by resolver
    virtual ~VarDeclareListBase() = default;
  };

  export class Export : public Stmt
  {
  public:
    Export(Token&& tk, std::unique_ptr<stmt::Stmt>&& d) noexcept;
    Token keyword;
    std::unique_ptr<stmt::Stmt> declare;
  };

  export class Expression : public Stmt
  {
  public:
    Expression(std::unique_ptr<expr::Expr>&& expr) noexcept;
    std::unique_ptr<expr::Expr> expression;
  };

  export class Var : public VarDeclareBase
  {
  public:
    Var(Token&& tk, std::unique_ptr<expr::Expr>&& init) noexcept;
    std::unique_ptr<expr::Expr> initializer;
  };

  export class While : public Stmt
  {
  public:
    While(std::unique_ptr<expr::Expr>&& cond, std::unique_ptr<Stmt>&& bd, Token&& r_paren) noexcept;
    std::unique_ptr<expr::Expr> condition;
    std::unique_ptr<Stmt> body;

    // for error reporting
    Token right_paren;
  };

  export class Block : public Stmt
  {
  public:
    Block(std::vector<std::unique_ptr<Stmt>>&& stmts) noexcept;
    std::vector<std::unique_ptr<Stmt>> statements;
  };

  export class If : public Stmt
  {
  public:
    If(
      std::unique_ptr<expr::Expr>&& cond,
      std::unique_ptr<Stmt>&& thenb,
      std::unique_ptr<Stmt>&& elseb,
      Token&& r_paren
    ) noexcept;
    std::unique_ptr<expr::Expr> condition;
    std::unique_ptr<Stmt> then_branch;
    std::unique_ptr<Stmt> else_branch;

    // for error reporting
    Token right_paren;
  };

  export class Function : public VarDeclareBase, public VarDeclareListBase
  {
  public:
    Function(Token&& tk, std::vector<Token>&& par, std::vector<std::unique_ptr<Stmt>>&& bd) noexcept;
    std::vector<std::unique_ptr<Stmt>> body;
  };

  export class Return : public Stmt
  {
  public:
    Return(Token&& tk, std::unique_ptr<expr::Expr>&& v) noexcept;
    // for error reporting
    Token keyword;
    std::unique_ptr<expr::Expr> value;
  };

  export class Class : public VarDeclareBase
  {
  public:
    Class(Token&& tk, std::unique_ptr<expr::Expr>&& super, std::vector<std::unique_ptr<Function>>&& ms) noexcept;

    std::unique_ptr<expr::Expr> superclass;
    std::vector<std::unique_ptr<Function>> methods;

    // to be filled by resolver, this is refer to the `this' variable
    VarStoreType this_store_type;
  };

  export class Break : public Stmt
  {
  public:
    Break(Token&& tk) noexcept;
    // for error reporting
    Token keyword;
  };

  export class Continue : public Stmt
  {
  public:
    Continue(Token&& tk) noexcept;
    // for error reporting
    Token keyword;
  };

  export class Import : public VarDeclareBase
  {
  public:
    Import(Token&& tk, std::vector<Token>&& path) noexcept;

    std::vector<Token> libpath;
  };

  export class From : public VarDeclareListBase
  {
  public:
    From(std::vector<Token>&& vars, std::vector<Token>&& path) noexcept;
    std::vector<Token> libpath;
  };

  export class For : public Stmt
  {
  public:
    For(
      std::unique_ptr<Stmt>&& init,
      std::unique_ptr<expr::Expr>&& cond,
      std::unique_ptr<expr::Expr>&& incre,
      std::unique_ptr<Stmt>&& bd,
      Token&& r_paren
    ) noexcept;
    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<expr::Expr> condition;
    std::unique_ptr<expr::Expr> increment;
    std::unique_ptr<Stmt> body;

    // for error reporting
    Token right_paren;
  };

  export template<typename R>
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
    virtual R visit_import_stmt(gsl::not_null<Import*> stmt) = 0;
    virtual R visit_from_stmt(gsl::not_null<From*> stmt) = 0;
    virtual R visit_export_stmt(gsl::not_null<Export*> stmt) = 0;

    GSL_SUPPRESS(c.21)
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
      if (auto p = dynamic_cast<Import*>(stmt); p != nullptr)
      {
        return visit_import_stmt(p);
      }
      if (auto p = dynamic_cast<From*>(stmt); p != nullptr)
      {
        return visit_from_stmt(p);
      }
      if (auto p = dynamic_cast<Export*>(stmt); p != nullptr)
      {
        return visit_export_stmt(p);
      }
      throw FatalError("Unknown stmt type");
    }
  };
}

namespace foxlox::stmt
{
  Expression::Expression(std::unique_ptr<expr::Expr>&& expr) noexcept :
    expression(std::move(expr))
  {
  }
  Var::Var(Token&& tk, std::unique_ptr<expr::Expr>&& init) noexcept :
    VarDeclareBase(std::move(tk)),
    initializer(std::move(init))
  {
  }
  While::While(std::unique_ptr<expr::Expr>&& cond, std::unique_ptr<Stmt>&& bd, Token&& r_paren) noexcept :
    condition(std::move(cond)),
    body(std::move(bd)),
    right_paren(std::move(r_paren))
  {
  }
  Block::Block(std::vector<std::unique_ptr<Stmt>>&& stmts) noexcept :
    statements(std::move(stmts))
  {
  }
  If::If(
    std::unique_ptr<expr::Expr>&& cond,
    std::unique_ptr<Stmt>&& thenb,
    std::unique_ptr<Stmt>&& elseb,
    Token&& r_paren
  ) noexcept :
    condition(std::move(cond)),
    then_branch(std::move(thenb)),
    else_branch(std::move(elseb)),
    right_paren(std::move(r_paren))
  {
  }
  Function::Function(Token&& tk, std::vector<Token>&& par, std::vector<std::unique_ptr<Stmt>>&& bd) noexcept :
    VarDeclareBase(std::move(tk)),
    VarDeclareListBase(std::move(par)),
    body(std::move(bd))
  {
  }
  Return::Return(Token&& tk, std::unique_ptr<expr::Expr>&& v) noexcept :
    keyword(std::move(tk)),
    value(std::move(v))
  {
  }
  Class::Class(Token&& tk, std::unique_ptr<expr::Expr>&& super, std::vector<std::unique_ptr<Function>>&& ms) noexcept :
    VarDeclareBase(std::move(tk)),
    superclass(std::move(super)),
    methods(std::move(ms)),
    this_store_type{}
  {
  }
  Break::Break(Token&& tk) noexcept :
    keyword(std::move(tk))
  {
  }
  Continue::Continue(Token&& tk) noexcept :
    keyword(std::move(tk))
  {
  }
  For::For(
    std::unique_ptr<Stmt>&& init,
    std::unique_ptr<expr::Expr>&& cond,
    std::unique_ptr<expr::Expr>&& incre,
    std::unique_ptr<Stmt>&& bd,
    Token&& r_paren
  ) noexcept :
    initializer(std::move(init)),
    condition(std::move(cond)),
    increment(std::move(incre)),
    body(std::move(bd)),
    right_paren(std::move(r_paren))
  {
  }
  Export::Export(Token&& tk, std::unique_ptr<stmt::Stmt>&& d) noexcept :
    keyword(std::move(tk)),
    declare(std::move(d))
  {
  }
  Import::Import(Token&& tk, std::vector<Token>&& path) noexcept :
    VarDeclareBase(std::move(tk)),
    libpath(std::move(path))
  {
  }
  From::From(std::vector<Token>&& vars, std::vector<Token>&& path) noexcept :
    VarDeclareListBase(std::move(vars)),
    libpath(std::move(path))
  {
  }
  VarDeclareBase::VarDeclareBase(Token&& nm) noexcept :
    name(std::move(nm)),
    store_type(VarStoreType::Stack)
  {
  }
  VarDeclareListBase::VarDeclareListBase(std::vector<Token>&& names) noexcept :
    var_names(std::move(names))
  {
  }
}
