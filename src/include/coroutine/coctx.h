#ifndef TINYRPC_COMMON_COROUTINE_COCTX_H
#define TINYRPC_COMMON_COROUTINE_COCTX_H

#include "coroutine/utils.h"

MRPC_NAMESPACE_BEGIN

// 几个常见寄存器
enum {
	kRBP = 6,     // rbp, bottom of stack
	kRDI = 7,     // rdi, first para when call function
	kRSI = 8,     // rsi, second para when call function
	kRETAddr = 9, // the next excute cmd address, it will be assigned to rip
	kRSP = 13,    // rsp, top of stack
};

// 协程寄存器组
struct coctx {
	void* regs[14];
};


extern "C" {
// save current register's state to fitst coctx, and from second coctx take out
// register's state to assign register
extern void coctx_swap(coctx*, coctx*) asm("coctx_swap");		// 执行交换

};

MRPC_NAMESPACE_END

#endif
