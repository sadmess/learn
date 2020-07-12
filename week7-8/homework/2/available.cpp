class Expression {
private:
  unsigned _opcode;
  const Value *_lhs, *_rhs;

public:
  Expression(const Instruction &inst) {
    // @DONE 初始化
    // 在该实验中，只考察二元运算符。所以可以放心的初始化_rhs
    _opcode = inst.getOpcode();
    _lhs = inst.getOperand(0);
    _rhs = inst.getOperand(1);
  }

  bool operator==(const Expression &Expr) const {
    switch (_opcode) {
    
    case Instruction::Add:
    case Instruction::FAdd:
    case Instruction::Mul:
    case Instruction::FMul:
      return ((Expr.getLHSOperand() == _lhs && Expr.getRHSOperand() == _rhs) ||
              (Expr.getLHSOperand() == _rhs && Expr.getRHSOperand() == _lhs) &&
                  Expr.getOpcode() == _opcode);
    default:
      return Expr.getLHSOperand() == _lhs && Expr.getRHSOperand() == _rhs &&
             Expr.getOpcode() == _opcode;
    }
  }

  unsigned getOpcode() const { return _opcode; }
  const Value *getLHSOperand() const { return _lhs; }
  const Value *getRHSOperand() const { return _rhs; }

  friend raw_ostream &operator<<(raw_ostream &outs, const Expression &expr);
};

raw_ostream &operator<<(raw_ostream &outs, const Expression &expr) {
  outs << "[" << Instruction::getOpcodeName(expr._opcode) << " ";
  expr._lhs->printAsOperand(outs, false);
  outs << ", ";
  expr._rhs->printAsOperand(outs, false);
  outs << "]";

  return outs;
}



// Construct a hash code for 'Expression'.
template <> struct std::hash<Expression> {
  std::size_t operator()(const Expression &expr) const {
    std::hash<unsigned> unsigned_hasher;
    std::hash<const Value *> pvalue_hasher;

    std::size_t opcode_hash = unsigned_hasher(expr.getOpcode());
    std::size_t lhs_operand_hash = pvalue_hasher((expr.getLHSOperand()));
    std::size_t rhs_operand_hash = pvalue_hasher((expr.getRHSOperand()));

    return opcode_hash ^ (lhs_operand_hash << 1) ^ (rhs_operand_hash << 1);
  }
};



class MyPass final
    : public dfa::Framework<Expression, dfa::Direction::Forward> {
protected:
  virtual BitVector IC() const override {
    return BitVector(_domain.size(), true);
  }
  virtual BitVector BC() const override {
    return BitVector(_domain.size(), false);
  }
  virtual BitVector MeetOp(const BasicBlock &bb) const override {
    // @DONE 此处应该是集合交运算后的结果
    BitVector result(_domain.size(), true);
    errs() << "----------------------------------------" << '\n';
    bool ispre = 0;
    for (const BasicBlock *block : predecessors(&bb)) {
      ispre = 1;
      block->dump();
      // 所有前驱基础块的最后一条Instruction的OUT集合，就是整个基础块的IN集
      const Instruction &last_inst_in_block = block->back();
      BitVector curr_bv = _inst_bv_map.at(&last_inst_in_block);
      errs() << "\n" << last_inst_in_block << ' ';
      //printDomainWithMask(curr_bv);
      errs() << "\n";
      result &= curr_bv;
    }
    if (!ispre) {
      BitVector zero(_domain.size(), false);
      result &= zero;
    }
      
    return result;
  }
  virtual bool TransferFunc(const Instruction &inst, const BitVector &ibv,
                            BitVector &obv) override {
    BitVector new_obv = ibv;

    //e_gen的计算，注意x op y加入egen和kill掉egen相关表达式的顺序不能互换

    // gen x_op_y 注意这里要判断是否为二元运算符
    if (isa<BinaryOperator>(inst) && _domain.find(inst) != _domain.end())
      new_obv[getDomainIndex(Expression(inst))] = true;
    // kill 所有引用的表达式
    for (auto elem : _domain)
      if (elem.getLHSOperand() == &inst || elem.getRHSOperand() == &inst)
        new_obv[getDomainIndex(elem)] = false;
    bool hasChanged = new_obv != obv;
    obv = new_obv;
    return hasChanged;
  }
  virtual void
  InitializeDomainFromInstruction(const Instruction &inst) override {
    if (isa<BinaryOperator>(inst)) {
      _domain.emplace(inst);
    }
  }

public:
  static char ID;

  MyPass() : dfa::Framework<domain_element_t, direction_c>(ID) {}
  virtual ~MyPass() override {}
};

char MyPass::ID = 1;
INITIALIZE_PASS(MyPass, "CSCD70", "ass1", false, false)