#include "framework.h"
#include "llvm/InitializePasses.h"
class Variable {
private:
  const Value *_val;

public:
  Variable(const Value *val) : _val(val) {}

  bool operator==(const Variable &val) const { return _val == val.getValue(); }

  const Value *getValue() const { return _val; }

  friend raw_ostream &operator<<(raw_ostream &outs, const Variable &val);
};

raw_ostream &operator<<(raw_ostream &outs, const Variable &var) {
  outs << "[";
  var._val->printAsOperand(outs, false);
  outs << "]";
  return outs;
}


template <> struct std::hash<Variable> {
  std::size_t operator()(const Variable &var) const {
    std::hash<const Value *> value_ptr_hasher;

    std::size_t value_hash = value_ptr_hasher((var.getValue()));

    return value_hash;
  }
};



/// @todo Implement @c Liveness using the @c dfa::Framework interface.
class MyPass final
    : public dfa::Framework<Variable, dfa::Direction::Backward> {
protected:
  virtual BitVector IC() const override {
    return BitVector(_domain.size(), false);
  }
  virtual BitVector BC() const override {
    return BitVector(_domain.size(), false);
  }

  virtual BitVector MeetOp(const BasicBlock &bb) const override {
    // @DONE 此处应该是集合并运算后的结果
    BitVector result(_domain.size(), false);
    for (const BasicBlock *block : successors(&bb)) {
      // 所有后继基础块的第一条Instruction的IN集合就是当前基础块的OUT集
      BitVector curr_bv = _inst_bv_map.at(&block->front());
      // 对含有phi指令的基础块作特殊处理，因为phi指令涉及到的变量需要来自于我们要处理的块才是活跃的
      // 当前处理的后继块遍历所有phi指令
      for (auto phi_iter = block->phis().begin();
           phi_iter != block->phis().end(); phi_iter++) {
        const PHINode &phi_inst = *phi_iter;
        // 遍历当前phi指令可能的前驱基础块
        for (auto phi_inst_iter = phi_inst.block_begin();
             phi_inst_iter != phi_inst.block_end(); phi_inst_iter++) {
          BasicBlock *const &curr_bb = *phi_inst_iter;
          // 如果当前前驱基础块不是现在的基础块
          if (curr_bb != &bb) {
            const Value *curr_val = phi_inst.getIncomingValueForBlock(curr_bb);
            // 如果当前值在domain中存在
            int idx = getDomainIndex(Variable(curr_val));
            if (idx != -1) {
              // 将临时变量中对应变量的bit设置为false
              assert(curr_bv[idx] = true);
              curr_bv[idx] = false;
            }
          }
        }
      }
      // 与临时变量做集合并操作
      result |= curr_bv;
    }
    return result;
  }
  virtual bool TransferFunc(const Instruction &inst, const BitVector &ibv,
                            BitVector &obv) override {
    // 注意：此时的ibv传入的是out集合，obv传入的是in集
    // @DONE 计算单个指令的IN集合
    // 注：使用getDomainIndex函数时，其参数中的Variable会隐式构造，不必手动调用

    BitVector new_obv = ibv;

    // use操作
    for (auto iter = inst.op_begin(); iter != inst.op_end(); iter++) {
      const Value *val = dyn_cast<Value>(*iter);
      assert(val != NULL);
      // 如果当前Variable存在于domain
      if (_domain.find(val) != _domain.end())
        new_obv[getDomainIndex(val)] = true;
    }
    // def操作。不是所有的指令都会定值，例如ret，所以要设置条件判断
    if (getDomainIndex(&inst) != -1)
      new_obv[getDomainIndex(&inst)] = false;

    bool hasChanged = new_obv != obv;
    obv = new_obv;
    return hasChanged;
  }
  virtual void
  InitializeDomainFromInstruction(const Instruction &inst) override {
    for (auto iter = inst.op_begin(); iter != inst.op_end(); iter++)
      if (isa<Instruction>(*iter) || isa<Argument>(*iter))
        _domain.emplace(Variable(*iter));
  }

public:
  static char ID;

  MyPass() : dfa::Framework<domain_element_t, direction_c>(ID) {}
  virtual ~MyPass() override {}
};

char MyPass::ID = 1;
INITIALIZE_PASS(MyPass, "CSCD70", "ass2", false, false)