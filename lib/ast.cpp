#include "ast.hpp"
#include "lisp.hpp"
#include <iostream>


namespace lisp {
namespace ast {

ObjectPtr Import::execute(Environment& env)
{
    return env.getNull();
}


ObjectPtr Integer::execute(Environment& env)
{
    return env.loadI(cachedVal_);
}


ObjectPtr Double::execute(Environment& env)
{
    return env.loadI(cachedVal_);
}


ObjectPtr String::execute(Environment& env)
{
    return env.loadI(cachedVal_);
}


ObjectPtr Null::execute(Environment& env)
{
    return env.getNull();
}


ObjectPtr True::execute(Environment& env)
{
    return env.getBool(true);
}


ObjectPtr False::execute(Environment& env)
{
    return env.getBool(false);
}


ObjectPtr LValue::execute(Environment& env)
{
    return env.load(cachedVarLoc_);
}


class InterpretedFunctionImpl : public Function::Impl {
public:
    InterpretedFunctionImpl(ast::Lambda* impl) : impl_(impl)
    {
    }


    ObjectPtr call(Environment& env, Arguments& args)
    {
        auto derived = env.derive();
        for (size_t i = 0; i < args.size(); ++i) {
            derived->push(args[i]);
        }
        auto up = env.getNull();
        for (auto& statement : impl_->statements_) {
            up = statement->execute(*derived);
        }
        return up;
    }


private:
    const ast::Lambda* impl_;
};


ObjectPtr Lambda::execute(Environment& env)
{
    auto impl = make_unique<InterpretedFunctionImpl>(this);
    const auto argc = argNames_.size();
    return env.create<lisp::Function>(
        [&]() -> const char* {
            if (not docstring_.empty()) {
                return docstring_.c_str();
            } else {
                return nullptr;
            }
        }(),
        argc, std::move(impl));
}


ObjectPtr Application::execute(Environment& env)
{
    auto loaded = checkedCast<lisp::Function>(toApply_->execute(env));
    Arguments args;
    for (const auto& arg : args_) {
        args.push_back(arg->execute(env));
    }
    return loaded->call(args);
}


ObjectPtr Let::Binding::execute(Environment& env)
{
    env.push(value_->execute(env));
    return env.getNull();
}


ObjectPtr Let::execute(Environment& env)
{
    auto derived = env.derive();
    for (const auto& binding : bindings_) {
        binding->execute(*derived);
    }
    ObjectPtr up = env.getNull();
    for (const auto& st : statements_) {
        up = st->execute(*derived);
    }
    return up;
}


ObjectPtr Begin::execute(Environment& env)
{
    auto up = env.getNull();
    for (const auto& st : statements_) {
        up = st->execute(env);
    }
    return up;
}


ObjectPtr If::execute(Environment& env)
{
    auto cond = condition_->execute(env);
    if (cond == env.getBool(true)) {
        return trueBranch_->execute(env);
    } else if (cond == env.getBool(false)) {
        return falseBranch_->execute(env);
    } else {
        throw std::runtime_error("bad if expression condition");
    }
}


ObjectPtr Or::execute(Environment& env)
{
    for (const auto& statement : statements_) {
        auto result = statement->execute(env);
        if (not(result == env.getBool(false))) {
            return result;
        }
    }
    return env.getBool(false);
}


ObjectPtr And::execute(Environment& env)
{
    ObjectPtr result = env.getBool(true);
    for (const auto& statement : statements_) {
        result = statement->execute(env);
        if (result == env.getBool(false)) {
            return env.getBool(false);
        }
    }
    return result;
}


ObjectPtr Def::execute(Environment& env)
{
    env.push(value_->execute(env));
    return env.getNull();
}


ObjectPtr Set::execute(Environment& env)
{
    env.store(cachedVarLoc_, value_->execute(env));
    return env.getNull();
}


ObjectPtr UserObject::execute(Environment& env)
{
    return value_;
}


void Import::init(Environment& env, Scope& scope)
{
    std::cout << "importing " << name_ << std::endl;
}


void String::init(Environment& env, Scope& scope)
{
    cachedVal_ =
        env.storeI(env.create<lisp::String>(value_.c_str(), value_.length()));
}


void Integer::init(Environment& env, Scope& scope)
{
    cachedVal_ = env.storeI(env.create<lisp::Integer>(value_));
}


void Double::init(Environment& env, Scope& scope)
{
    cachedVal_ = env.storeI(env.create<lisp::Double>(value_));
}


void LValue::init(Environment& env, Scope& scope)
{
    cachedVarLoc_ = scope.find(name_);
}


void Lambda::init(Environment& env, Scope& scope)
{
    Scope::setParent(&scope);
    for (const auto& argName : argNames_) {
        Scope::insert(argName);
    }
    for (const auto& statement : statements_) {
        statement->init(env, *this);
    }
}


void Application::init(Environment& env, Scope& scope)
{
    toApply_->init(env, scope);
    for (const auto& arg : args_) {
        arg->init(env, scope);
    }
}


void Let::Binding::init(Environment& env, Scope& scope)
{
    scope.insert(name_);
    value_->init(env, scope);
}


void Let::init(Environment& env, Scope& scope)
{
    Scope::setParent(&scope);
    for (const auto& binding : bindings_) {
        binding->init(env, *this);
    }
    for (const auto& statement : statements_) {
        statement->init(env, *this);
    }
}


void Begin::init(Environment& env, Scope& scope)
{
    for (const auto& statement : statements_) {
        statement->init(env, scope);
    }
}


void If::init(Environment& env, Scope& scope)
{
    condition_->init(env, scope);
    trueBranch_->init(env, scope);
    falseBranch_->init(env, scope);
}


void Or::init(Environment& env, Scope& scope)
{
    for (const auto& statement : statements_) {
        statement->init(env, scope);
    }
}


void And::init(Environment& env, Scope& scope)
{
    for (const auto& statement : statements_) {
        statement->init(env, scope);
    }
}


void Def::init(Environment& env, Scope& scope)
{
    scope.insert(name_);
    value_->init(env, scope);
}


void Set::init(Environment& env, Scope& scope)
{
    cachedVarLoc_ = scope.find(name_);
    value_->init(env, scope);
}


void TopLevel::init(Environment& env, Scope& scope)
{
    // The builtin functions wouldn't be visible to the compiler otherwise.
    const auto names = getBuiltinList();
    for (auto& name : names) {
        scope.insert(name);
    }
    Begin::init(env, scope);
}


} // namespace ast
} // namespace lisp
