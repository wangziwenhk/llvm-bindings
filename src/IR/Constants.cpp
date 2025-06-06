#include "IR/index.h"
#include "ADT/index.h"
#include "Util/index.h"

//===----------------------------------------------------------------------===//
//                        Constant Class
//===----------------------------------------------------------------------===//

void Constant::Init(Napi::Env env, Napi::Object &exports) {
    Napi::HandleScope scope(env);
    const Napi::Function func = DefineClass(env, "Constant", {
            StaticMethod("getNullValue", Constant::getNullValue),
            StaticMethod("getAllOnesValue", Constant::getAllOnesValue),
            InstanceMethod("isNullValue", &Constant::isNullValue),
            InstanceMethod("isOneValue", &Constant::isOneValue),
            InstanceMethod("isAllOnesValue", &Constant::isAllOnesValue),
            InstanceMethod("getType", &Constant::getType)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    Inherit(env, constructor.Value(), User::constructor.Value());
    exports.Set("Constant", func);
}

Napi::Object Constant::New(Napi::Env env, llvm::Constant *constant) {
    if (llvm::isa<llvm::GlobalValue>(constant)) {
        return GlobalValue::New(env, llvm::cast<llvm::GlobalValue>(constant));
    } else if (llvm::isa<llvm::ConstantInt>(constant)) {
        return ConstantInt::New(env, llvm::cast<llvm::ConstantInt>(constant));
    } else if (llvm::isa<llvm::ConstantFP>(constant)) {
        return ConstantFP::New(env, llvm::cast<llvm::ConstantFP>(constant));
    } else if (llvm::isa<llvm::ConstantArray>(constant)) {
        return ConstantArray::New(env, llvm::cast<llvm::ConstantArray>(constant));
    } else if (llvm::isa<llvm::ConstantStruct>(constant)) {
        return ConstantStruct::New(env, llvm::cast<llvm::ConstantStruct>(constant));
    } else if (llvm::isa<llvm::ConstantPointerNull>(constant)) {
        return ConstantPointerNull::New(env, llvm::cast<llvm::ConstantPointerNull>(constant));
    } else if (llvm::isa<llvm::ConstantDataArray>(constant)) {
        return ConstantDataArray::New(env, llvm::cast<llvm::ConstantDataArray>(constant));
    } else if (llvm::isa<llvm::ConstantExpr>(constant)) {
        return ConstantExpr::New(env, llvm::cast<llvm::ConstantExpr>(constant));
    } else if (llvm::isa<llvm::UndefValue>(constant)) {
        return UndefValue::New(env, llvm::cast<llvm::UndefValue>(constant));
    }
    // TODO: more structured clearly
    return constructor.New({Napi::External<llvm::Constant>::New(env, constant)});
}

bool Constant::IsClassOf(const Napi::Value &value) {
    return value.IsNull() || value.As<Napi::Object>().InstanceOf(constructor.Value());
}

llvm::Constant *Constant::Extract(const Napi::Value &value) {
    if (value.IsNull()) {
        return nullptr;
    }
    return Unwrap(value.As<Napi::Object>())->getLLVMPrimitive();
}

Constant::Constant(const Napi::CallbackInfo &info) : ObjectWrap(info) {
    const Napi::Env env = info.Env();
    if (!info.IsConstructCall() || info.Length() == 0 || !info[0].IsExternal()) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Constant::constructor);
    }
    const auto external = info[0].As<Napi::External<llvm::Constant>>();
    constant = external.Data();
}

Napi::Value Constant::getNullValue(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() == 0 || !Type::IsClassOf(info[0])) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Constant::getNullValue);
    }
    llvm::Type *type = Type::Extract(info[0]);
    llvm::Constant *constant = llvm::Constant::getNullValue(type);
    return Constant::New(env, constant);
}

Napi::Value Constant::getAllOnesValue(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() == 0 || !Type::IsClassOf(info[0])) {
        throw Napi::TypeError::New(env, ErrMsg::Class::Constant::getAllOnesValue);
    }
    llvm::Type *type = Type::Extract(info[0]);
    llvm::Constant *constant = llvm::Constant::getAllOnesValue(type);
    return Constant::New(env, constant);
}

Napi::Value Constant::isNullValue(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    return Napi::Boolean::New(env, constant->isNullValue());
}

Napi::Value Constant::isOneValue(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    return Napi::Boolean::New(env, constant->isOneValue());
}

Napi::Value Constant::isAllOnesValue(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    return Napi::Boolean::New(env, constant->isAllOnesValue());
}

Napi::Value Constant::getType(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    llvm::Type *type = constant->getType();
    return Type::New(env, type);
}

llvm::Constant *Constant::getLLVMPrimitive() {
    return constant;
}

//===----------------------------------------------------------------------===//
//                        ConstantInt Class
//===----------------------------------------------------------------------===//

typedef llvm::ConstantInt *(GetBool)(llvm::LLVMContext &);

template<GetBool method>
Napi::Value getBoolFactory(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() == 0 || !LLVMContext::IsClassOf(info[0])) {
        throw Napi::TypeError::New(env, "ConstantInt.getTrue/getFalse needs to be called with: (context: LLVMContext)");
    }
    llvm::LLVMContext &context = LLVMContext::Extract(info[0]);
    llvm::ConstantInt *constantInt = method(context);
    return ConstantInt::New(env, constantInt);
}

void ConstantInt::Init(Napi::Env env, Napi::Object &exports) {
    Napi::HandleScope scope(env);
    const Napi::Function func = DefineClass(env, "ConstantInt", {
            StaticMethod("get", &ConstantInt::get),
            StaticMethod("getTrue", &getBoolFactory<llvm::ConstantInt::getTrue>),
            StaticMethod("getFalse", &getBoolFactory<llvm::ConstantInt::getFalse>),
            InstanceMethod("getType", &ConstantInt::getType)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    Inherit(env, constructor.Value(), Constant::constructor.Value());
    exports.Set("ConstantInt", func);
}

Napi::Object ConstantInt::New(Napi::Env env, llvm::ConstantInt *constantInt) {
    return constructor.New({Napi::External<llvm::ConstantInt>::New(env, constantInt)});
}

bool ConstantInt::IsClassOf(const Napi::Value &value) {
    return value.IsNull() || value.As<Napi::Object>().InstanceOf(constructor.Value());
}

llvm::ConstantInt *ConstantInt::Extract(const Napi::Value &value) {
    if (value.IsNull()) {
        return nullptr;
    }
    return Unwrap(value.As<Napi::Object>())->getLLVMPrimitive();
}

ConstantInt::ConstantInt(const Napi::CallbackInfo &info) : ObjectWrap(info) {
    const Napi::Env env = info.Env();
    if (!info.IsConstructCall() || info.Length() == 0 || !info[0].IsExternal()) {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantInt::constructor);
    }
    const auto external = info[0].As<Napi::External<llvm::ConstantInt>>();
    constantInt = external.Data();
}

llvm::ConstantInt *ConstantInt::getLLVMPrimitive() {
    return constantInt;
}

Napi::Value ConstantInt::get(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    const unsigned argsLen = info.Length();
    if (argsLen >= 2 && LLVMContext::IsClassOf(info[0]) && APInt::IsClassOf(info[1])) {
        llvm::LLVMContext &context = LLVMContext::Extract(info[0]);
        const llvm::APInt &value = APInt::Extract(info[1]);
        return ConstantInt::New(env, llvm::ConstantInt::get(context, value));
    } else if (argsLen >= 2 && IntegerType::IsClassOf(info[0]) && info[1].IsNumber()) {
        llvm::IntegerType *intType = IntegerType::Extract(info[0]);
        const uint64_t value = info[1].As<Napi::Number>().Int64Value();
        const bool isSigned = argsLen >= 3 && info[2].As<Napi::Boolean>();
        return ConstantInt::New(env, llvm::ConstantInt::get(intType, value, isSigned));
    } else if (argsLen >= 2 && Type::IsClassOf(info[0])) {
        llvm::Type *type = Type::Extract(info[0]);
        if (APInt::IsClassOf(info[1])) {
            const llvm::APInt &value = APInt::Extract(info[1]);
            return Constant::New(env, llvm::ConstantInt::get(type, value));
        } else if (info[1].IsNumber()) {
            const uint64_t value = info[1].As<Napi::Number>().Int64Value();
            const bool isSigned = argsLen >= 3 && info[2].As<Napi::Boolean>();
            return Constant::New(env, llvm::ConstantInt::get(type, value, isSigned));
        }
    }
    throw Napi::TypeError::New(env, ErrMsg::Class::ConstantInt::get);
}

Napi::Value ConstantInt::getType(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    llvm::IntegerType *type = constantInt->getType();
    return IntegerType::New(env, type);
}

//===----------------------------------------------------------------------===//
//                        ConstantFP Class
//===----------------------------------------------------------------------===//

void ConstantFP::Init(Napi::Env env, Napi::Object &exports) {
    Napi::HandleScope scope(env);
    const Napi::Function func = DefineClass(env, "ConstantFP", {
            StaticMethod("get", &ConstantFP::get),
            StaticMethod("getNaN", &ConstantFP::getNaN),
            InstanceMethod("getType", &ConstantFP::getType)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    Inherit(env, constructor.Value(), Constant::constructor.Value());
    exports.Set("ConstantFP", func);
}

Napi::Object ConstantFP::New(Napi::Env env, llvm::ConstantFP *constantFP) {
    return constructor.New({Napi::External<llvm::ConstantFP>::New(env, constantFP)});
}

bool ConstantFP::IsClassOf(const Napi::Value &value) {
    return value.IsNull() || value.As<Napi::Object>().InstanceOf(constructor.Value());
}

llvm::ConstantFP *ConstantFP::Extract(const Napi::Value &value) {
    if (value.IsNull()) {
        return nullptr;
    }
    return Unwrap(value.As<Napi::Object>())->getLLVMPrimitive();
}

ConstantFP::ConstantFP(const Napi::CallbackInfo &info) : ObjectWrap(info) {
    const Napi::Env env = info.Env();
    if (!info.IsConstructCall() || info.Length() == 0 || !info[0].IsExternal()) {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantFP::constructor);
    }
    const auto external = info[0].As<Napi::External<llvm::ConstantFP>>();
    constantFP = external.Data();
}

llvm::ConstantFP *ConstantFP::getLLVMPrimitive() {
    return constantFP;
}

Napi::Value ConstantFP::get(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() >= 2) {
        if (Type::IsClassOf(info[0])) {
            llvm::Type *type = Type::Extract(info[0]);
            llvm::Constant *result = nullptr;
            if (info[1].IsNumber()) {
                const double value = info[1].As<Napi::Number>();
                result = llvm::ConstantFP::get(type, value);
            } else if (APFloat::IsClassOf(info[1])) {
                const llvm::APFloat &value = APFloat::Extract(info[1]);
                result = llvm::ConstantFP::get(type, value);
            } else if (info[1].IsString()) {
                const std::string value = info[1].As<Napi::String>();
                result = llvm::ConstantFP::get(type, value);
            }
            if (result) {
                return Constant::New(env, result);
            }
        } else if (LLVMContext::IsClassOf(info[0]) && APFloat::IsClassOf(info[1])) {
            llvm::LLVMContext &context = LLVMContext::Extract(info[0]);
            const llvm::APFloat &value = APFloat::Extract(info[1]);
            llvm::ConstantFP *result = llvm::ConstantFP::get(context, value);
            return ConstantFP::New(env, result);
        }
    }
    throw Napi::TypeError::New(env, ErrMsg::Class::ConstantFP::get);
}

Napi::Value ConstantFP::getNaN(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() >= 1 && Type::IsClassOf(info[0])) {
        llvm::Type *type = Type::Extract(info[0]);
        llvm::Constant *nan = llvm::ConstantFP::getNaN(type);
        return Constant::New(env, nan);
    }
    throw Napi::TypeError::New(env, ErrMsg::Class::ConstantFP::getNaN);
}

Napi::Value ConstantFP::getType(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    llvm::Type *type = constantFP->getType();
    return Type::New(env, type);
}

//===----------------------------------------------------------------------===//
//                        ConstantArray Class
//===----------------------------------------------------------------------===//

void ConstantArray::Init(Napi::Env env, Napi::Object &exports) {
    Napi::HandleScope scope(env);
    const Napi::Function func = DefineClass(env, "ConstantArray", {
            StaticMethod("get", &ConstantArray::get),
            InstanceMethod("getType", &ConstantArray::getType)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    Inherit(env, constructor.Value(), Constant::constructor.Value());
    exports.Set("ConstantArray", func);
}

Napi::Object ConstantArray::New(Napi::Env env, llvm::ConstantArray *constantArray) {
    return constructor.New({Napi::External<llvm::ConstantArray>::New(env, constantArray)});
}

bool ConstantArray::IsClassOf(const Napi::Value &value) {
    return value.IsNull() || value.As<Napi::Object>().InstanceOf(constructor.Value());
}

llvm::ConstantArray *ConstantArray::Extract(const Napi::Value &value) {
    if (value.IsNull()) {
        return nullptr;
    }
    return Unwrap(value.As<Napi::Object>())->getLLVMPrimitive();
}

ConstantArray::ConstantArray(const Napi::CallbackInfo &info) : ObjectWrap(info) {
    const Napi::Env env = info.Env();
    if (!info.IsConstructCall() || info.Length() == 0 || !info[0].IsExternal()) {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantArray::constructor);
    }
    const auto external = info[0].As<Napi::External<llvm::ConstantArray>>();
    constantArray = external.Data();
}

llvm::ConstantArray *ConstantArray::getLLVMPrimitive() {
    return constantArray;
}

Napi::Value ConstantArray::get(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() < 2 || !ArrayType::IsClassOf(info[0]) || !info[1].IsArray()) {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantArray::get);
    }
    llvm::ArrayType *arrayType = ArrayType::Extract(info[0]);
    const auto valuesArray = info[1].As<Napi::Array>();
    const unsigned numValues = valuesArray.Length();
    std::vector<llvm::Constant *> values(numValues);
    for (unsigned i = 0; i < numValues; ++i) {
        values[i] = Constant::Extract(valuesArray.Get(i));
    }
    llvm::Constant *constantArray = llvm::ConstantArray::get(arrayType, values);
    return Constant::New(env, constantArray);
}

Napi::Value ConstantArray::getType(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    llvm::ArrayType *type = constantArray->getType();
    return ArrayType::New(env, type);
}

//===----------------------------------------------------------------------===//
//                        ConstantStruct Class
//===----------------------------------------------------------------------===//

void ConstantStruct::Init(Napi::Env env, Napi::Object &exports) {
    Napi::HandleScope scope(env);
    const Napi::Function func = DefineClass(env, "ConstantStruct", {
            StaticMethod("get", &ConstantStruct::get),
            InstanceMethod("getType", &ConstantStruct::getType)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    Inherit(env, constructor.Value(), Constant::constructor.Value());
    exports.Set("ConstantStruct", func);
}

Napi::Object ConstantStruct::New(Napi::Env env, llvm::ConstantStruct *constantStruct) {
    return constructor.New({Napi::External<llvm::ConstantStruct>::New(env, constantStruct)});
}

bool ConstantStruct::IsClassOf(const Napi::Value &value) {
    return value.IsNull() || value.As<Napi::Object>().InstanceOf(constructor.Value());
}

llvm::ConstantStruct *ConstantStruct::Extract(const Napi::Value &value) {
    if (value.IsNull()) {
        return nullptr;
    }
    return Unwrap(value.As<Napi::Object>())->getLLVMPrimitive();
}

ConstantStruct::ConstantStruct(const Napi::CallbackInfo &info) : ObjectWrap(info) {
    const Napi::Env env = info.Env();
    if (!info.IsConstructCall() || info.Length() == 0 || !info[0].IsExternal()) {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantStruct::constructor);
    }
    const auto external = info[0].As<Napi::External<llvm::ConstantStruct>>();
    constantStruct = external.Data();
}

llvm::ConstantStruct *ConstantStruct::getLLVMPrimitive() {
    return constantStruct;
}

Napi::Value ConstantStruct::get(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() < 2 || !StructType::IsClassOf(info[0]) || !info[1].IsArray()) {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantStruct::get);
    }
    llvm::StructType *structType = StructType::Extract(info[0]);
    const auto valuesArray = info[1].As<Napi::Array>();
    const unsigned numValues = valuesArray.Length();
    std::vector<llvm::Constant *> values(numValues);
    for (unsigned i = 0; i < numValues; ++i) {
        values[i] = Constant::Extract(valuesArray.Get(i));
    }
    llvm::Constant *constantStruct = llvm::ConstantStruct::get(structType, values);
    return Constant::New(env, constantStruct);
}

Napi::Value ConstantStruct::getType(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    llvm::StructType *type = constantStruct->getType();
    return StructType::New(env, type);
}

//===----------------------------------------------------------------------===//
//                        ConstantPointerNull Class
//===----------------------------------------------------------------------===//

void ConstantPointerNull::Init(Napi::Env env, Napi::Object &exports) {
    Napi::HandleScope scope(env);
    const Napi::Function func = DefineClass(env, "ConstantPointerNull", {
            StaticMethod("get", &ConstantPointerNull::get),
            InstanceMethod("getType", &ConstantPointerNull::getType)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    Inherit(env, constructor.Value(), Constant::constructor.Value());
    exports.Set("ConstantPointerNull", func);
}

Napi::Object ConstantPointerNull::New(Napi::Env env, llvm::ConstantPointerNull *constantPointerNull) {
    return constructor.New({Napi::External<llvm::ConstantPointerNull>::New(env, constantPointerNull)});
}

bool ConstantPointerNull::IsClassOf(const Napi::Value &value) {
    return value.IsNull() || value.As<Napi::Object>().InstanceOf(constructor.Value());
}

llvm::ConstantPointerNull *ConstantPointerNull::Extract(const Napi::Value &value) {
    if (value.IsNull()) {
        return nullptr;
    }
    return Unwrap(value.As<Napi::Object>())->getLLVMPrimitive();
}

ConstantPointerNull::ConstantPointerNull(const Napi::CallbackInfo &info) : ObjectWrap(info) {
    const Napi::Env env = info.Env();
    if (!info.IsConstructCall() || info.Length() == 0 || !info[0].IsExternal()) {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantPointerNull::constructor);
    }
    const auto external = info[0].As<Napi::External<llvm::ConstantPointerNull>>();
    constantPointerNull = external.Data();
}

llvm::ConstantPointerNull *ConstantPointerNull::getLLVMPrimitive() {
    return constantPointerNull;
}

Napi::Value ConstantPointerNull::get(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() == 0 || !PointerType::IsClassOf(info[0])) {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantPointerNull::get);
    }
    llvm::PointerType *type = PointerType::Extract(info[0]);
    return ConstantPointerNull::New(env, llvm::ConstantPointerNull::get(type));
}

Napi::Value ConstantPointerNull::getType(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    llvm::PointerType *type = constantPointerNull->getType();
    return PointerType::New(env, type);
}

//===----------------------------------------------------------------------===//
//                        ConstantDataArray Class
//===----------------------------------------------------------------------===//

void ConstantDataArray::Init(Napi::Env env, Napi::Object &exports) {
    Napi::HandleScope scope(env);
    const Napi::Function func = DefineClass(env, "ConstantDataArray", {
            StaticMethod("get", &ConstantDataArray::get),
            StaticMethod("getString", &ConstantDataArray::getString),
            InstanceMethod("getType", &ConstantDataArray::getType)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    Inherit(env, constructor.Value(), Constant::constructor.Value());
    exports.Set("ConstantDataArray", func);
}

Napi::Object ConstantDataArray::New(Napi::Env env, llvm::ConstantDataArray *constantDataArray) {
    return constructor.New({Napi::External<llvm::ConstantDataArray>::New(env, constantDataArray)});
}

bool ConstantDataArray::IsClassOf(const Napi::Value &value) {
    return value.IsNull() || value.As<Napi::Object>().InstanceOf(constructor.Value());
}

llvm::ConstantDataArray *ConstantDataArray::Extract(const Napi::Value &value) {
    if (value.IsNull()) {
        return nullptr;
    }
    return Unwrap(value.As<Napi::Object>())->getLLVMPrimitive();
}

ConstantDataArray::ConstantDataArray(const Napi::CallbackInfo &info) : ObjectWrap(info) {
    const Napi::Env env = info.Env();
    if (!info.IsConstructCall() || info.Length() == 0 || !info[0].IsExternal()) {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantDataArray::constructor);
    }
    const auto external = info[0].As<Napi::External<llvm::ConstantDataArray>>();
    constantDataArray = external.Data();
}

llvm::ConstantDataArray *ConstantDataArray::getLLVMPrimitive() {
    return constantDataArray;
}

Napi::Value ConstantDataArray::get(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    const unsigned argsLen = info.Length();
    if (argsLen == 2 && LLVMContext::IsClassOf(info[0]) && info[1].IsArray()) {
        std::vector<int64_t> array;
        if (assembleArray(info[1].As<Napi::Array>(), array)) {
            llvm::LLVMContext &context = LLVMContext::Extract(info[0]);
            llvm::Constant *constant = llvm::ConstantDataArray::get(context, array);
            return Constant::New(env, constant);
        }
    }
    throw Napi::TypeError::New(env, ErrMsg::Class::ConstantDataArray::get);
}

Napi::Value ConstantDataArray::getString(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    const unsigned argsLen = info.Length();
    if (argsLen == 2 && LLVMContext::IsClassOf(info[0]) && info[1].IsString() ||
        argsLen == 3 && LLVMContext::IsClassOf(info[0]) && info[1].IsString() && info[2].IsBoolean()) {
        llvm::LLVMContext &context = LLVMContext::Extract(info[0]);
        const std::string &initializer = info[1].As<Napi::String>();
        bool addNull = false;
        if (argsLen == 3) {
            addNull = info[2].As<Napi::Boolean>();
        }
        llvm::Constant *constant = llvm::ConstantDataArray::getString(context, initializer, addNull);
        return Constant::New(env, constant);
    } else {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantDataArray::getString);
    }
}

Napi::Value ConstantDataArray::getType(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    llvm::ArrayType *type = constantDataArray->getType();
    return ArrayType::New(env, type);
}

//===----------------------------------------------------------------------===//
//                        ConstantExpr Class
//===----------------------------------------------------------------------===//

void ConstantExpr::Init(Napi::Env env, Napi::Object &exports) {
    Napi::HandleScope scope(env);
    const Napi::Function func = DefineClass(env, "ConstantExpr", {
            // TODO: need more methods by use of factory mode
            StaticMethod("getBitCast", &ConstantExpr::getBitCast),
            StaticMethod("getAdd", &ConstantExpr::getAdd),
            StaticMethod("getSub", &ConstantExpr::getSub),
            StaticMethod("getMul", &ConstantExpr::getMul),
            StaticMethod("getXor", &ConstantExpr::getXor),
            StaticMethod("getNeg", &ConstantExpr::getNeg),
            StaticMethod("getFNeg",&ConstantExpr::getFNeg),
            StaticMethod("getNot",&ConstantExpr::getNot),
            StaticMethod("getAlignOf",&ConstantExpr::getAlignOf),
            StaticMethod("getSizeOf",&ConstantExpr::getSizeOf),
            StaticMethod("getOffsetOf",&ConstantExpr::getOffsetOf),
            StaticMethod("getAnd",&ConstantExpr::getAnd),
            StaticMethod("getOr",&ConstantExpr::getOr),
            StaticMethod("getUMin",&ConstantExpr::getUMin),
            StaticMethod("getShl",&ConstantExpr::getShl),
            StaticMethod("getLShr",&ConstantExpr::getLShr),
            StaticMethod("getAShr",&ConstantExpr::getAShr),
            StaticMethod("getTrunc",&ConstantExpr::getTrunc),
            StaticMethod("getSExt",&ConstantExpr::getSExt),
            StaticMethod("getZExt",&ConstantExpr::getZExt),
            StaticMethod("getFPTrunc",&ConstantExpr::getFPTrunc),
            StaticMethod("getFPExtend",&ConstantExpr::getFPExtend),
            StaticMethod("getUIToFP",&ConstantExpr::getUIToFP),
            StaticMethod("getSIToFP",&ConstantExpr::getUIToFP),
            StaticMethod("getFPToUI",&ConstantExpr::getFPToUI),
            StaticMethod("getFPToSI",&ConstantExpr::getFPToSI),
            InstanceMethod("getType", &ConstantExpr::getType),
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    Inherit(env, constructor.Value(), Constant::constructor.Value());
    exports.Set("ConstantExpr", func);
}

Napi::Object ConstantExpr::New(Napi::Env env, llvm::ConstantExpr *constantExpr) {
    return constructor.New({Napi::External<llvm::ConstantExpr>::New(env, constantExpr)});
}

bool ConstantExpr::IsClassOf(const Napi::Value &value) {
    return value.IsNull() || value.As<Napi::Object>().InstanceOf(constructor.Value());
}

llvm::ConstantExpr *ConstantExpr::Extract(const Napi::Value &value) {
    if (value.IsNull()) {
        return nullptr;
    }
    return Unwrap(value.As<Napi::Object>())->getLLVMPrimitive();
}

ConstantExpr::ConstantExpr(const Napi::CallbackInfo &info) : ObjectWrap(info) {
    const Napi::Env env = info.Env();
    if (!info.IsConstructCall() || info.Length() == 0 || !info[0].IsExternal()) {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::constructor);
    }
    const auto external = info[0].As<Napi::External<llvm::ConstantExpr>>();
    constantExpr = external.Data();
}

llvm::ConstantExpr *ConstantExpr::getLLVMPrimitive() {
    return constantExpr;
}

Napi::Value ConstantExpr::getBitCast(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() == 2 && Constant::IsClassOf(info[0]) && Type::IsClassOf(info[1])) {
        llvm::Constant *constant = Constant::Extract(info[0]);
        llvm::Type *type = Type::Extract(info[1]);
        return Constant::New(env, llvm::ConstantExpr::getBitCast(constant, type));
    }
    throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getBitCast);
}

Napi::Value ConstantExpr::getAdd(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>4 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getAdd);
    }

    llvm::Constant *lhs = Constant::Extract(info[0]);
    llvm::Constant *rhs = Constant::Extract(info[1]);

    bool hasNUW = false, hasNSW = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        hasNUW = info[2].As<Napi::Boolean>().Value();
    if (info.Length() >= 4 && info[3].IsBoolean())
        hasNSW = info[3].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getAdd(lhs, rhs, hasNUW, hasNSW));
}

Napi::Value ConstantExpr::getSub(const Napi::CallbackInfo &info)
{
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>4 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getSub);
    }

    llvm::Constant *lhs = Constant::Extract(info[0]);
    llvm::Constant *rhs = Constant::Extract(info[1]);

    bool hasNUW = false, hasNSW = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        hasNUW = info[2].As<Napi::Boolean>().Value();
    if (info.Length() >= 4 && info[3].IsBoolean())
        hasNSW = info[3].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getSub(lhs, rhs, hasNUW, hasNSW));
}

Napi::Value ConstantExpr::getMul(const Napi::CallbackInfo &info)
{
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>4 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getMul);
    }

    llvm::Constant *lhs = Constant::Extract(info[0]);
    llvm::Constant *rhs = Constant::Extract(info[1]);

    bool hasNUW = false, hasNSW = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        hasNUW = info[2].As<Napi::Boolean>().Value();
    if (info.Length() >= 4 && info[3].IsBoolean())
        hasNSW = info[3].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getMul(lhs, rhs, hasNUW, hasNSW));
}

Napi::Value ConstantExpr::getXor(const Napi::CallbackInfo &info)
{
    const Napi::Env env = info.Env();
    if(info.Length() != 2 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getXor);
    }

    llvm::Constant *lhs = Constant::Extract(info[0]);
    llvm::Constant *rhs = Constant::Extract(info[1]);

    return Constant::New(env, llvm::ConstantExpr::getXor(lhs, rhs));
}

Napi::Value ConstantExpr::getNeg(const Napi::CallbackInfo &info)
{
    const Napi::Env env = info.Env();
    if(info.Length()<1 || info.Length()>3 || !info[0].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getNeg);
    }

    llvm::Constant *c = Constant::Extract(info[0]);

    bool hasNUW = false, hasNSW = false;
    if (info.Length() >= 2 && info[1].IsBoolean())
        hasNUW = info[2].As<Napi::Boolean>().Value();
    if (info.Length() >= 3 && info[2].IsBoolean())
        hasNSW = info[3].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getNeg(c,hasNUW, hasNSW));
}

Napi::Value ConstantExpr::getFNeg(const Napi::CallbackInfo &info)
{
    const Napi::Env env = info.Env();
    if (info.Length() != 1 || !info[0].IsObject())
    {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getFNeg);
    }

    llvm::Constant *c = Constant::Extract(info[0]);

    return Constant::New(env, llvm::ConstantExpr::getFNeg(c));
}

Napi::Value ConstantExpr::getNot(const Napi::CallbackInfo& info){
    const Napi::Env env = info.Env();
    if (info.Length() != 1 || !info[0].IsObject())
    {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getNot);
    }

    llvm::Constant *c = Constant::Extract(info[0]);

    return Constant::New(env, llvm::ConstantExpr::getNot(c));
}

Napi::Value ConstantExpr::getAlignOf(const Napi::CallbackInfo& info)
{
    const Napi::Env env = info.Env();
    if(info.Length()!=1 || !info[0].IsObject()){
        throw Napi::TypeError::New(env,ErrMsg::Class::ConstantExpr::getAlignOf);
    }

    llvm::Type* type = Type::Extract(info[0]);

    return Constant::New(env,llvm::ConstantExpr::getAlignOf(type));
}

Napi::Value ConstantExpr::getSizeOf(const Napi::CallbackInfo& info){
    const Napi::Env env = info.Env();
    if(info.Length()!=1 || !info[0].IsObject()){
        throw Napi::TypeError::New(env,ErrMsg::Class::ConstantExpr::getSizeOf);
    }

    llvm::Type* type = Type::Extract(info[0]);

    return Constant::New(env,llvm::ConstantExpr::getSizeOf(type));
}

Napi::Value ConstantExpr::getOffsetOf(const Napi::CallbackInfo &info)
{
    const Napi::Env env = info.Env();
    if (info.Length() != 2 || !info[0].IsObject())
    {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getOffsetOf);
    }

    const Napi::Object obj0 = info[0].As<Napi::Object>();

    if (obj0.InstanceOf(StructType::constructor.Value()) && info[1].IsNumber())
    {
        llvm::StructType *type = StructType::Extract(obj0);
        const unsigned fieldNo = info[1].As<Napi::Number>().Uint32Value();
        llvm::Constant *c = llvm::ConstantExpr::getOffsetOf(type, fieldNo);
        return Constant::New(env, c);
    }
    else if (obj0.InstanceOf(Type::constructor.Value()) &&
             info[1].IsObject() &&
             info[1].As<Napi::Object>().InstanceOf(Constant::constructor.Value()))
    {
        llvm::Type *type = Type::Extract(obj0);
        llvm::Constant *FieldNo = Constant::Extract(info[1]);
        return Constant::New(env, llvm::ConstantExpr::getOffsetOf(type, FieldNo));
    }

    throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getOffsetOf);
}

Napi::Value ConstantExpr::getAnd(const Napi::CallbackInfo &info)
{
    const Napi::Env env = info.Env();

    if (info.Length() != 2 || !info[0].IsObject() || !info[1].IsObject())
    {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getAnd);
    }

    llvm::Constant *c1 = Constant::Extract(info[0]);
    llvm::Constant *c2 = Constant::Extract(info[1]);

    return Constant::New(env, llvm::ConstantExpr::getAnd(c1, c2));
}

Napi::Value ConstantExpr::getOr(const Napi::CallbackInfo &info)
{
    const Napi::Env env = info.Env();

    if (info.Length() != 2 || !info[0].IsObject() || !info[1].IsObject())
    {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getOr);
    }

    llvm::Constant *c1 = Constant::Extract(info[0]);
    llvm::Constant *c2 = Constant::Extract(info[1]);

    return Constant::New(env, llvm::ConstantExpr::getOr(c1, c2));
}

Napi::Value ConstantExpr::getUMin(const Napi::CallbackInfo &info)
{
    const Napi::Env env = info.Env();

    if (info.Length() != 2 || !info[0].IsObject() || !info[1].IsObject())
    {
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getOr);
    }

    llvm::Constant *c1 = Constant::Extract(info[0]);
    llvm::Constant *c2 = Constant::Extract(info[1]);

    return Constant::New(env, llvm::ConstantExpr::getUMin(c1, c2));
}

Napi::Value ConstantExpr::getShl(const Napi::CallbackInfo &info)
{
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>4 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getShl);
    }

    llvm::Constant *lhs = Constant::Extract(info[0]);
    llvm::Constant *rhs = Constant::Extract(info[1]);

    bool hasNUW = false, hasNSW = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        hasNUW = info[2].As<Napi::Boolean>().Value();
    if (info.Length() >= 4 && info[3].IsBoolean())
        hasNSW = info[3].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getShl(lhs, rhs, hasNUW, hasNSW));
}

Napi::Value ConstantExpr::getLShr(const Napi::CallbackInfo &info)
{
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>3 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getLShr);
    }

    llvm::Constant *lhs = Constant::Extract(info[0]);
    llvm::Constant *rhs = Constant::Extract(info[1]);

    bool isExact = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        isExact = info[2].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getLShr(lhs, rhs, isExact));
}

Napi::Value ConstantExpr::getAShr(const Napi::CallbackInfo &info)
{
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>3 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getAShr);
    }

    llvm::Constant *lhs = Constant::Extract(info[0]);
    llvm::Constant *rhs = Constant::Extract(info[1]);

    bool isExact = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        isExact = info[2].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getAShr(lhs, rhs, isExact));
}

Napi::Value ConstantExpr::getTrunc(const Napi::CallbackInfo &info)
{
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>3 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getTrunc);
    }

    llvm::Constant *c = Constant::Extract(info[0]);
    llvm::Type *type = Type::Extract(info[1]);

    bool OnlyIfReduced = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        OnlyIfReduced = info[2].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getTrunc(c,type, OnlyIfReduced));
}

Napi::Value ConstantExpr::getSExt(const Napi::CallbackInfo& info){
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>3 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getSExt);
    }

    llvm::Constant *c = Constant::Extract(info[0]);
    llvm::Type *type = Type::Extract(info[1]);

    bool OnlyIfReduced = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        OnlyIfReduced = info[2].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getSExt(c,type, OnlyIfReduced));
}

Napi::Value ConstantExpr::getZExt(const Napi::CallbackInfo& info){
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>3 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getZExt);
    }

    llvm::Constant *c = Constant::Extract(info[0]);
    llvm::Type *type = Type::Extract(info[1]);

    bool OnlyIfReduced = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        OnlyIfReduced = info[2].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getZExt(c,type, OnlyIfReduced));
}

Napi::Value ConstantExpr::getFPTrunc(const Napi::CallbackInfo& info){
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>3 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getFPTrunc);
    }

    llvm::Constant *c = Constant::Extract(info[0]);
    llvm::Type *type = Type::Extract(info[1]);

    bool OnlyIfReduced = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        OnlyIfReduced = info[2].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getFPTrunc(c,type, OnlyIfReduced));
}

Napi::Value ConstantExpr::getFPExtend(const Napi::CallbackInfo& info){
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>3 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getFPExtend);
    }

    llvm::Constant *c = Constant::Extract(info[0]);
    llvm::Type *type = Type::Extract(info[1]);

    bool OnlyIfReduced = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        OnlyIfReduced = info[2].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getFPExtend(c,type, OnlyIfReduced));
}

Napi::Value ConstantExpr::getUIToFP(const Napi::CallbackInfo& info){
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>3 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getUIToFP);
    }

    llvm::Constant *c = Constant::Extract(info[0]);
    llvm::Type *type = Type::Extract(info[1]);

    if(!type->isFloatingPointTy()){
        throw Napi::TypeError::New(env, "Target type must be a floating-point type");
    }

    bool OnlyIfReduced = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        OnlyIfReduced = info[2].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getUIToFP(c,type, OnlyIfReduced));
}

Napi::Value ConstantExpr::getSIToFP(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>3 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getSIToFP);
    }

    llvm::Constant *c = Constant::Extract(info[0]);
    llvm::Type *type = Type::Extract(info[1]);

    if(!type->isFloatingPointTy()){
        throw Napi::TypeError::New(env, "Target type must be a floating-point type");
    }

    bool OnlyIfReduced = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        OnlyIfReduced = info[2].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getSIToFP(c,type, OnlyIfReduced));
}

Napi::Value ConstantExpr::getFPToUI(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>3 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getFPToUI);
    }

    llvm::Constant *c = Constant::Extract(info[0]);
    llvm::Type *type = Type::Extract(info[1]);

    if(!c->getType()->isFloatingPointTy()){
        throw Napi::TypeError::New(env, "Constant's type must be a floating-point type");
    }

    bool OnlyIfReduced = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        OnlyIfReduced = info[2].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getFPToUI(c,type, OnlyIfReduced));
}

Napi::Value ConstantExpr::getFPToSI(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if(info.Length()<2 || info.Length()>3 || !info[0].IsObject() || !info[1].IsObject()){
        throw Napi::TypeError::New(env, ErrMsg::Class::ConstantExpr::getFPToUI);
    }

    llvm::Constant *c = Constant::Extract(info[0]);
    llvm::Type *type = Type::Extract(info[1]);

    if(!c->getType()->isFloatingPointTy()){
        throw Napi::TypeError::New(env, "Constant's type must be a floating-point type");
    }

    bool OnlyIfReduced = false;
    if (info.Length() >= 3 && info[2].IsBoolean())
        OnlyIfReduced = info[2].As<Napi::Boolean>().Value();

    return Constant::New(env, llvm::ConstantExpr::getFPToSI(c,type, OnlyIfReduced));
}


Napi::Value ConstantExpr::getType(const Napi::CallbackInfo &info)
{
    return Type::New(info.Env(), constantExpr->getType());
}

//===----------------------------------------------------------------------===//
//                        UndefValue Class
//===----------------------------------------------------------------------===//

void UndefValue::Init(Napi::Env env, Napi::Object &exports) {
    Napi::HandleScope scope(env);
    const Napi::Function func = DefineClass(env, "UndefValue", {
            StaticMethod("get", &UndefValue::get),
            InstanceMethod("getType", &UndefValue::getType)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    Inherit(env, constructor.Value(), Constant::constructor.Value());
    exports.Set("UndefValue", func);
}

Napi::Object UndefValue::New(Napi::Env env, llvm::UndefValue *undefValue) {
    return constructor.New({Napi::External<llvm::UndefValue>::New(env, undefValue)});
}

bool UndefValue::IsClassOf(const Napi::Value &value) {
    return value.IsNull() || value.As<Napi::Object>().InstanceOf(constructor.Value());
}

llvm::UndefValue *UndefValue::Extract(const Napi::Value &value) {
    if (value.IsNull()) {
        return nullptr;
    }
    return Unwrap(value.As<Napi::Object>())->getLLVMPrimitive();
}

UndefValue::UndefValue(const Napi::CallbackInfo &info) : ObjectWrap(info) {
    const Napi::Env env = info.Env();
    if (!info.IsConstructCall() || info.Length() == 0 || !info[0].IsExternal()) {
        throw Napi::TypeError::New(env, ErrMsg::Class::UndefValue::constructor);
    }
    const auto external = info[0].As<Napi::External<llvm::UndefValue >>();
    undefValue = external.Data();
}

llvm::UndefValue *UndefValue::getLLVMPrimitive() {
    return undefValue;
}

Napi::Value UndefValue::get(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    if (info.Length() == 1 && Type::IsClassOf(info[0])) {
        llvm::Type *type = Type::Extract(info[0]);
        llvm::UndefValue *undefValue = llvm::UndefValue::get(type);
        return UndefValue::New(env, undefValue);
    }
    throw Napi::TypeError::New(env, ErrMsg::Class::UndefValue::get);
}

Napi::Value UndefValue::getType(const Napi::CallbackInfo &info) {
    const Napi::Env env = info.Env();
    llvm::Type *type = undefValue->getType();
    return Type::New(env, type);
}
