#pragma once

import <map>;
import <vector>;
import <string>;

#include "expr.h"
#include "stmt.h"
#include "parser.h"

namespace foxlox
{
  struct ValueInfo
  {
    bool is_ready = false;
    VarDeclareAt declare;
  };

  struct Scope
  {
    int function_level{}; // how many layer of nested function are we in?
    std::map<std::string, ValueInfo> vars; // name : info
  };

  class Resolver : public expr::IVisitor<void>, public stmt::IVisitor<void>
  {
  public:
    explicit Resolver(AST&& a) noexcept;
    AST resolve();
    bool get_had_error() noexcept;
  private:
    AST ast;
    bool had_error;
    std::vector<Scope> scopes;
    
    enum class LoopType { NONE, WHILE, FOR } current_loop;
    enum class FunctionType { NONE, FUNCTION, METHOD, INITIALIZER } current_function;
    enum class ClassType { NONE, CLASS, SUBCLASS } current_class;

    void error(Token token, std::string_view message);

    void resolve(expr::Expr* expr);
    void resolve(stmt::Stmt* stmt);
    void resolve(std::vector<std::unique_ptr<stmt::Stmt>>& stmts);

    void begin_scope(bool is_new_function);
    void end_scope() noexcept;
    
    [[nodiscard]] ValueInfo* declare(Token name);
    void declare_a_var(stmt::VarDeclareBase* stmt);
    void declare_var_list(stmt::VarDeclareListBase* stmt);
    void declare_from_class(stmt::Class* stmt);
    void define(Token name);
    [[nodiscard]] VarDeclareAt resolve_local(Token name);
    void resolve_function(stmt::Function* function, FunctionType type);

    void visit_binary_expr(gsl::not_null<expr::Binary*> expr) final;
    void visit_grouping_expr(gsl::not_null<expr::Grouping*> expr) final;
    void visit_tuple_expr(gsl::not_null<expr::Tuple*> expr) final;
    void visit_literal_expr(gsl::not_null<expr::Literal*> expr) noexcept final;
    void visit_unary_expr(gsl::not_null<expr::Unary*> expr) final;
    void visit_variable_expr(gsl::not_null<expr::Variable*> expr) final;
    void visit_assign_expr(gsl::not_null<expr::Assign*> expr) final;
    void visit_logical_expr(gsl::not_null<expr::Logical*> expr) final;
    void visit_call_expr(gsl::not_null<expr::Call*> expr) final;
    void visit_get_expr(gsl::not_null<expr::Get*> expr) final;
    void visit_set_expr(gsl::not_null<expr::Set*> expr) final;
    void visit_this_expr(gsl::not_null<expr::This*> expr) final;
    void visit_super_expr(gsl::not_null<expr::Super*> expr) final;

    void visit_expression_stmt(gsl::not_null<stmt::Expression*> stmt) final;
    void visit_var_stmt(gsl::not_null<stmt::Var*> stmt) final;
    void visit_block_stmt(gsl::not_null<stmt::Block*> stmt) final;
    void visit_if_stmt(gsl::not_null<stmt::If*> stmt) final;
    void visit_while_stmt(gsl::not_null<stmt::While*> stmt) final;
    void visit_function_stmt(gsl::not_null<stmt::Function*> stmt) final;
    void visit_return_stmt(gsl::not_null<stmt::Return*> stmt) final;
    void visit_break_stmt(gsl::not_null<stmt::Break*> stmt) final;
    void visit_continue_stmt(gsl::not_null<stmt::Continue*> stmt) final;
    void visit_class_stmt(gsl::not_null<stmt::Class*> stmt) final;
    void visit_for_stmt(gsl::not_null<stmt::For*> stmt) final;
    void visit_import_stmt(gsl::not_null<stmt::Import*> stmt) final;
    void visit_from_stmt(gsl::not_null<stmt::From*> stmt) final;
    void visit_export_stmt(gsl::not_null<stmt::Export*> stmt) final;
  };
}
