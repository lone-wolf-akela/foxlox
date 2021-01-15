#pragma once

#include <vector>
#include <memory>
#include <string_view>

#include "value.h"
#include "token.h"
#include "stmt.h"

namespace foxlox
{
  using AST = std::vector<std::unique_ptr<stmt::Stmt>>;
  class Parser
  {
  public:
    Parser(std::vector<Token>&& tokens);
    void define(std::string_view name, CppFunc* func);
    AST parse();
    bool get_had_error() noexcept;
  private:
    AST ast;
    const std::vector<Token> tokens;
    int current;
    bool had_error;

    std::unique_ptr<stmt::Stmt> declaration();
    std::unique_ptr<stmt::Stmt> class_declaration();
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

    template<typename ... TokenTypes>
    bool match(TokenTypes ... types);
    bool check(TokenType type);
    Token advance();
    bool is_at_end();
    Token peek();
    Token previous();
    Token consume(TokenType type, std::string_view message);
    void error(Token token, std::string_view message);
    void synchronize();
  };
}
