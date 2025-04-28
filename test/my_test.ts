import llvm from '..';


const context = new llvm.LLVMContext();
const module = new llvm.Module('main', context);
const builder = new llvm.IRBuilder(context);

const returnType = builder.getInt32Ty();
const functionType = llvm.FunctionType.get(returnType, [], false);
const func = llvm.Function.Create(functionType, llvm.Function.LinkageTypes.ExternalLinkage, 'main', module);

const id = func.getIntrinsicID();
if(!func.isConstrainedFPIntrinsic()){
    console.log("FUCK")
}

const entryBB = llvm.BasicBlock.Create(context, 'entry', func);
builder.SetInsertPoint(entryBB);

// console.log(module.print())