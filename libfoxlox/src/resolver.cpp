#include <gsl/gsl>
#include <fmt/format.h>

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
    for (auto& stmt : ast)
    {
      resolve_stmt(stmt.get());
    }
    return std::move(ast);
  }
  void Resolver::resolve_expr(const expr::Expr* expr)
  {
    expr::IVisitor<void>::visit(expr);
  }
  void Resolver::resolve_stmt(const stmt::Stmt* stmt)
  {
    stmt::IVisitor<void>::visit(stmt);
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
    ValueInfo* vinfo = declare(stmt->name);
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
      stmt->store_type = stmt::VarStoreType::Stack;
    }
  }
  void Resolver::define(Token name)
  {
    scopes.back().vars[name.lexeme].is_ready = true;
  }
  VarDeclareAt Resolver::resolve_local(Token name)
  {
    const auto current_function_level = scopes.back().function_level;
    for (gsl::index i = 0; i < ssize(scopes); i++)
    {
      auto& this_scope = scopes[ssize(scopes) - 1 - i];
      auto found = this_scope.vars.find(name.lexeme);
      if (found != this_scope.vars.end())
      {
        if (current_function_level != this_scope.function_level)
        {
          // access a var from inside of a nested function
          // move the var from stack to closure value pool
          if (std::holds_alternative<stmt::Var*>(found->second.declare))
          {
            std::get<stmt::Var*>(found->second.declare)->store_type = stmt::VarStoreType::Closure;
          }
          else if (std::holds_alternative<stmt::Class*>(found->second.declare))
          {
            std::get<stmt::Class*>(found->second.declare)->store_type = stmt::VarStoreType::Closure;
          }
          else if (std::holds_alternative<VarDeclareAtFunc>(found->second.declare))
          {
            auto at = std::get<VarDeclareAtFunc>(found->second.declare);
            at.func->param_store_types[at.param_index] = stmt::VarStoreType::Closure;
          }
          assert(false);
        }
        return found->second.declare;
      }
    }
    error(name, fmt::format("Can't find variable with name: `{}'.", name.lexeme));
    return {};
  }
}