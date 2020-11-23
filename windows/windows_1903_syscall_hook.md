
## 写在前面

window更新了1903，外挂要跟上时代步伐（才不是为了让1903的室友和我一起玩），可是上次提到的gDxgkInterface已经被取消了，于是google学习了新的利用方式


Windows 10 1903更新了对动态追踪技术的原生支持（这技术我也没用过，8太清楚，大概就是调试的时候可以下一种叫探针的东西///

## dtrace的初始化

通过ida分析ntoskrnl!KeInitSystem函数，可以发现KiInitDynamicTraceSupport()函数

```
_QWORD *KiInitDynamicTraceSupport()
{
  signed int v0; // ebx
  _QWORD *result; // rax
  unsigned int *v2; // rax
  __int64 v3; // [rsp+30h] [rbp+8h]
  __int64 v4; // [rsp+38h] [rbp+10h]

  v0 = 1;
  if ( !(_BYTE)KdDebuggerNotPresent )
    v0 = 7;
  result = (_QWORD *)TraceInitSystem(&v4, &KiDynamicTraceCallouts, &qword_140574A70);
  if ( (signed int)result >= 0 )
  {
    v2 = (unsigned int *)RtlLookupFunctionEntry(qword_140574A70, &v3, 0i64);
    if ( v2 )
    {
      qword_140574A70 = v3 + *v2;
      qword_140574A78 = v3 + v2[1];
    }
    result = (_QWORD *)v4;
    *(_QWORD *)v4 = &KiDynamicTraceContext;
    KiDynamicTraceEnabled = v0;
  }
  return result;
}
```

初始化函数对调试器进行检测，如果存在则将```KiDynamicTraceEnabled```设置为7，不是一个布尔值，应该包含其它含义

继续跟发现```TraceInitSystem```函数被调用，第一个参数被设置为一个名为```KiDynamicTraceContext```的全局变量

```
KiDynamicTraceContext dd 5E8h           ; DATA XREF: KiInitDynamicTraceSupport+30957↓o
.rdata:0000000140376AB4 01 00 00 00                                   dd 1
.rdata:0000000140376AB8 90 00 00 00                                   dd 90h
.rdata:0000000140376ABC 00 00 00 00                                   dd 0
.rdata:0000000140376AC0 10 16 88 40 01 00 00 00                       dq offset KeSetSystemServiceCallback
.rdata:0000000140376AC8 A0 0F 88 40 01 00 00 00                       dq offset KeSetTracepoint
.rdata:0000000140376AD0 20 BD 8F 40 01 00 00 00                       dq offset EtwRegisterEventCallback
```
## KeSetSystemServiceCallback的利用
因为目的是hook系统调用，所以```KeSetSystemServiceCallback```十分诱人，f5一下

```
if ( a3
    && a3 != *(_QWORD *)((char *)&KiDynamicTraceCallouts + (-(signed __int64)(a2 != 0) & 0xFFFFFFFFFFFFFFF8ui64) + 24) )
  {
    v4 = 0xC0000022;
  }
```

KiDynamicTraceCallouts应该是一个回调函数表
如果不是表内合法调用，则返回错误，无法设置

那么这里就是第一个要patch掉的地方

```
PAGE:0000000140881636 4D 85 C0                                      test    r8, r8
PAGE:0000000140881639 74 23                                         jz      short loc_14088165E
PAGE:000000014088163B 8A C2                                         mov     al, dl
PAGE:000000014088163D F6 D8                                         neg     al
PAGE:000000014088163F 48 8D 05 EA 33 CF FF                          lea     rax, KiDynamicTraceCallouts
PAGE:0000000140881646 4D 1B D2                                      sbb     r10, r10
PAGE:0000000140881649 49 83 E2 F8                                   and     r10, 0FFFFFFFFFFFFFFF8h
PAGE:000000014088164D 4D 3B 44 02 18                                cmp     r8, [r10+rax+18h]
PAGE:0000000140881652 74 0A                                         jz      short loc_14088165E
PAGE:0000000140881654 BF 22 00 00 C0                                mov     edi, 0C0000022h
PAGE:0000000140881659 E9 40 01 00 00                                jmp     loc_14088179E
```
只需将第一个74改为75即可

```
__int64 __fastcall KeSetSystemServiceCallback(_BYTE *a1, char a2, __int64 a3, __int64 a4)
{
  unsigned int v4; // edi
  __int64 v5; // r15
  __int64 v6; // rbp
  char v7; // r14
  _BYTE *v8; // rbx
  __int64 v9; // r8
  unsigned int v10; // ecx
  __int64 v11; // rax
  _DWORD *i; // rdx
  __int64 v13; // rsi
  struct _KTHREAD *v14; // rax
  unsigned __int64 v15; // rcx
  char v16; // r14
  char v17; // dl
  int v19; // [rsp+0h] [rbp-38h]

  v4 = 0;
  v5 = a4;
  v6 = a3;
  v7 = a2;
  v8 = a1;
  if ( a3 && a3 != *(_QWORD *)((char *)&KiDynamicTraceCallouts + (-(__int64)(a2 != 0) & 0xFFFFFFFFFFFFFFF8ui64) + 24) )
  {
    v4 = -1073741790;
  }
  else
  {
    v9 = KiGetSystemServiceTraceTable();
    if ( v9 )
    {
      v10 = 0;
      while ( *v8 )
      {
        v10 = ((1025 * (v10 + (char)*v8)) >> 6) ^ (1025 * (v10 + (char)*v8));
        ++v8;
      }
      v11 = 0i64;
      for ( i = (_DWORD *)(v9 + 52); v10 != *i; i += 16 )
      {
        v11 = (unsigned int)(v11 + 1);
        if ( (unsigned int)v11 >= 0x1D0 )
          return (unsigned int)-1073741275;
      }
      v13 = (v11 << 6) + v9 + 16;
      if ( !v13 )
        return (unsigned int)-1073741275;
      v14 = KeGetCurrentThread();
      --v14->KernelApcDisable;
      ExAcquirePushLockExclusiveEx((ULONG_PTR)&KiSystemServiceTraceCallbackLock, 0i64);
      v15 = -(__int64)(v7 != 0) & 0xFFFFFFFFFFFFFFF8ui64;
      v16 = -v7;
      v17 = *(_BYTE *)(v13 - (v16 != 0) + 41);
      if ( v6 )
      {
        if ( v17 )
        {
          v4 = -1073740008;
        }
        else
        {
          *(_QWORD *)(v15 + v13 + 56) = v5;
          _InterlockedOr(&v19, 0);
          *(_BYTE *)(v13 - (v16 != 0) + 41) = 1;
          if ( ++KiSystemServiceTraceCallbackCount == 1 )
            _InterlockedOr(&KiDynamicTraceMask, 1u);
        }
      }
      else if ( v17 )
      {
        if ( !--KiSystemServiceTraceCallbackCount )
          _InterlockedAnd(&KiDynamicTraceMask, 0xFFFFFFFE);
        *(_BYTE *)(v13 - (v16 != 0) + 41) = 0;
        _InterlockedOr(&v19, 0);
        while ( KiSystemServiceTraceCallbacksActive )
          _mm_pause();
        *(_QWORD *)(v15 + v13 + 56) = 0i64;
      }
      if ( (_InterlockedExchangeAdd64(
              (volatile signed __int64 *)&KiSystemServiceTraceCallbackLock,
              0xFFFFFFFFFFFFFFFFui64) & 6) == 2 )
        ExfTryToWakePushLock(&KiSystemServiceTraceCallbackLock);
      KeAbPostRelease((ULONG_PTR)&KiSystemServiceTraceCallbackLock);
      KeLeaveCriticalRegionThread(KeGetCurrentThread());
    }
    else
    {
      v4 = -1073741670;
    }
  }
  return v4;
}
```

第一个参数经检验为不带nt开头的系统调用函数名（后面会对其进行简单hash，可通过此检查）

第二个参数为isEntry，即是syscall执行之前进行回调还是之后进行回调

第三个参数为要设置的callback

第四个参数unknown

v17变量是在函数注册后被设置为1，所以if（v17）校验可直接通过，最后```_InterlockedOr(&KiDynamicTraceMask, 1u);```将KiDynamicTraceMask设置为1，后面这个1也可以通过一处if（多通过一个，少一个patch，多一点稳定hhhhh~~~

在```kisystemcall64```函数中
KiDynamicTraceMask已经被设置为1
```
if ( KiDynamicTraceMask & 1 )
        {
          v50 = v22;
          v51 = v23;
          v52 = a5;
          v53 = a6;
          v54 = v30;
          v62 = KiTrackSystemCallEntry((__int64)v30, (__int64)&v50, 4, (__int64)&v60);
          v48 = v54(v50, v51, v52, v53);
          result = KiTrackSystemCallExit(v62, v48);
        }
```
汇编
```
sub     rsp, 50h
.text:00000001401D3197                 mov     [rsp+1E0h+var_1C0], rcx
.text:00000001401D319C                 mov     [rsp+1E0h+var_1B8], rdx
.text:00000001401D31A1                 mov     [rsp+1E0h+var_1B0], r8
.text:00000001401D31A6                 mov     [rsp+1E0h+var_1A8], r9
.text:00000001401D31AB                 mov     [rsp+1E0h+var_1A0], r10
.text:00000001401D31B0                 mov     rcx, r10
.text:00000001401D31B3                 mov     rdx, rsp
.text:00000001401D31B6                 add     rdx, 20h ; ' '
.text:00000001401D31BA                 mov     r8, 4
.text:00000001401D31C1                 mov     r9, rsp
.text:00000001401D31C4                 add     r9, 70h ; 'p'
.text:00000001401D31C8                 call    KiTrackSystemCallEntry
.text:00000001401D31CD                 mov     [rbp-50h], rax
.text:00000001401D31D1                 mov     rcx, [rsp+1E0h+var_1C0]
.text:00000001401D31D6                 mov     rdx, [rsp+1E0h+var_1B8]
.text:00000001401D31DB                 mov     r8, [rsp+1E0h+var_1B0]
.text:00000001401D31E0                 mov     r9, [rsp+1E0h+var_1A8]
.text:00000001401D31E5                 mov     r10, [rsp+1E0h+var_1A0]
.text:00000001401D31EA                 add     rsp, 50h
.text:00000001401D31EE                 mov     rax, r10
.text:00000001401D31F1                 call    rax
```
这里只要控制KiTrackSystemCallEntry，修改栈信息，即可控制rax，保证传递正确参数的同时，调用我们想调用的自写函数

继而进入KiTrackSystemCallEntry
```
if ( v8 && *(_BYTE *)(v8 + 40) && KiDynamicTraceEnabled )
    {
      _InterlockedIncrement(&KiSystemServiceTraceCallbacksActive);
      qword_140574A40(*(_QWORD *)(v8 + 24), *(_QWORD *)(v8 + 48), *(unsigned int *)(v8 + 32), v7, v6, v5);
      _InterlockedDecrement(&KiSystemServiceTraceCallbacksActive);
    }
```
调用qword_140574A40，我们通过patch，先通过外部if，再修改qword_140574A40为另外一个空闲地址（提前放置好shellcode）
shellcode要实现的就是控制r10为要执行的自写函数
shellcode:

```
    ;rcx 为原syscall
        push rsi
	push r12
	lea r12, hook func
	lea rax, [rsp + 48] 
	mov rsi, 20 
again:                  ;在栈上寻找原syscall
	add rax, 8
	dec rsi
	test rsi,rsi
	jz fail
	cmp qword ptr[rax], rcx
	jne again
	mov [rax], r12
fail:
	pop r12
	pop rsi
	xor rax,rax
	ret
```

这样只需要在驱动中调用```KeSetSystemServiceCallback("syscallWithoutNt", TRUE, (ULONG64)callback, NULL)```

callback执行的时候shellcode将栈上下文控制rax执行到我们要执行的函数
实现hook

至于解hook。。。emmmm，因为驱动加载和最终目的的原因，8考虑了
