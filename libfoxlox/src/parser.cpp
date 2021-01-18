#include <fmt/format.h>

#include "common.h"
#include "parser.h"

namespace
{
  class ParseError
  {
  };
}

namespace foxlox
{
  template<typename ...TokenTypes>
  bool Parser::match(TokenTypes ...types) // match any
  {
    if ((check(types) || ...))
    {
      advance();
      return true;
    }
    return false;
  }
  Parser::Parser(std::vector<Token>&& tokens) : tokens(std::move(tokens))
  {
    current = 0;
    had_error = false;
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
      if (match(TokenType::CLASS)) { return class_declaration(); }
      if (match(TokenType::FUN)) { return function("function"); }
      if (match(TokenType::VAR)) { return var_declaration(); }
      return statement();
    }
    catch (ParseError)
    {
      synchronize();
      return nullptr;
    }
  }
  std::unique_ptr<stmt::Stmt> Parser::class_declaration()
  {
    Token name = consume(TokenType::IDENTIFIER, "Expect class name.");
    std::unique_ptr<expr::Expr> superclass = nullptr;
    if (match(TokenType::COLON))
    {
      superclass = expression();
    }
    consume(TokenType::LEFT_BRACE, "Expect `{' before class body.");

    std::vector<std::unique_ptr<stmt::Function>> methods;
    while (!check(TokenType::RIGHT_BRACE) && !is_at_end())
    {
      methods.emplace_back(function("method"));
    }

    consume(TokenType::RIGHT_BRACE, "Expect `}' after class body.");

    return std::make_unique<stmt::Class>(std::move(name), std::move(superclass), std::move(methods));
  }
  std::unique_ptr<stmt::Function> Parser::function(std::string_view kind)
  {
    auto name = consume(TokenType::IDENTIFIER, fmt::format("Expect {} name.", kind));
    consume(TokenType::LEFT_PAREN, fmt::format("Expect `(' after {} name.", kind));
    std::vector<Token> parameters;
    if (!check(TokenType::RIGHT_PAREN))
    {
      do
      {
        parameters.emplace_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
      } while (match(TokenType::COMMA));
    }
    if (parameters.size() >= 255)
    {
      error(peek(), "Can't have more than 255 parameters.");
    }
    consume(TokenType::RIGHT_PAREN, "Expect `)' after parameters.");
    consume(TokenType::LEFT_BRACE, fmt::format("Expect `{{' before {} body.", kind));
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
    auto name = consume(TokenType::IDENTIFIER, "Expect variable name.");
    auto initializer = match(TokenType::EQUAL) ? expression() : nullptr;
    consume(TokenType::SEMICOLON, "Expect `;' after variable declaration.");
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
    consume(TokenType::SEMICOLON, "Expect `;' after return value.");
    return std::make_unique<stmt::Return>(std::move(keyword), std::move(value));
  }
  std::unique_ptr<stmt::Stmt> Parser::break_statement()
  {
    auto keyword = previous();
    consume(TokenType::SEMICOLON, "Expect `;' after `break'.");
    return std::make_unique<stmt::Break>(std::move(keyword));
  }
  std::unique_ptr<stmt::Stmt> Parser::continue_statement()
  {
    auto keyword = previous();
    consume(TokenType::SEMICOLON, "Expect `;' after `continue'.");
    return std::make_unique<stmt::Continue>(std::move(keyword));
  }
  std::unique_ptr<stmt::Stmt> Parser::for_statement()
  {
    consume(TokenType::LEFT_PAREN, "Expect `(' after `for'.");
    auto initializer =
      match(TokenType::SEMICOLON) ? nullptr :
      match(TokenType::VAR) ? var_declaration() :
      expression_statement();
    auto condition = check(TokenType::SEMICOLON) ? nullptr : expression();
    consume(TokenType::SEMICOLON, "Expect `;' after loop condition.");
    auto increment = check(TokenType::RIGHT_PAREN) ? nullptr : expression();
    auto r_paren = consume(TokenType::RIGHT_PAREN, "Expect `)' after for clauses.");
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
    consume(TokenType::LEFT_PAREN, "Expect `(' after `while'.");
    auto condition = expression();
    auto r_paren = consume(TokenType::RIGHT_PAREN, "Expect `)' after condition.");
    auto body = statement();
    return std::make_unique<stmt::While>(std::move(condition), std::move(body), std::move(r_paren));
  }
  std::unique_ptr<stmt::Stmt> Parser::if_statement()
  {
    consume(TokenType::LEFT_PAREN, "Expect `(' after `if'.");
    auto condition = expression();
    auto r_paren = consume(TokenType::RIGHT_PAREN, "Expect `)' after if condition.");
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
    consume(TokenType::RIGHT_BRACE, "Expect `}' after block.");
    return std::move(statements);
  }
  std::unique_ptr<stmt::Stmt> Parser::expression_statement()
  {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expect `;' after expression.");
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
        TokenType tk = 
          equals.type == TokenType::PLUS_EQUAL ? TokenType::PLUS :
          equals.type == TokenType::MINUS_EQUAL ? TokenType::MINUS :
          equals.type == TokenType::STAR_EQUAL ? TokenType::STAR :
          equals.type == TokenType::SLASH_EQUAL ? TokenType::SLASH :
          TokenType::SLASH_SLASH;
        value = std::make_unique<expr::Binary>(std::move(expr->clone()), 
          Token(tk, equals.lexeme, equals.literal, equals.line), std::move(value));
      }
      auto p_expr = expr.get();
      if (auto variable = dynamic_cast<expr::Variable*>(p_expr); variable != nullptr)
      {
        return std::make_unique<expr::Assign>(std::move(variable->name), std::move(value));
      }
      if (auto get = dynamic_cast<expr::Get*>(p_expr); get != nullptr)
      {
        return std::make_unique<expr::Set>(std::move(get->obj), std::move(get->name), std::move(value));
      }
      error(equals, "Invalid assignment target.");
    }
    return std::move(expr);
  }
  std::unique_ptr<expr::Expr> Parser::or_expr()
  {
    auto expr = and_expr();
    while (match(TokenType::OR))
    {
      auto op = previous();
      auto right = and_expr();
      expr = std::make_unique<expr::Logical>(std::move(expr), std::move(op), std::move(right));
    }
    return std::move(expr);
  }
  std::unique_ptr<expr::Expr> Parser::and_expr()
  {
    auto expr = equality();
    while (match(TokenType::AND))
    {
      auto op = previous();
      auto right = equality();
      expr = std::make_unique<expr::Logical>(std::move(expr), std::move(op), std::move(right));
    }
    return std::move(expr);
  }
  std::unique_ptr<expr::Expr> Parser::equality()
  {
    auto expr = comparison();
    while (match(TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL))
    {
      auto op = previous();
      auto right = comparison();
      expr = std::make_unique<expr::Binary>(std::move(expr), std::move(op), std::move(right));
    }
    return std::move(expr);
  }
  std::unique_ptr<expr::Expr> Parser::comparison()
  {
    auto expr = term();
    while (match(TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL))
    {
      auto op = previous();
      auto right = term();
      expr = std::make_unique<expr::Binary>(std::move(expr), std::move(op), std::move(right));
    }
    return std::move(expr);
  }
  std::unique_ptr<expr::Expr> Parser::term()
  {
    auto expr = factor();
    while (match(TokenType::MINUS, TokenType::PLUS))
    {
      auto op = previous();
      auto right = factor();
      expr = std::make_unique<expr::Binary>(std::move(expr), std::move(op), std::move(right));
    }
    return std::move(expr);
  }
  std::unique_ptr<expr::Expr> Parser::factor()
  {
    auto expr = unary();
    while (match(TokenType::SLASH, TokenType::STAR, TokenType::SLASH_SLASH))
    {
      Token op = previous();
      auto right = unary();
      expr = std::make_unique<expr::Binary>(std::move(expr), std::move(op), std::move(right));
    }
    return std::move(expr);
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
      auto literal_one = std::make_unique<expr::Literal>(CompiletimeValue(int64_t(1)), op);
      Token tk(
        op.type == TokenType::PLUS_PLUS ? TokenType::PLUS : TokenType::MINUS,
        op.lexeme, 
        op.literal, 
        op.line);
      if (auto variable = dynamic_cast<expr::Variable*>(right.get()); variable != nullptr)
      {
        auto assigned_to = variable->name;
        auto bin = std::make_unique<expr::Binary>(std::move(right), std::move(tk), std::move(literal_one));
        return std::make_unique<expr::Assign>(std::move(assigned_to), std::move(bin));
      }
      if (auto get = dynamic_cast<expr::Get*>(right.get()); get != nullptr)
      {
        auto set_to_obj = get->obj->clone();
        auto set_to_name = get->name;
        auto bin = std::make_unique<expr::Binary>(std::move(right), std::move(tk), std::move(literal_one));
        return std::make_unique<expr::Set>(std::move(set_to_obj), std::move(set_to_name), std::move(bin));
      }
    }
    return call();
  }
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
        Token name = consume(TokenType::IDENTIFIER, "Expect property name after `.'.");
        expr = std::make_unique<expr::Get>(std::move(expr), std::move(name));
      }
      else { break; }
    }
    return std::move(expr);
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
    Token paren = consume(TokenType::RIGHT_PAREN, "Expect `)' after arguments.");
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
      consume(TokenType::RIGHT_PAREN, "Expect `)' after expression.");
      return std::make_unique<expr::Grouping>(std::move(expr));
    }
    if (match(TokenType::THIS))
    {
      return std::make_unique<expr::This>(previous());
    }
    if (match(TokenType::SUPER))
    {
      Token keyword = previous();
      consume(TokenType::DOT, "Expect `.' after `super'.");
      Token method = consume(TokenType::IDENTIFIER, "Expect superclass method name.");
      return std::make_unique<expr::Super>(std::move(keyword), std::move(method));
    }
    if (match(TokenType::IDENTIFIER))
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
      consume(TokenType::COMMA, "Expect `,' after expression.");
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
      if (tk.type == TokenType::TKERROR)
      {
        error(tk, tk.lexeme);
      }
      else
      {
        return std::move(tk);
      }
    }
  }

  bool Parser::is_at_end()
  {
    return peek().type == TokenType::TKEOF;
  }

  Token Parser::peek()
  {
    return tokens[current];
  }

  Token Parser::previous()
  {
    return tokens[current - 1];
  }

  Token Parser::consume(TokenType type, std::string_view message)
  {
    if (check(type))
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
      }

      advance();
    }
  }
}
