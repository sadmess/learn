#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Custom.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/InitializePasses.h"
#include <iostream>
using namespace llvm;

namespace {
  struct MyPass : public ModulePass {
      bool m_flag;
    std::vector<Instruction *> deleteInst;
    static char ID;
      MyPass() : MyPass(ID) {
        m_flag = false;
      }
    MyPass(bool flag) : ModulePass(ID) {
        this->m_flag = flag;
      initializeMyPassPass(*PassRegistry::getPassRegistry());
    }
    virtual void getAnalysisUsage(AnalysisUsage& AU) const {
      AU.setPreservesAll();
    }
    bool functionInfo(Module & M) { 
        errs() << "CSCD70 Functions Information Pass"<< "\n";
        errs() << "Number of function:" << M.size() << "\n";
        int numFuncs = M.size();
        StringRef *Names = new StringRef[numFuncs]();
        int *numArgs = new int[numFuncs]();
        int *numCalls = new int[numFuncs]();
        int *numBlcoks = new int[numFuncs]();
        int *numInsts = new int[numFuncs]();
        int index = 0;
        for (auto i1 = M.begin(); i1 != M.end(); i1++) {
          Function &F = *i1;
          StringRef name = F.getName();
          Names[index] = name;
          numArgs[index] = F.arg_size();
          numBlcoks[index] = F.size();
          int numInstsTmp = 0;
          for (auto i2 = F.begin(); i2 != F.end(); i2++) {
            BasicBlock &B = *i2;
            numInstsTmp += B.size();
            for (auto i3 = B.begin(); i3 != B.end(); i3++) {
              if (CallInst *c = dyn_cast<CallInst>(i3)) {
                if (c->getCalledFunction()) {
                  int calleeIndex = 0;
                  StringRef callee = c->getCalledFunction()->getName();
                  for (int i = 0; i < numFuncs; i++) {
                    if (Names[i].equals(callee)) {
                      calleeIndex = i;
                    }
                  }
                  numCalls[calleeIndex]++;
                }
              }
            }
          }
          numInsts[index] = numInstsTmp;
          index++;
        }
        for (auto iter = M.begin(); iter != M.end(); ++iter) {
          Function &func = *iter;
          errs()
              << "=================llvm=================\n"; // Eye catcher
          errs() << "Name:" << func.getName() << "\n";
          errs() << "Number of Arguments: " << func.arg_size() << "\n";
          errs() << "Number of Direct Call Sites in the same LLVM module: "
                 << func.getNumUses() << "\n";
          errs() << "Number of Basic Blocks: " << func.size() << "\n";
          errs() << "Number of Instructions: " << func.getInstructionCount()
                 << "\n";
          errs()
              << "===========================================\n"; // Eye catcher
        }
        for (int i = 0; i < numFuncs; i++) {
          errs()
              << "===========================================\n"; // Eye catcher
          errs() << "Function name: " << Names[i].str() << "\n";
          errs() << "Number of arguments: " << numArgs[i] << "\n";
          errs() << "Number of calls: " << numCalls[i] << "\n";
          errs() << "Number of basic blocks: " << numBlcoks[i] << "\n";
          errs() << "Number of instructions: " << numInsts[i] << "\n";
          errs()
              << "===========================================\n"; // Eye catcher
        }
        return false;
    }
    
    void multiInstCase1(Instruction& I, Value* toReplace, int* numOfOpts) {
      for (auto i1 = I.user_begin(); i1 != I.user_end(); i1++) {
        Instruction &user = *(dyn_cast<Instruction>(*i1));
        if (user.isBinaryOp()) {
          return;
        }
        auto opiter = user.op_begin();
        Value *opFirst = *(opiter);
        Value *opSecond = *(++opiter);
        if (!(opFirst->getType()->isIntegerTy()&&opSecond->getType()->isIntegerTy())) {
          return;
        }
        if (user.getOpcode() == Instruction::Sub) {
          if (ConstantInt *cSecond = dyn_cast<ConstantInt>(opSecond)) {
            if (cSecond->isOne()) {
              errs() << "Multi-Inst case a - 1: " << user << "\n";
              user.replaceAllUsesWith(toReplace);
              deleteInst.push_back(&user);
              (*numOfOpts)++;
            }
          }
        } 
        else if (user.getOpcode() == Instruction::Add) {
          if (ConstantInt *cFirst = dyn_cast<ConstantInt>(opFirst)) {
            if (cFirst->isMinusOne()) {
              errs() << "Multi-Inst case -1 + a: " << user << "\n";
              user.replaceAllUsesWith(toReplace);
              //user.eraseFromParent();
              (*numOfOpts)++;
            }
          } 
          else if (ConstantInt *cSecond = dyn_cast<ConstantInt>(opSecond)) {
            if (cSecond->isOne()) {
              errs() << "Multi-Inst case a + (-1): " << user << "\n";
              user.replaceAllUsesWith(toReplace);
              deleteInst.push_back(&user);
              (*numOfOpts)++;
            }
          }
        }
      }
    }
    void multiInstCase2(Instruction &I, Value *toReplace,
                        int *numOfOpts) // Multi-Inst a = b * 3 or a = 3 * b
    {
      for (auto iter = I.user_begin(); iter != I.user_end(); ++iter) {
        Instruction &user = *(dyn_cast<Instruction>(*iter));
        if (!(user.isBinaryOp())) // Only consider binary ops
        {
          return;
        }
        auto opIter = user.op_begin();
        Value *_1st_operand = *(opIter);
        Value *_2nd_operand = *(++opIter);
        if (!(_1st_operand->getType()->isIntegerTy() &&
              _2nd_operand->getType()->isIntegerTy())) {
          return; // Both the operands must be integer
        }
        if (user.getOpcode() == Instruction::UDiv || user.getOpcode() == Instruction::SDiv) // udiv or sdiv
        {
          if (ConstantInt *c2 = dyn_cast<ConstantInt>(_2nd_operand)) {
            if (c2->equalsInt(
                    3)) // c = a / 3 where a can be either signed or unsigned
            {
              errs() << "Multi-Inst case a / 3: " << user << "\n";
              user.replaceAllUsesWith(toReplace);
              deleteInst.push_back(&user);
              (*numOfOpts)++;
            }
          }
        }
      }
    }
    int *runOnBasicBlock(BasicBlock &B) {
      
      int numOfOpts = 0;
      int numOfOptsAI = 0; // Number of Algebraic Identity optimizations
      int numOfOptsSR = 0; // Number of Strength Reductions optimizations
      int numOfOptsMI = 0; // Number of Multi-Inst optimizations
      int *result = new int[4]();

      for (auto instIter = B.begin(); instIter != B.end(); ++instIter) {
        
        Instruction &inst = *instIter;
        errs() << inst << '\n';
        if (inst.isBinaryOp()) // 如果是两操作运算符再进行进一步处理
        {
          auto opIter = inst.op_begin();
          Value *_1st_operand = *(opIter);
          Value *_2nd_operand = *(++opIter);
          if (!(_1st_operand->getType()->isIntegerTy() &&
                _2nd_operand->getType()->isIntegerTy())) {
            return 0; // 只处理操作数是整数的运算
          }

          if (inst.getOpcode() == Instruction::Add) // add
          {
            if (ConstantInt *c1 = dyn_cast<ConstantInt>(_1st_operand)) {
              if (c1->isZero()) // 0 + x
              {
                errs() << "Algebraic Identity case 0 + x: " << inst << "\n";
                numOfOptsAI++;
                numOfOpts++;
                inst.replaceAllUsesWith(_2nd_operand);
                deleteInst.push_back(&inst);
              } else if (c1->isOne()) // Multi-Inst a = 1 + b
              {
                int subnum = 0;
                multiInstCase1(inst, _2nd_operand, &subnum);
                numOfOptsMI += subnum;
                numOfOpts += subnum;
              }
            } else if (ConstantInt *c2 = dyn_cast<ConstantInt>(_2nd_operand)) {
              if (c2->isZero()) // x + 0
              {
                errs() << "Algebraic Identity case x + 0: " << inst << "\n";
                numOfOptsAI++;
                numOfOpts++;
                inst.replaceAllUsesWith(_1st_operand);
                deleteInst.push_back(&inst);
              } else if (c2->isOne()) // Multi-Inst a = b + 1
              {
                errs() << "Multi-Inst a = b + 1 " << inst << "\n";
                int subnum = 0;
                multiInstCase1(inst, _1st_operand, &subnum);
                numOfOptsMI += subnum;
                numOfOpts += subnum;
              }
            }
          } else if (inst.getOpcode() == Instruction::Mul) // mul
          {
            if (ConstantInt *c1 = dyn_cast<ConstantInt>(_1st_operand)) {
              if (c1->isOne()) // 1 * x
              {
                errs() << "Algebraic Identity case 1 * x: " << inst << "\n";
                numOfOptsAI++;
                numOfOpts++;
                inst.replaceAllUsesWith(_2nd_operand);
                deleteInst.push_back(&inst);
              } else if (c1->equalsInt(2)) // 2 * x
              {
                errs() << "Strength Reductions case 2 * x: " << inst << "\n";
                numOfOptsSR++;
                numOfOpts++;
                Instruction *new_inst = BinaryOperator::Create(
                    Instruction::Shl, _2nd_operand,
                    ConstantInt::get(B.getContext(),
                                     llvm::APInt(32, 1, false)));
                new_inst->insertAfter(&inst);
                inst.replaceAllUsesWith(new_inst);
                deleteInst.push_back(&inst);
              } else if (c1->equalsInt(3)) // Multi-Inst a = 3 * b
              {
                int subnum = 0;
                multiInstCase2(inst, _2nd_operand, &subnum);
                numOfOptsMI += subnum;
                numOfOpts += subnum;
              }
            } else if (ConstantInt *c2 = dyn_cast<ConstantInt>(_2nd_operand)) {
              if (c2->isOne()) // x * 1
              {
                errs() << "Algebraic Identity case x * 1: " << inst << "\n";
                numOfOptsAI++;
                numOfOpts++;
                inst.replaceAllUsesWith(_1st_operand);
                deleteInst.push_back(&inst);
              } else if (c2->equalsInt(2)) // x * 2
              {
                errs() << "Strength Reductions case x * 2: " << inst << "\n";
                numOfOptsSR++;
                numOfOpts++;
                Instruction *new_inst = BinaryOperator::Create(
                    Instruction::Shl, _1st_operand,
                    ConstantInt::get(B.getContext(),
                                     llvm::APInt(32, 1, false)));
                new_inst->insertAfter(&inst);
                inst.replaceAllUsesWith(new_inst);
                deleteInst.push_back(&inst);
              } else if (c2->equalsInt(3)) // Multi-Inst a = b * 3
              {
                int subnum = 0;
                multiInstCase2(inst, _1st_operand, &subnum);
                numOfOptsMI += subnum;
                numOfOpts += subnum;
              }
            }

          } else if (inst.getOpcode() == Instruction::UDiv) // udiv
          {
            if (ConstantInt *c2 = dyn_cast<ConstantInt>(_2nd_operand)) {
              if (c2->equalsInt(2)) // x / 2 where x is unsigned
              {
                errs() << "Strength Reductions case x / 2 (unsigned x): "
                       << inst << "\n";
                numOfOptsSR++;
                numOfOpts++;
                Instruction *new_inst = BinaryOperator::Create(
                    Instruction::AShr, _1st_operand,
                    ConstantInt::get(B.getContext(),
                                     llvm::APInt(32, 1, false)));
                new_inst->insertAfter(&inst);
                inst.replaceAllUsesWith(new_inst);
                deleteInst.push_back(&inst);
              }
            }
          }
        }
      }
      result[0] = numOfOpts;
      result[1] = numOfOptsAI;
      result[2] = numOfOptsSR;
      result[3] = numOfOptsMI;
      for (Instruction *inst : deleteInst) {
        if (inst->isSafeToRemove())
          inst->eraseFromParent();
      }
      return result;
    }
    int *runOnFunction(Function &F) {
      errs() << "\n============================================================"
                "=======================\n"; // Eye catcher
      errs() << "***** Function: " << F.getName() << "\n";
      errs() << "=============================================================="
                "=====================\n"; // Eye catcher

      int *result = new int[4]();

      for (auto iter = F.begin(); iter != F.end(); ++iter) {
        int *subresult = runOnBasicBlock(*iter);
        result[0] += subresult[0];
        result[1] += subresult[1];
        result[2] += subresult[2];
        result[3] += subresult[3];
        delete[] subresult;
      }
      return result;
    }
    bool localOpt(Module& M) {
      errs() << "\n[ CSCD70 Local Optimization Pass ]\n\n";
      errs() << "Test Module: " << M.getName() << "\n";

      int numOfOpts = 0;
      int numOfOptsAI = 0; // Number of Algebraic Identity optimizations
      int numOfOptsSR = 0; // Number of Strength Reductions optimizations
      int numOfOptsMI = 0; // Number of Multi-Inst optimizations

      for (auto iter = M.begin(); iter != M.end(); ++iter) {
        int *subresult = runOnFunction(*iter);
        numOfOpts += subresult[0];
        numOfOptsAI += subresult[1];
        numOfOptsSR += subresult[2];
        numOfOptsMI += subresult[3];
        delete[] subresult;
      }
      errs() << "\n------------------------------------------------------------"
                "-----------------------\n"; // Eye catcher
      errs() << "Summary --\n";
      errs() << "Transformations applied:\n";
      errs() << "    Algebraic Identity: " << numOfOptsAI << "\n";
      errs() << "    Strength Reduction: " << numOfOptsSR << "\n";
      errs() << "    Multi-Inst Optimization: " << numOfOptsMI << "\n";
      errs() << "    Total number of optimizations: " << numOfOpts << "\n";

      return (numOfOpts != 0);
    }
    virtual bool runOnModule(Module &M) {
        bool Changed = false;
        //functionInfo(M);
        localOpt(M);
        return Changed;
    }
  };
} // namespace
  char MyPass::ID = 1;
  INITIALIZE_PASS(MyPass, "CSCD70", "ass1", false, false)