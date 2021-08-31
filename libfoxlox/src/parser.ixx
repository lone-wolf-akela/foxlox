module;
#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>
export module foxlox:parser;

import <vector>;
import <memory>;
import <string_view>;
import <format>;
import <concepts>;

import :token;
import :value;
import :stmt;
import :format_error;


namespace foxlox
{
  export using AST = std::vector<std::unique_ptr<stmt::Stmt>>;
  export class Parser
  {
  public:
    explicit Parser(std::vector<Token>&& tokens) noexcept;
    void define(std::string_view name, CppFunc* func);
    AST parse();
    bool get_had_error() noexcept;
  private:
    AST ast;
    const std::vector<Token> tokens;
    gsl::index current;
    bool had_error;

    std::unique_ptr<stmt::Stmt> declaration();
    std::unique_ptr<stmt::Stmt> export_declaration();
    std::unique_ptr<stmt::Stmt> class_declaration();
    std::unique_ptr<stmt::Stmt> from_declaration();
    std::unique_ptr<stmt::Stmt> import_declaration();
    std::unique_ptr<stmt::Function> function(std::string_view kind);
    std::unique_ptr<stmt::Stmt> var_declaration();
    std::unique_ptr<stmt::Stmt> statement();
    std::unique_ptr<stmt::Stmt> return_statement();
    std::unique_ptr<stmt::Stmt> break_statement();
    std::unique_ptr<stmt::Stmt> continue_statement();
    std::unique_ptr<stmt::Stmt> for_statement();
    std::unique_ptr<stmt::Stmt> while_statement();
    std::unique_ptr<stmt::Stmt> if_statement();
    std::vector<std::unique_ptr<stmt::Stmt>> block();
    std::unique_ptr<stmt::Stmt> expression_statement();
    std::unique_ptr<expr::Expr> expression();
    std::unique_ptr<expr::Expr> assignment();
    std::unique_ptr<expr::Expr> assignment_impl(Token equals, std::unique_ptr<expr::Expr>&& left, std::unique_ptr<expr::Expr>&& right);
    std::unique_ptr<expr::Expr> or_expr();
    std::unique_ptr<expr::Expr> and_expr();
    std::unique_ptr<expr::Expr> equality();
    std::unique_ptr<expr::Expr> comparison();
    std::unique_ptr<expr::Expr> term();
    std::unique_ptr<expr::Expr> factor();
    std::unique_ptr<expr::Expr> unary();
    std::unique_ptr<expr::Expr> call();
    std::unique_ptr<expr::Expr> finish_call(std::unique_ptr<expr::Expr>&& callee);
    std::unique_ptr<expr::Expr> primary();
    std::unique_ptr<expr::Expr> tuple(std::unique_ptr<expr::Expr>&& first);

    template<std::same_as<TokenType> ... Args>
    bool match(Args ... types);
    bool check(TokenType type);
    Token advance();
    bool is_at_end();
    Token peek();
    Token previous();
    template<std::same_as<TokenType> ... Args>
    Token consume(std::string_view message, Args ... types);
    void error(Token token, std::string_view message);
    void synchronize();
  };
}

namespace
{
  class ParseError
  {
  };
}

namespace foxlox
{
  template<std::same_as<TokenType> ... Args>
  bool Parser::match(Args ... types) // match any
  {
    if ((check(types) || ...))
    {
      advance();
      return true;
    }
    return false;
  }
  Parser::Parser(std::vector<Token>&& tokens) noexcept :
    tokens(std::move(tokens)),
    current(0),
    had_error(false)
  {
  }
  void Parser::define(std::string_view name, CppFunc* func)
  {
    Token tk_name(TokenType::IDENTIFIER, name, {}, 0);
    auto initializer = std::make_unique<expr::Literal>(CompiletimeValue(func), tk_name);
    auto var_stmt = std::make_unique<stmt::Var>(std::move(tk_name), std::move(initializer));
    ast.emplace_back(std::move(var_stmt));
  }
  AST Parser::parse()
  {
    had_error = false;

    while (!is_at_end())
    {
      ast.emplace_back(declaration());
    }
    return std::move(ast);
  }
  bool Parser::get_had_error() noexcept
  {
    return had_error;
  }
  std::unique_ptr<stmt::Stmt> Parser::declaration()
  {
    try
    {
      if (match(TokenType::EXPORT)) { return export_declaration(); }
      if (match(TokenType::CLASS)) { return class_declaration(); }
      if (match(TokenType::FUN)) { return function("function"); }
      if (match(TokenType::VAR)) { return var_declaration(); }
      if (match(TokenType::FROM)) { return from_declaration(); }
      if (match(TokenType::IMPORT)) { return import_declaration(); }
      return statement();
    }
    catch (ParseError)
    {
      synchronize();
      return nullptr;
    }
  }
  std::unique_ptr<stmt::Stmt> Parser::export_declaration()
  {
    auto keyword = previous();
    std::unique_ptr<stmt::Stmt> dec = declaration();
    if (dynamic_cast<stmt::VarDeclareBase*>(dec.get()) == nullptr)
    {
      error(keyword, "Export from a `from' statement is not allowed.");
    }
    return std::make_unique<stmt::Export>(std::move(keyword), std::move(dec));
  }
  std::unique_ptr<stmt::Stmt> Parser::class_declaration()
  {
    Token name = consume("Expect class name.", TokenType::IDENTIFIER);
    std::unique_ptr<expr::Expr> superclass = nullptr;
    if (match(TokenType::COLON))
    {
      superclass = expression();
    }
    consume("Expect `{' before class body.", TokenType::LEFT_BRACE);

    std::vector<std::unique_ptr<stmt::Function>> methods;
    while (!check(TokenType::RIGHT_BRACE) && !is_at_end())
    {
      methods.emplace_back(function("method"));
    }

    consume("Expect `}' after class body.", TokenType::RIGHT_BRACE);

    return std::make_unique<stmt::Class>(std::move(name), std::move(superclass), std::move(methods));
  }
  std::unique_ptr<stmt::Stmt> Parser::from_declaration()
  {
    std::vector<Token> path;
    do
    {
      path.emplace_back(consume("Expect valid lib path.", TokenType::IDENTIFIER));
    } while (match(TokenType::DOT));
    consume("Expect `import'.", TokenType::IMPORT);
    std::vector<Token> vars;
    do
    {
      vars.emplace_back(consume("Expect valid variable name.", TokenType::IDENTIFIER));
    } while (match(TokenType::COMMA));
    consume("Expect `;' after `from' statement.", TokenType::SEMICOLON);
    return std::make_unique<stmt::From>(std::move(vars), std::move(path));
  }
  std::unique_ptr<stmt::Stmt> Parser::import_declaration()
  {
    std::vector<Token> path;
    do
    {
      path.emplace_back(consume("Expect valid lib path.", TokenType::IDENTIFIER));
    } while (match(TokenType::DOT));
    Token name = match(TokenType::AS) ?
      consume("Expect variable name.", TokenType::IDENTIFIER) : path.back();
    consume("Expect `;' after `import' statement.", TokenType::SEMICOLON);
    return std::make_unique<stmt::Import>(std::move(name), std::move(path));
  }
  std::unique_ptr<stmt::Function> Parser::function(std::string_view kind)
  {
    auto name = consume(std::format("Expect {} name.", kind), TokenType::IDENTIFIER);
    consume(std::format("Expect `(' after {} name.", kind), TokenType::LEFT_PAREN);
    std::vector<Token> parameters;
    if (!check(TokenType::RIGHT_PAREN))
    {
      do
      {
        parameters.emplace_back(consume("Expect parameter name.", TokenType::IDENTIFIER));
      } while (match(TokenType::COMMA));
    }
    if (parameters.size() >= 255)
    {
      error(peek(), "Can't have more than 255 parameters.");
    }
    consume("Expect `)' after parameters.", TokenType::RIGHT_PAREN);
    consume(std::format("Expect `{{' before {} body.", kind), TokenType::LEFT_BRACE);
    auto body = block();
    if (body.empty() || dynamic_cast<stmt::Return*>(body.back().get()) == nullptr)
    {
      // there's no return at the end of a function
      // let's add one
      auto ret = std::make_unique<stmt::Return>(Token(TokenType::RETURN, "", {}, name.line), nullptr);
      body.emplace_back(std::move(ret));
    }
    return std::make_unique<stmt::Function>(std::move(name), std::move(parameters), std::move(body));
  }
  std::unique_ptr<stmt::Stmt> Parser::var_declaration()
  {
    auto name = consume("Expect variable name.", TokenType::IDENTIFIER, TokenType::UNDERLINE);
    auto initializer = match(TokenType::EQUAL) ? expression() : nullptr;
    consume("Expect `;' after variable declaration.", TokenType::SEMICOLON);
    return std::make_unique<stmt::Var>(std::move(name), std::move(initializer));
  }
  std::unique_ptr<stmt::Stmt> Parser::statement()
  {
    if (match(TokenType::FOR)) { return for_statement(); }
    if (match(TokenType::IF)) { return if_statement(); }
    if (match(TokenType::RETURN)) { return return_statement(); }
    if (match(TokenType::BREAK)) { return break_statement(); }
    if (match(TokenType::CONTINUE)) { return continue_statement(); }
    if (match(TokenType::WHILE)) { return while_statement(); }
    if (match(TokenType::LEFT_BRACE)) { return std::make_unique<stmt::Block>(block()); }
    return expression_statement();
  }
  std::unique_ptr<stmt::Stmt> Parser::return_statement()
  {
    auto keyword = previous();
    std::unique_ptr<expr::Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON))
    {
      value = expression();
    }
    consume("Expect `;' after return value.", TokenType::SEMICOLON);
    return std::make_unique<stmt::Return>(std::move(keyword), std::move(value));
  }
  std::unique_ptr<stmt::Stmt> Parser::break_statement()
  {
    auto keyword = previous();
    consume("Expect `;' after `break'.", TokenType::SEMICOLON);
    return std::make_unique<stmt::Break>(std::move(keyword));
  }
  std::unique_ptr<stmt::Stmt> Parser::continue_statement()
  {
    auto keyword = previous();
    consume("Expect `;' after `continue'.", TokenType::SEMICOLON);
    return std::make_unique<stmt::Continue>(std::move(keyword));
  }
  std::unique_ptr<stmt::Stmt> Parser::for_statement()
  {
    consume("Expect `(' after `for'.", TokenType::LEFT_PAREN);
    auto initializer =
      match(TokenType::SEMICOLON) ? nullptr :
      match(TokenType::VAR) ? var_declaration() :
      expression_statement();
    auto condition = check(TokenType::SEMICOLON) ? nullptr : expression();
    consume("Expect `;' after loop condition.", TokenType::SEMICOLON);
    auto increment = check(TokenType::RIGHT_PAREN) ? nullptr : expression();
    auto r_paren = consume("Expect `)' after for clauses.", TokenType::RIGHT_PAREN);
    auto body = statement();
    return std::make_unique<stmt::For>(
      std::move(initializer),
      std::move(condition),
      std::move(increment),
      std::move(body),
      std::move(r_paren)
      );
  }
  std::unique_ptr<stmt::Stmt> Parser::while_statement()
  {
    consume("Expect `(' after `while'.", TokenType::LEFT_PAREN);
    auto condition = expression();
    auto r_paren = consume("Expect `)' after condition.", TokenType::RIGHT_PAREN);
    auto body = statement();
    return std::make_unique<stmt::While>(std::move(condition), std::move(body), std::move(r_paren));
  }
  std::unique_ptr<stmt::Stmt> Parser::if_statement()
  {
    consume("Expect `(' after `if'.", TokenType::LEFT_PAREN);
    auto condition = expression();
    auto r_paren = consume("Expect `)' after if condition.", TokenType::RIGHT_PAREN);
    auto then_branch = statement();
    auto else_branch = match(TokenType::ELSE) ? statement() : nullptr;
    return std::make_unique<stmt::If>(std::move(condition), std::move(then_branch), std::move(else_branch), std::move(r_paren));
  }
  std::vector<std::unique_ptr<stmt::Stmt>> Parser::block()
  {
    std::vector<std::unique_ptr<stmt::Stmt>> statements;
    while (!check(TokenType::RIGHT_BRACE) && !is_at_end())
    {
      statements.emplace_back(declaration());
    }
    consume("Expect `}' after block.", TokenType::RIGHT_BRACE);
    return statements;
  }
  std::unique_ptr<stmt::Stmt> Parser::expression_statement()
  {
    auto expr = expression();
    consume("Expect `;' after expression.", TokenType::SEMICOLON);
    return std::make_unique<stmt::Expression>(std::move(expr));
  }
  std::unique_ptr<expr::Expr> Parser::expression()
  {
    return assignment();
  }
  std::unique_ptr<expr::Expr> Parser::assignment()
  {
    auto expr = or_expr();
    if (match(
      TokenType::EQUAL,
      TokenType::PLUS_EQUAL,
      TokenType::MINUS_EQUAL,
      TokenType::STAR_EQUAL,
      TokenType::SLASH_EQUAL,
      TokenType::SLASH_SLASH_EQUAL))
    {
      auto equals = previous();
      auto value = assignment();
      // desugaring
      if (equals.type != TokenType::EQUAL)
      {
        const TokenType tk =
          equals.type == TokenType::PLUS_EQUAL ? TokenType::PLUS :
          equals.type == TokenType::MINUS_EQUAL ? TokenType::MINUS :
          equals.type == TokenType::STAR_EQUAL ? TokenType::STAR :
          equals.type == TokenType::SLASH_EQUAL ? TokenType::SLASH :
          TokenType::SLASH_SLASH;
        value = std::make_unique<expr::Binary>(expr->clone(),
          Token(tk, equals.lexeme, equals.literal, equals.line), std::move(value));
      }
      if (auto tuple = dynamic_cast<expr::Tuple*>(expr.get()); tuple != nullptr)
      {
        auto assign_list = tuple->exprs
          | ranges::views::transform([=,this](auto&& e) {
              return assignment_impl(equals, std::move(e), std::make_unique<expr::NoOP>());
            })
          | ranges::to<std::vector<std::unique_ptr<expr::Expr>>>;
        return std::make_unique<expr::TupleUnpack>(std::move(value), std::move(assign_list));
      }
      else
      {
        return assignment_impl(equals, std::move(expr), std::move(value));
      }
    }
    return expr;
  }
  std::unique_ptr<expr::Expr> Parser::assignment_impl(Token equals, std::unique_ptr<expr::Expr>&& left, std::unique_ptr<expr::Expr>&& right)
  {
    auto p_expr = left.get();
    if (auto variable = dynamic_cast<expr::Variable*>(p_expr); variable != nullptr)
    {
      return std::make_unique<expr::Assign>(std::move(variable->name), std::move(right));
    }
    if (auto get = dynamic_cast<expr::Get*>(p_expr); get != nullptr)
    {
      return std::make_unique<expr::Set>(std::move(get->obj), std::move(get->name), std::move(right));
    }
    error(equals, "Invalid assignment target.");
    return std::move(left);
  }
  GSL_SUPPRESS(r.5)
    std::unique_ptr<expr::Expr> Parser::or_expr()
  {
    auto expr = and_expr();
    while (match(TokenType::OR))
    {
      auto op = previous();
      auto right = and_expr();
      expr = std::make_unique<expr::Logical>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
  }
  GSL_SUPPRESS(r.5)
    std::unique_ptr<expr::Expr> Parser::and_expr()
  {
    auto expr = equality();
    while (match(TokenType::AND))
    {
      auto op = previous();
      auto right = equality();
      expr = std::make_unique<expr::Logical>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
  }
  GSL_SUPPRESS(r.5)
    std::unique_ptr<expr::Expr> Parser::equality()
  {
    auto expr = comparison();
    while (match(TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL))
    {
      auto op = previous();
      auto right = comparison();
      expr = std::make_unique<expr::Binary>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
  }
  GSL_SUPPRESS(r.5)
    std::unique_ptr<expr::Expr> Parser::comparison()
  {
    auto expr = term();
    while (match(TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL))
    {
      auto op = previous();
      auto right = term();
      expr = std::make_unique<expr::Binary>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
  }
  GSL_SUPPRESS(r.5)
    std::unique_ptr<expr::Expr> Parser::term()
  {
    auto expr = factor();
    while (match(TokenType::MINUS, TokenType::PLUS))
    {
      auto op = previous();
      auto right = factor();
      expr = std::make_unique<expr::Binary>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
  }
  GSL_SUPPRESS(r.5)
    std::unique_ptr<expr::Expr> Parser::factor()
  {
    auto expr = unary();
    while (match(TokenType::SLASH, TokenType::STAR, TokenType::SLASH_SLASH))
    {
      Token op = previous();
      auto right = unary();
      expr = std::make_unique<expr::Binary>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
  }
  std::unique_ptr<expr::Expr> Parser::unary()
  {
    if (match(TokenType::BANG, TokenType::MINUS))
    {
      Token op = previous();
      std::unique_ptr<expr::Expr> right = unary();
      return std::make_unique<expr::Unary>(std::move(op), std::move(right));
    }
    if (match(TokenType::PLUS_PLUS, TokenType::MINUS_MINUS))
    {
      Token op = previous();
      //de-sugarlize
      std::unique_ptr<expr::Expr> right = unary();
      auto literal_one = std::make_unique<expr::Literal>(CompiletimeValue(int64_t{ 1 }), op);
      Token tk(
        op.type == TokenType::PLUS_PLUS ? TokenType::PLUS : TokenType::MINUS,
        op.lexeme,
        op.literal,
        op.line);
      if (auto variable = dynamic_cast<expr::Variable const*>(right.get()); variable != nullptr)
      {
        auto assigned_to = variable->name;
        auto bin = std::make_unique<expr::Binary>(std::move(right), std::move(tk), std::move(literal_one));
        return std::make_unique<expr::Assign>(std::move(assigned_to), std::move(bin));
      }
      if (auto get = dynamic_cast<expr::Get const*>(right.get()); get != nullptr)
      {
        auto set_to_obj = get->obj->clone();
        auto set_to_name = get->name;
        auto bin = std::make_unique<expr::Binary>(std::move(right), std::move(tk), std::move(literal_one));
        return std::make_unique<expr::Set>(std::move(set_to_obj), std::move(set_to_name), std::move(bin));
      }
    }
    return call();
  }
  GSL_SUPPRESS(r.5)
    std::unique_ptr<expr::Expr> Parser::call()
  {
    auto expr = primary();
    while (true)
    {
      if (match(TokenType::LEFT_PAREN))
      {
        expr = finish_call(std::move(expr));
      }
      else if (match(TokenType::DOT))
      {
        Token name = consume("Expect property name after `.'.", TokenType::IDENTIFIER);
        expr = std::make_unique<expr::Get>(std::move(expr), std::move(name));
      }
      else { break; }
    }
    return expr;
  }

  std::unique_ptr<expr::Expr> Parser::finish_call(std::unique_ptr<expr::Expr>&& callee)
  {
    std::vector<std::unique_ptr<expr::Expr>> arguments;
    if (!check(TokenType::RIGHT_PAREN))
    {
      do
      {
        arguments.emplace_back(expression());
      } while (match(TokenType::COMMA));
    }
    Token paren = consume("Expect `)' after arguments.", TokenType::RIGHT_PAREN);
    if (arguments.size() >= 255)
    {
      error(peek(), "Can't have more than 255 arguments.");
    }
    return std::make_unique<expr::Call>(std::move(callee), std::move(paren), std::move(arguments));
  }

  std::unique_ptr<expr::Expr> Parser::primary()
  {
    if (match(TokenType::FALSE))
    {
      return std::make_unique<expr::Literal>(false, previous());
    }
    if (match(TokenType::TRUE))
    {
      return std::make_unique<expr::Literal>(true, previous());
    }
    if (match(TokenType::NIL))
    {
      return std::make_unique<expr::Literal>(CompiletimeValue(), previous());
    }
    if (match(TokenType::INT, TokenType::DOUBLE, TokenType::STRING))
    {
      GSL_SUPPRESS(lifetime.3)
        return std::make_unique<expr::Literal>(std::move(previous().literal), previous());
    }
    if (match(TokenType::LEFT_PAREN))
    {
      if (match(TokenType::RIGHT_PAREN))
      {
        // empty tuple
        return std::make_unique<expr::Tuple>(std::vector<std::unique_ptr<expr::Expr>>{});
      }
      auto expr = expression();
      if (check(TokenType::COMMA))
      {
        return tuple(std::move(expr));
      }
      consume("Expect `)' after expression.", TokenType::RIGHT_PAREN);
      return std::make_unique<expr::Grouping>(std::move(expr));
    }
    if (match(TokenType::THIS))
    {
      return std::make_unique<expr::This>(previous());
    }
    if (match(TokenType::SUPER))
    {
      Token keyword = previous();
      consume("Expect `.' after `super'.", TokenType::DOT);
      Token method = consume("Expect superclass method name.", TokenType::IDENTIFIER);
      return std::make_unique<expr::Super>(std::move(keyword), std::move(method));
    }
    if (match(TokenType::IDENTIFIER, TokenType::UNDERLINE)) // a placeholder `_' is kinda of a special variable
    {
      return std::make_unique<expr::Variable>(previous());
    }

    error(peek(), "Expect expression.");
    throw ParseError();
  }

  std::unique_ptr<expr::Expr> Parser::tuple(std::unique_ptr<expr::Expr>&& first)
  {
    std::vector<std::unique_ptr<expr::Expr>> exprs;
    exprs.emplace_back(std::move(first));
    while (!match(TokenType::RIGHT_PAREN))
    {
      consume("Expect `,' after expression.", TokenType::COMMA);
      if (!match(TokenType::RIGHT_PAREN))
      {
        exprs.emplace_back(expression());
      }
      else
      {
        break;
      }
    }
    return std::make_unique<expr::Tuple>(std::move(exprs));
  }

  bool Parser::check(TokenType type)
  {
    if (is_at_end())
    {
      return false;
    }
    return peek().type == type;
  }

  Token Parser::advance()
  {
    while (true)
    {
      if (is_at_end()) { return peek(); }
      current++;
      auto tk = previous();
      return tk;
    }
  }

  bool Parser::is_at_end()
  {
    return peek().type == TokenType::TKEOF;
  }

  Token Parser::peek()
  {
    return tokens.at(current);
  }

  Token Parser::previous()
  {
    return tokens.at(current - 1);
  }

  template<std::same_as<TokenType> ... Args>
  Token Parser::consume(std::string_view message, Args ... types)
  {
    if ((check(types) || ...))
    {
      return advance();
    }
    error(peek(), message);
    throw ParseError();
  }

  void Parser::error(Token token, std::string_view message)
  {
    format_error(token, message);
    had_error = true;
  }

  void Parser::synchronize()
  {
    advance();
    while (!is_at_end())
    {
      if (previous().type == TokenType::SEMICOLON)
      {
        return;
      }
      switch (peek().type)
      {
      case TokenType::CLASS:
      case TokenType::FUN:
      case TokenType::VAR:
      case TokenType::FOR:
      case TokenType::IF:
      case TokenType::WHILE:
      case TokenType::RETURN:
        return;
      default:
        break;
      }

      advance();
    }
  }
}
