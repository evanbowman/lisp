#include <functional>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>

#include "lexer.hpp"
#include "types.hpp"
#include "environment.hpp"

namespace lisp {

template <typename Proc>
void dolist(Environment& env, ObjectPtr list, Proc&& proc) {
    Heap::Ptr<Pair> current = checkedCast<Pair>(env, list);
    proc(current->getCar());
    while (not isType<Null>(env, current->getCdr())) {
        current = checkedCast<Pair>(env, current->getCdr());
        proc(current->getCar());
    }
}

static const struct BuiltinSubrInfo {
    const char* name;
    const char* docstring;
    Arguments::Count requiredArgs;
    Subr::Impl impl;
} builtins[] = {
    {"cons", "(cons CAR CDR)", 2,
     [](Environment& env, const Arguments& args) {
         return env.create<Pair>(args[0], args[1]);
     }},
    {"list", nullptr, 0,
     [](Environment& env, const Arguments& args) -> ObjectPtr {
         if (auto argc = args.count()) {
             auto tail = env.create<Pair>(args[argc - 1],
                                          env.getNull());
             Local<Pair> list(env, tail);
             for (auto pos = argc - 2; pos > -1; --pos) {
                 list = env.create<Pair>(args[pos], list.get());
             }
             return list.get();
         } else {
             return env.getNull();
         }
     }},
    {"car", nullptr, 1,
     [](Environment& env, const Arguments& args) {
         return checkedCast<Pair>(env, args[0])->getCar();
     }},
    {"cdr", nullptr, 1,
     [](Environment& env, const Arguments& args) {
         return checkedCast<Pair>(env, args[0])->getCdr();
     }},
    {"set-car!", nullptr, 2,
     [](Environment& env, const Arguments& args) {
         checkedCast<Pair>(env, args[0])->setCar(args[1]);
         return env.getNull();
     }},
    {"set-cdr!", nullptr, 2,
     [](Environment& env, const Arguments& args) {
         checkedCast<Pair>(env, args[0])->setCdr(args[1]);
         return env.getNull();
     }},
    {"null?", nullptr, 1,
     [](Environment& env, const Arguments& args) {
         return env.getBool(isType<Null>(env, args[0]));
     }},
    {"pair?", nullptr, 1,
     [](Environment& env, const Arguments& args) {
         return env.getBool(isType<Pair>(env, args[0]));
     }},
    {"bool?", nullptr, 1,
     [](Environment& env, const Arguments& args) {
         return env.getBool(isType<Boolean>(env, args[0]));
     }},
    {"eq?", nullptr, 2,
     [](Environment& env, const Arguments& args) {
         return env.getBool(args[0] == args[1]);
     }},
    {"identity", nullptr, 1,
     [](Environment& env, const Arguments& args) {
         return args[0];
     }},
    {"length", nullptr, 1,
     [](Environment& env, const Arguments& args) {
         FixNum::Rep length = 0;
         if (not isType<Null>(env, args[0])) {
             dolist(env, args[0], [&](ObjectPtr) { ++length; });
         }
         return env.create<FixNum>(length);
     }},
    {"map", nullptr, 2,
     [](Environment& env, const Arguments& args)
     -> ObjectPtr {
         std::vector<ObjectPtr> paramVec;
         if (not isType<Null>(env, args[1])) {
             auto subr = checkedCast<Subr>(env, args[0]);
             Heap::Ptr<Pair> lst = checkedCast<Pair>(env, args[1]);
             paramVec = {lst->getCar()};
             Local<Pair> result(env,
                                env.create<Pair>(subr->
                                                 call(paramVec),
                                                 env.getNull()));
             auto current = result.get();
             while (not isType<Null>(env, lst->getCdr())) {
                 lst = checkedCast<Pair>(env, lst->getCdr());
                 paramVec = {lst->getCar()};
                 auto next = env.create<Pair>(subr->call(paramVec),
                                              env.getNull());
                 current->setCdr(next);
                 current = next;
             }
             return result.get();
         }
         return env.getNull();
     }},
    {"apply", nullptr, 2,
     [](Environment& env, const Arguments& args) {
         std::vector<ObjectPtr> params;
         if (not isType<Null>(env, args[1])) {
             dolist(env, args[1], [&](ObjectPtr elem) {
                                      params.push_back(elem);
                                  });
         }
         auto subr = checkedCast<Subr>(env, args[0]);
         return subr->call(params);
     }},
    {"+", nullptr, 0,
     [](Environment& env, const Arguments& args) {
         FixNum::Rep result = 0;
         for (Arguments::Count i = 0; i < args.count(); ++i) {
             result += checkedCast<FixNum>(env, args[i])
                 ->value();
         }
         return env.create<FixNum>(result);
     }},
    {"*", nullptr, 0,
     [](Environment& env, const Arguments& args) {
         FixNum::Rep result = 1;
         for (Arguments::Count i = 0; i < args.count(); ++i) {
             result *= checkedCast<FixNum>(env, args[i])
                 ->value();
         }
         return env.create<FixNum>(result);
     }},
    {"help", nullptr, 1,
     [](Environment& env, const Arguments& args) -> ObjectPtr {
         auto subr = checkedCast<Subr>(env, args[0]);
         if (auto doc = subr->getDocstring()) {
             std::cout << doc << std::endl;
         }
         return env.getNull();
     }}
};

void Environment::store(const std::string& key, ObjectPtr value) {
    vars_.insert({key, value});
}

ObjectPtr Environment::load(const std::string& key) {
    auto found = vars_.find(key);
    if (found not_eq vars_.end()) {
        return found->second;
    }
    throw std::runtime_error("binding for \'" + key + "\' does not exist");
}

ObjectPtr Context::getNull() {
    return nullValue_.get();
}

ObjectPtr Environment::getNull() {
    return context_->getNull();
}

ObjectPtr Context::getBool(bool trueOrFalse) {
    return booleans_[trueOrFalse].get();
}

ObjectPtr Environment::getBool(bool trueOrFalse) {
    return context_->getBool(trueOrFalse);
}

EnvPtr Environment::derive() {
    throw std::runtime_error("TODO");
}

EnvPtr Environment::parent() {
    return parent_;
}

EnvPtr Environment::reference() {
    return shared_from_this();
}

Context::Context(const Configuration& config) :
    heap_(1000000),
    topLevel_(std::make_shared<Environment>(this, nullptr)),
    booleans_{{topLevel_->create<Boolean>(false)},
              {topLevel_->create<Boolean>(true)}},
    nullValue_{topLevel_->create<Null>()} {
    std::for_each(std::begin(builtins), std::end(builtins),
                 [&](const BuiltinSubrInfo& info) {
                     auto subr = topLevel_->create<Subr>(info.docstring,
                                                         info.requiredArgs,
                                                         info.impl);
                     topLevel_->store(info.name, subr);
                 });
}

std::shared_ptr<Environment> Context::topLevel() {
    return topLevel_;
}

} // namespace lisp
