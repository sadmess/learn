#include <algorithm>
#include <vector>
#include <llvm\Analysis\LoopPass.h>
#include <llvm\IR\Dominators.h>
#include <llvm\Analysis\ValueTracking.h>
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
class MyPass : public LoopPass {
private:
  std::vector<Instruction *> invariants;

public:
  static char ID;

  MyPass() : LoopPass(ID) {}

  ~MyPass() {}

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.setPreservesCFG();
  }

  virtual bool testCMConditions(const Instruction *I) { 
    return isSafeToSpeculativelyExecute(I) && //过滤掉会引发异常的指令，以及phi br alloca等等，详见实现
        !I->mayReadFromMemory() && //与内存相关的指令不予操作
        !isa<LandingPadInst>(I); //异常处理指令不予操作
  }

  virtual bool
  testDominateExitingBlocks(const Instruction *I,
                            SmallVectorImpl<BasicBlock *> &exitingBlocks,
                            DominatorTree &DT) {
    bool result = true;
    for (BasicBlock *exitBlock : exitingBlocks) {
      if (!DT.dominates(I, exitBlock)) {
        result = false;
        break;
      }
    }
    return result;
  }

  //循环不变计算检测，输入：循环L，每个三地址指令的ud链。输出：L的循环不变计算语句。
  virtual bool findInvariants(Loop *L) {
    bool if_found_invariants = false;
    for (auto blockIter = L->block_begin(); blockIter != L->block_end();
         ++blockIter) {
      BasicBlock &bb = **blockIter;
      for (auto instIter = bb.begin(); instIter != bb.end(); ++instIter) {
        Instruction &inst = *instIter;
        // 已经是循环不变量的指令不做检查
        if (std::find(invariants.begin(), invariants.end(), &inst) ==
            invariants.end()) {
          bool verified_invariant = true;
          //遍历指令的所有操作数，只有一个指令的所有的运算分量是循环不变量时该指令也是循环不变量
          for (auto opIter = inst.op_begin(); opIter != inst.op_end();
               ++opIter) {
            Value *operand = *opIter;
            if (Instruction *instOperand = dyn_cast<Instruction>(operand)) {
              if (std::find(invariants.begin(), invariants.end(),
                            instOperand) == invariants.end()) {
                // 如果一个运算分量既不是循环不变量并且在循环内部
                if (L->contains(instOperand)) {
                  // 那么暂时不能确定包含他的指令是不是循环不变量
                  verified_invariant = false;
                } else {
                   //如果指令不在循环内部，那么他是循环不变量
                  invariants.push_back(instOperand);
                }
              }
            }
          }
          //如果是就加入（雾
          if (verified_invariant) {
            invariants.push_back(&inst);
            if_found_invariants = true;
          }
        }
      }
    }
    return if_found_invariants;
  }

  virtual bool runOnLoop(Loop *L, LPPassManager &LPM) {
    bool changed = false;
    SmallVector<BasicBlock *, 8> exitingBlocks;
    L->getExitingBlocks(exitingBlocks);
    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();

    BasicBlock *preHeader = L->getLoopPreheader();
    if (preHeader != NULL) {
      //重复执行，直到某次没有新的语句可标记为“不变”为止。
      bool found_more_invariants = true;
      while (found_more_invariants) {
        found_more_invariants = findInvariants(L);
      }

      // 将符合条件的语句移动到prehead，三个条件因为ssa所以仅需要保证s所在的基本块是循环所有出口结点(有后继结点在循环外的结点)的支配结点
      auto InsertPt = preHeader->getTerminator();
      for (Instruction *invariant : invariants) {
        if (testDominateExitingBlocks(invariant, exitingBlocks, DT) &&
            testCMConditions(invariant)) {
          // Move the instruction to preHeader
          invariant->moveBefore(InsertPt);
          changed = true;
        }
      }
    }
    return changed;
  }
};
char MyPass::ID = 1;
INITIALIZE_PASS(MyPass, "CSCD70", "ass3", false, false)