#include "IR/index.h"
#include "Util/index.h"

void Function::Init(Napi::Env env, Napi::Object &exports) {
    Napi::HandleScope scope(env);
    const Napi::Function func = DefineClass(env, "Function", {
                                                StaticMethod("Create", &Function::Create),
                                                StaticMethod("isTargetIntrinsic", &Function::isTargetIntrinsic_static),
                                                InstanceMethod("arg_size", &Function::argSize),
                                                InstanceMethod("getArg", &Function::getArg),
                                                InstanceMethod("getReturnType", &Function::getReturnType),
                                                InstanceMethod("addBasicBlock", &Function::addBasicBlock),
                                                InstanceMethod("getEntryBlock", &Function::getEntryBlock),
                                                InstanceMethod("getExitBlock", &Function::getExitBlock),
                                                InstanceMethod("insertAfter", &Function::insertAfter),
                                                InstanceMethod("deleteBody", &Function::deleteBody),
                                                InstanceMethod("removeFromParent", &Function::removeFromParent),
                                                InstanceMethod("eraseFromParent", &Function::eraseFromParent),
                                                InstanceMethod("use_empty", &Function::useEmpty),
                                                InstanceMethod("user_empty", &Function::userEmpty),
                                                InstanceMethod("getNumUses", &Function::getNumUses),
                                                InstanceMethod("removeDeadConstantUsers", &Function::removeDeadConstantUsers),
                                                InstanceMethod("hasPersonalityFn", &Function::hasPersonalityFn),
                                                InstanceMethod("setPersonalityFn", &Function::setPersonalityFn),
                                                InstanceMethod("setDoesNotThrow", &Function::setDoesNotThrow),
                                                InstanceMethod("setSubprogram", &Function::setSubprogram),
                                                InstanceMethod("getSubprogram", &Function::getSubprogram),
                                                InstanceMethod("getType", &Function::getType),
                                                InstanceMethod("addFnAttr", &Function::addFnAttr),
                                                InstanceMethod("addParamAttr", &Function::addParamAttr),
                                                InstanceMethod("addRetAttr", &Function::addRetAttr),
                                                InstanceMethod("hasLazyArguments", &Function::hasLazyArguments),
                                                InstanceMethod("isMaterializable", &Function::isMaterializable),
                                                InstanceMethod("setIsMaterializable", &Function::setIsMaterializable),
                                                InstanceMethod("getIntrinsicID", &Function::getIntrinsicID),
                                                InstanceMethod("isIntrinsic", &Function::isIntrinsic),
                                                InstanceMethod("isTargetIntrinsic", &Function::isTargetIntrinsic),
                                                InstanceMethod("isConstrainedFPIntrinsic", &Function::isConstrainedFPIntrinsic),
                                            });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    Inherit(env, constructor.Value(), GlobalObject::constructor.Value());
    exports.Set("Function", func);
}

Napi::Object Function::New(Napi::Env env, llvm::Function *function) {
    return constructor.New({Napi::External<llvm::Function>::New(env, function)});
}

bool Function::IsClassOf(const Napi::Value &value) {
    return value.IsNull() || value.As<Napi::Object>().InstanceOf(constructor.Value());
}

llvm::Function *Function::Extract(const Napi::Value &value) {
    if (value.IsNull()) {
        return nullptr;
    }
    return Unwrap(value.As<Napi::Object>())->getLLVMPrimitive();
}

Function::Function(const Napi::CallbackInfo &info) : ObjectWrap(info) {
    const Napi::Env env = info.Env();
    if (!info.IsConstructCall() || info.Length() == 0 || !info[0].IsExternal()) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Function::Create);
    }
    const auto external = info[0].As<Napi::External<llvm::Function> >();
    function = external.Data();
}

Napi::Value Function::Create(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    const unsigned argsLen = info.Length();
    if (argsLen < 2 ||
        !FunctionType::IsClassOf(info[0]) ||
        !info[1].IsNumber() ||
        argsLen >= 3 && !info[2].IsString() ||
        argsLen >= 4 && !Module::IsClassOf(info[3])) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Function::Create);
    }
    llvm::FunctionType *funcType = FunctionType::Extract(info[0]);
    const llvm::GlobalValue::LinkageTypes linkage = static_cast<llvm::GlobalValue::LinkageTypes>(info[1].As<
        Napi::Number>().Uint32Value());
    std::string name;
    llvm::Module *module = nullptr;
    if (argsLen >= 3) {
        name = info[2].As<Napi::String>();
    }
    if (argsLen >= 4) {
        module = Module::Extract(info[3]);
    }
    llvm::Function *function = llvm::Function::Create(funcType, linkage, static_cast<unsigned>(-1), name, module);
    return New(env, function);
}

llvm::Function *Function::getLLVMPrimitive() {
    return function;
}

Napi::Value Function::hasLazyArguments(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() != 0) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Function::hasLazyArguments);
    }
    return Napi::Boolean::New(info.Env(), this->function->hasLazyArguments());
}

Napi::Value Function::isMaterializable(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() != 0) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Function::isMaterializable);
    }
    return Napi::Boolean::New(info.Env(), this->function->isMaterializable());
}

void Function::setIsMaterializable(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() != 1 || !info[0].IsBoolean()) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Function::setIsMaterializable);
    }
    const bool value = info[0].As<Napi::Boolean>().Value();
    this->function->setIsMaterializable(value);
}

Napi::Value Function::getIntrinsicID(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() != 0) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Function::getIntrinsicID);
    }
    return Napi::Number::New(env, function->getIntrinsicID());
}

Napi::Value Function::isIntrinsic(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() != 0) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Function::isIntrinsic);
    }
    return Napi::Boolean::New(env, function->isIntrinsic());
}

Napi::Value Function::isTargetIntrinsic_static(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() != 1 && !info[0].IsNumber()) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Function::isTargetIntrinsic_static);
    }
    const llvm::Intrinsic::ID id = info[0].As<Napi::Number>().Uint32Value();
    return Napi::Boolean::New(env, llvm::Function::isTargetIntrinsic(id));
}

Napi::Value Function::isTargetIntrinsic(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() != 0) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Function::isTargetIntrinsic);
    }
    return Napi::Boolean::New(env, function->isTargetIntrinsic());
}

Napi::Value Function::isConstrainedFPIntrinsic(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() != 0) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Function::isConstrainedFPIntrinsic);
    }
    return Napi::Boolean::New(env, function->isConstrainedFPIntrinsic());
}

Napi::Value Function::argSize(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    return Napi::Number::New(env, static_cast<double>(function->arg_size()));
}

Napi::Value Function::getArg(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() >= 1 && info[0].IsNumber()) {
        llvm::Argument *arg = function->getArg(info[0].As<Napi::Number>());
        return Argument::New(env, arg);
    }
    throw Napi::TypeError::New(env, ErrMsg::Class::Function::getArg);
}

Napi::Value Function::getReturnType(const Napi::CallbackInfo &info) {
    return Type::New(info.Env(), function->getReturnType());
}

void Function::addBasicBlock(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() == 0 || !BasicBlock::IsClassOf(info[0])) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Function::addBasicBlock);
    }
    llvm::BasicBlock *basicBlock = BasicBlock::Extract(info[0]);
    function->getBasicBlockList().push_back(basicBlock);
}

Napi::Value Function::getEntryBlock(const Napi::CallbackInfo &info) {
    return BasicBlock::New(info.Env(), &(function->getEntryBlock()));
}

Napi::Value Function::getExitBlock(const Napi::CallbackInfo &info) {
    return BasicBlock::New(info.Env(), &(function->back()));
}

void Function::insertAfter(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() < 2 || !BasicBlock::IsClassOf(info[0]) || !BasicBlock::IsClassOf(info[1])) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Function::insertAfter);
    }
    llvm::BasicBlock *where = BasicBlock::Extract(info[0]);
    llvm::BasicBlock *bb = BasicBlock::Extract(info[1]);
    function->getBasicBlockList().insertAfter(where->getIterator(), bb);
}

void Function::deleteBody(const Napi::CallbackInfo &info) {
    function->deleteBody();
}

void Function::removeFromParent(const Napi::CallbackInfo &info) {
    function->removeFromParent();
}

void Function::eraseFromParent(const Napi::CallbackInfo &info) {
    function->eraseFromParent();
}

Napi::Value Function::useEmpty(const Napi::CallbackInfo &info) {
    return Napi::Boolean::New(info.Env(), function->use_empty());
}

Napi::Value Function::userEmpty(const Napi::CallbackInfo &info) {
    return Napi::Boolean::New(info.Env(), function->user_empty());
}

Napi::Value Function::getNumUses(const Napi::CallbackInfo &info) {
    return Napi::Number::New(info.Env(), function->getNumUses());
}

void Function::removeDeadConstantUsers(const Napi::CallbackInfo &info) {
    function->removeDeadConstantUsers();
}

Napi::Value Function::hasPersonalityFn(const Napi::CallbackInfo &info) {
    return Napi::Boolean::New(info.Env(), function->hasPersonalityFn());
}

void Function::setPersonalityFn(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() == 1 && Constant::IsClassOf(info[0])) {
        llvm::Constant *fn = Constant::Extract(info[0]);
        function->setPersonalityFn(fn);
        return;
    }
    throw Napi::TypeError::New(env, ErrMsg::Class::Function::setPersonalityFn);
}

void Function::setDoesNotThrow(const Napi::CallbackInfo &info) {
    function->setDoesNotThrow();
}

void Function::setSubprogram(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() == 1 && DISubprogram::IsClassOf(info[0])) {
        llvm::DISubprogram *subprogram = DISubprogram::Extract(info[0]);
        function->setSubprogram(subprogram);
        return;
    }
    throw Napi::TypeError::New(env, ErrMsg::Class::Function::setSubprogram);
}

Napi::Value Function::getSubprogram(const Napi::CallbackInfo &info) {
    return DISubprogram::New(info.Env(), function->getSubprogram());
}

Napi::Value Function::getType(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    llvm::PointerType *type = function->getType();
    return PointerType::New(env, type);
}

void Function::addFnAttr(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();

    const unsigned argsLen = info.Length();

    if (argsLen >= 1 && argsLen <= 2) {
        if (argsLen == 1 && info[0].IsNumber()) {
            unsigned rawAttrKind = info[0].As<Napi::Number>();
            if (rawAttrKind < llvm::Attribute::AttrKind::FirstEnumAttr ||
                rawAttrKind > llvm::Attribute::AttrKind::LastEnumAttr) {
                throw Napi::TypeError::New(env, ErrMsg::Class::Attribute::invalidAttrKind);
            }
            function->addFnAttr(static_cast<llvm::Attribute::AttrKind>(rawAttrKind));
            return;
        } else if (argsLen == 1 && Attribute::IsClassOf(info[0])) {
            const llvm::Attribute attr = Attribute::Extract(info[0]);
            function->addFnAttr(attr);
            return;
        } else if (info[0].IsString()) {
            const std::string attrKind = info[0].As<Napi::String>();
            if (argsLen == 1) {
                function->addFnAttr(attrKind);
                return;
            } else if (argsLen == 2 && info[1].IsString()) {
                const std::string value = info[1].As<Napi::String>();
                function->addFnAttr(attrKind, value);
                return;
            }
        }
    }

    throw Napi::TypeError::New(env, ErrMsg::Class::Function::addFnAttr);
}

void Function::addParamAttr(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();

    if (info.Length() == 2 && info[0].IsNumber()) {
        const unsigned argNo = info[0].As<Napi::Number>();

        if (info[1].IsNumber()) {
            unsigned rawAttrKind = info[1].As<Napi::Number>();
            if (rawAttrKind < llvm::Attribute::AttrKind::FirstEnumAttr ||
                rawAttrKind > llvm::Attribute::AttrKind::LastEnumAttr) {
                throw Napi::TypeError::New(env, ErrMsg::Class::Attribute::invalidAttrKind);
            }
            function->addParamAttr(argNo, static_cast<llvm::Attribute::AttrKind>(rawAttrKind));
            return;
        } else if (Attribute::IsClassOf(info[1])) {
            const llvm::Attribute attr = Attribute::Extract(info[1]);
            function->addParamAttr(argNo, attr);
            return;
        }
    }

    throw Napi::TypeError::New(env, ErrMsg::Class::Function::addParamAttr);
}

void Function::addRetAttr(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();

    if (info.Length() == 1) {
        if (info[0].IsNumber()) {
            unsigned rawAttrKind = info[0].As<Napi::Number>();
            if (rawAttrKind < llvm::Attribute::AttrKind::FirstEnumAttr ||
                rawAttrKind > llvm::Attribute::AttrKind::LastEnumAttr) {
                throw Napi::TypeError::New(env, ErrMsg::Class::Attribute::invalidAttrKind);
            }
            function->addRetAttr(static_cast<llvm::Attribute::AttrKind>(rawAttrKind));
            return;
        } else if (Attribute::IsClassOf(info[0])) {
            const llvm::Attribute attr = Attribute::Extract(info[0]);
            function->addRetAttr(attr);
            return;
        }
    }

    throw Napi::TypeError::New(env, ErrMsg::Class::Function::addRetAttr);
}
