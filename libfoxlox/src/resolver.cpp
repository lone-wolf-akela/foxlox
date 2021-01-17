#include <gsl/gsl>
#include <fmt/format.h>
#include <range/v3/all.hpp>

#include "common.h"

#include "resolver.h"

namespace foxlox
{
  Resolver::Resolver(AST&& a) : ast(std::move(a))
  {
    current_function = FunctionType::NONE;
    current_loop = LoopType::NONE;
    current_class = ClassType::NONE;

    had_error = false;
  }
  AST Resolver::resolve()
  {
    // global is also a scope
    begin_scope(true);
    resolve(ast);
    end_scope();
    return std::move(ast);
  }
  bool Resolver::get_had_error()
  {
    return had_error;
  }
  void Resolver::error(Token token, std::string_view message)
  {
    format_error(token, message);
    had_error = true;
  }
  void Resolver::resolve(expr::Expr* expr)
  {
    expr::IVisitor<void>::visit(expr);
  }
  void Resolver::resolve(stmt::Stmt* stmt)
  {
    stmt::IVisitor<void>::visit(stmt);
  }
  void Resolver::resolve(std::vector<std::unique_ptr<stmt::Stmt>>& stmts)
  {
    for (auto& stmt : stmts)
    {
      resolve(stmt.get());
    }
  }
  void Resolver::begin_scope(bool is_new_function)
  {
    int last_func_level = scopes.empty() ? 0 : scopes.back().function_level;
    scopes.emplace_back();
    scopes.back().function_level = is_new_function ? last_func_level + 1 : last_func_level;
  }
  void Resolver::end_scope()
  {
    scopes.pop_back();
  }
  [[nodiscard]] ValueInfo* Resolver::declare(Token name)
  {
    auto& scope = scopes.back();
    if (scope.vars.contains(name.lexeme))
    {
      error(name, "Redefine variable with the same name in this scope.");
      return nullptr;
    }
    scope.vars[name.lexeme] = ValueInfo{ .is_ready = false };
    return &scope.vars[name.lexeme];
  }
  void Resolver::declare_from_varstmt(stmt::Var* stmt)
  {
    ValueInfo* vinfo = declare(stmt->name);
    if (vinfo != nullptr)
    {
      vinfo->declare = stmt;
      stmt->store_type = stmt::VarStoreType::Stack;
    }
  }
  void Resolver::declare_from_functionparam(stmt::Function* stmt, int param_index)
  {
    ValueInfo* vinfo = declare(stmt->param[param_index]);
    if (vinfo != nullptr)
    {
      vinfo->declare = VarDeclareAtFunc{ .func = stmt, .param_index = param_index };
      if (stmt->param_store_types.size() <= param_index)
      {
        stmt->param_store_types.resize(param_index + 1);
        stmt->param_store_types[param_index] = stmt::VarStoreType::Stack;
      }
    }
  }
  void Resolver::declare_from_class(stmt::Class* stmt)
  {
    ValueInfo* vinfo = declare(stmt->name);
    if (vinfo != nullptr)
    {
      vinfo->declare = stmt;
      stmt->name_store_type = stmt::VarStoreType::Stack;
      stmt->this_store_type = stmt::VarStoreType::Stack;
    }
  }
  void Resolver::declare_from_functionname(stmt::Function* stmt)
  {
    ValueInfo* vinfo = declare(stmt->name);
    if (vinfo != nullptr)
    {
      vinfo->declare = stmt;
      stmt->name_store_type = stmt::VarStoreType::Stack;
    }
  }
  void Resolver::define(Token name)
  {
    scopes.back().vars[name.lexeme].is_ready = true;
  }
  [[nodiscard]] VarDeclareAt Resolver::resolve_local(Token name)
  {
    const auto current_function_level = scopes.back().function_level;
    for (gsl::index i = 0; i < ssize(scopes); i++)
    {
      auto& this_scope = scopes[ssize(scopes) - 1 - i];
      auto found = this_scope.vars.find(name.lexeme);
      if (found != this_scope.vars.end())
      {
        if (!found->second.is_ready)
        {
          error(name, "Can't read local variable in its own initializer.");
          return{};
        }
        if (current_function_level != this_scope.function_level)
        {
          // access a var from inside of a nested function
          // move the var from stack to closure value pool
          if (std::holds_alternative<stmt::Var*>(found->second.declare))
          {
            std::get<stmt::Var*>(found->second.declare)->store_type = stmt::VarStoreType::Static;
          }
          else if (std::holds_alternative<stmt::Class*>(found->second.declare))
          {
            std::get<stmt::Class*>(found->second.declare)->name_store_type = stmt::VarStoreType::Static;
          }
          else if (std::holds_alternative<stmt::Function*>(found->second.declare))
          {
            std::get<stmt::Function*>(found->second.declare)->name_store_type = stmt::VarStoreType::Static;
          }
          else if (std::holds_alternative<VarDeclareAtFunc>(found->second.declare))
          {
            auto at = std::get<VarDeclareAtFunc>(found->second.declare);
            at.func->param_store_types[at.param_index] = stmt::VarStoreType::Static;
          }
          else if (std::holds_alternative<VarDeclareAtClass>(found->second.declare))
          {
            if (current_function_level - this_scope.function_level >= 2)
            {
              error(name, "Capturing `this' in non-method function is not allowed.");
              return{};
            }
          }
          else
          {
            assert(false);
          }
        }
        return found->second.declare;
      }
    }
    error(name, fmt::format("Can't find variable with name: `{}'.", name.lexeme));
    return {};
  }
  void Resolver::resolve_function(stmt::Function* function, FunctionType type)
  {
    const auto enclosing_func = current_function;
    current_function = type;

    begin_scope(true);

    for (auto [index, param] : function->param | ranges::views::enumerate)
    {
      declare_from_functionparam(function, gsl::narrow_cast<int>(index));
      define(param);
    }
    resolve(function->body);
    end_scope();

    current_function = enclosing_func;
  }
  void Resolver::visit_binary_expr(gsl::not_null<expr::Binary*> expr)
  {
    resolve(expr->left.get());
    resolve(expr->right.get());
  }
  void Resolver::visit_grouping_expr(gsl::not_null<expr::Grouping*> expr)
  {
    resolve(expr->expression.get());
  }
  void Resolver::visit_literal_expr(gsl::not_null<expr::Literal*> /*expr*/)
  {
    // do nothing
    return;
  }
  void Resolver::visit_unary_expr(gsl::not_null<expr::Unary*> expr)
  {
    resolve(expr->right.get());
  }
  void Resolver::visit_variable_expr(gsl::not_null<expr::Variable*> expr)
  {
    expr->declare = resolve_local(expr->name);
  }
  void Resolver::visit_assign_expr(gsl::not_null<expr::Assign*> expr)
  {
    resolve(expr->value.get());
    expr->declare = resolve_local(expr->name);
  }
  void Resolver::visit_logical_expr(gsl::not_null<expr::Logical*> expr)
  {
    resolve(expr->left.get());
    resolve(expr->right.get());
  }
  void Resolver::visit_call_expr(gsl::not_null<expr::Call*> expr)
  {
    resolve(expr->callee.get());
    for (auto& arg : expr->arguments)
    {
      resolve(arg.get());
    }
  }
  void Resolver::visit_get_expr(gsl::not_null<expr::Get*> expr)
  {
    // note: access from keyword `super' is not in get_expr but in super_expr
    // so we do not check them here
    if (expr->name.lexeme == "__init__")
    {
      error(expr->name, "Explicit call on constructor is not allowed (unless after `super').");
    }
    else if (dynamic_cast<expr::This*>(expr->obj.get()) == nullptr
      && expr->name.lexeme.starts_with("_"))
    {
      error(expr->name, "Can't access private members on instance other than `this' or `super'.");
    }
    resolve(expr->obj.get());
  }
  void Resolver::visit_set_expr(gsl::not_null<expr::Set*> expr)
  {
    if (dynamic_cast<expr::This*>(expr->obj.get()) == nullptr 
      && expr->name.lexeme.starts_with("_"))
    {
      error(expr->name, "Can't access private members on instance other than `this'.");
    }
    resolve(expr->value.get());
    resolve(expr->obj.get());
  }
  void Resolver::visit_this_expr(gsl::not_null<expr::This*> expr)
  {
    if (current_class == ClassType::NONE)
    {
      error(expr->keyword, "Can't use `this' outside of a class.");
      return;
    }
    expr->declare = resolve_local(expr->keyword);
  }
  void Resolver::visit_super_expr(gsl::not_null<expr::Super*> expr)
  {
    if (current_class == ClassType::NONE)
    {
      error(expr->keyword, "Can't use `super' outside of a class.");
    }
    else if (current_class != ClassType::SUBCLASS)
    {
      error(expr->keyword, "Can't use `super' in a class with no superclass.");
    }
    expr->declare = resolve_local(expr->keyword);
  }
  void Resolver::visit_expression_stmt(gsl::not_null<stmt::Expression*> stmt)
  {
    resolve(stmt->expression.get());
  }
  void Resolver::visit_var_stmt(gsl::not_null<stmt::Var*> stmt)
  {
    declare_from_varstmt(stmt);
    if (stmt->initializer.get() != nullptr)
    {
      resolve(stmt->initializer.get());
    }
    define(stmt->name);
  }
  void Resolver::visit_block_stmt(gsl::not_null<stmt::Block*> stmt)
  {
    begin_scope(false);
    resolve(stmt->statements);
    end_scope();
  }
  void Resolver::visit_if_stmt(gsl::not_null<stmt::If*> stmt)
  {
    resolve(stmt->condition.get());
    if (const auto p = dynamic_cast<stmt::Var*>(stmt->then_branch.get()); p != nullptr)
    {
      error(p->name, "Conditioned variable declaration is not allowed.");
    }
    resolve(stmt->then_branch.get());
    if (stmt->else_branch.get() != nullptr)
    {
      if (const auto p = dynamic_cast<stmt::Var*>(stmt->then_branch.get()); p != nullptr)
      {
        error(p->name, "Conditioned variable declaration is not allowed.");
      }
      resolve(stmt->else_branch.get());
    }
  }
  void Resolver::visit_while_stmt(gsl::not_null<stmt::While*> stmt)
  {
    resolve(stmt->condition.get());

    const auto enclosing_loop = current_loop;
    current_loop = LoopType::WHILE;
    if (const auto p = dynamic_cast<stmt::Var*>(stmt->body.get()); p != nullptr)
    {
      error(p->name, "Conditioned variable declaration is not allowed.");
    }
    resolve(stmt->body.get());
    current_loop = enclosing_loop;
  }
  void Resolver::visit_function_stmt(gsl::not_null<stmt::Function*> stmt)
  {
    declare_from_functionname(stmt);
    define(stmt->name);
    resolve_function(stmt, FunctionType::FUNCTION);
  }
  void Resolver::visit_return_stmt(gsl::not_null<stmt::Return*> stmt)
  {
    if (stmt->value.get() != nullptr)
    {
      if (current_function == FunctionType::INITIALIZER)
      {
        error(stmt->keyword, "Can't return a value from an class initializer.");
      }
      resolve(stmt->value.get());
    }
    else if(current_function == FunctionType::INITIALIZER)
    {
      // make the initializer return `this'
      stmt->value = std::make_unique<expr::This>(
        Token(TokenType::THIS, "this", {}, stmt->keyword.line));
      resolve(stmt->value.get());
    }      
  }
  void Resolver::visit_break_stmt(gsl::not_null<stmt::Break*> stmt)
  {
    if (current_loop == LoopType::NONE)
    {
      error(stmt->keyword, "Can't use `break' outside of a loop body.");
    }
  }
  void Resolver::visit_continue_stmt(gsl::not_null<stmt::Continue*> stmt)
  {
    if (current_loop == LoopType::NONE)
    {
      error(stmt->keyword, "Can't use `continue' outside of a loop body.");
    }
  }
  void Resolver::visit_class_stmt(gsl::not_null<stmt::Class*> stmt)
  {
    const auto enclosing_class = current_class;
    current_class = ClassType::CLASS;

    declare_from_class(stmt);
    define(stmt->name);

    if (stmt->superclass.get() != nullptr)
    {
      resolve(stmt->superclass.get());
      current_class = ClassType::SUBCLASS;
    }

    begin_scope(true);
    if (current_class == ClassType::CLASS || current_class == ClassType::SUBCLASS)
    {
      scopes.back().vars["this"] = ValueInfo{ .is_ready = true, .declare = VarDeclareAtClass{stmt} };
    }
    if (current_class == ClassType::SUBCLASS)
    {
      scopes.back().vars["super"] = ValueInfo{ .is_ready = true, .declare = VarDeclareAtClass{stmt} };
    }
    for (auto& method : stmt->methods)
    {
      FunctionType declaration = FunctionType::METHOD;
      if (method->name.lexeme == "__init__")
      {
        declaration = FunctionType::INITIALIZER;
      }
      resolve_function(method.get(), declaration);
    }
    end_scope();

    current_class = enclosing_class;
  }
  void Resolver::visit_for_stmt(gsl::not_null<stmt::For*> stmt)
  {
    begin_scope(false);

    resolve(stmt->initializer.get());
    resolve(stmt->condition.get());
    resolve(stmt->increment.get());

    LoopType enclosingt_loop = current_loop;
    current_loop = LoopType::FOR;
    if (const auto p = dynamic_cast<stmt::Var*>(stmt->body.get()); p != nullptr)
    {
      error(p->name, "Conditioned variable declaration is not allowed.");
    }
    resolve(stmt->body.get());
    current_loop = enclosingt_loop;

    end_scope();
  }
  void Resolver::visit_tuple_expr(gsl::not_null<expr::Tuple*> expr)
  {
    for (auto& e : expr->exprs)
    {
      resolve(e.get());
    }
  }
}