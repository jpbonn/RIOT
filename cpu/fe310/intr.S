
#include "encoding.h"

#define	REGSIZE		4

  .section      .text.entry	
  .align 2
  .global trap_entry

trap_entry:
	// save all regs (except zero reg) on stack
	addi	sp, sp, -32*REGSIZE

	sw	x1,   1*REGSIZE(sp)
	sw	x2,   2*REGSIZE(sp)
	sw	x3,   3*REGSIZE(sp)
	sw	x4,   4*REGSIZE(sp)
	sw	x5,   5*REGSIZE(sp)
	sw	x6,   6*REGSIZE(sp)
	sw	x7,   7*REGSIZE(sp)
	sw	x8,   8*REGSIZE(sp)
	sw	x9,   9*REGSIZE(sp)
	sw	x10, 10*REGSIZE(sp)
	sw	x11, 11*REGSIZE(sp)
	sw	x12, 12*REGSIZE(sp)
	sw	x13, 13*REGSIZE(sp)
	sw	x14, 14*REGSIZE(sp)
	sw	x15, 15*REGSIZE(sp)
	sw	x16, 16*REGSIZE(sp)
	sw	x17, 17*REGSIZE(sp)
	sw	x18, 18*REGSIZE(sp)
	sw	x19, 19*REGSIZE(sp)
	sw	x20, 20*REGSIZE(sp)
	sw	x21, 21*REGSIZE(sp)
	sw	x22, 22*REGSIZE(sp)
	sw	x23, 23*REGSIZE(sp)
	sw	x24, 24*REGSIZE(sp)
	sw	x25, 25*REGSIZE(sp)
	sw	x26, 26*REGSIZE(sp)
	sw	x27, 27*REGSIZE(sp)
	sw	x28, 28*REGSIZE(sp)
	sw	x29, 29*REGSIZE(sp)
	sw	x30, 30*REGSIZE(sp)
	sw	x31, 31*REGSIZE(sp)

	// Call handle_trap with cause, exception pointer and current stack ptr
    csrr a0, mcause
    csrr a1, mepc
    mv   a2, sp
    call handle_trap

    // handle_trap returns sp of active thread
    mv   sp, a0
    li t0, MSTATUS_MPP
    csrs mstatus, t0

	lw	x1,   1*REGSIZE(sp)
	lw	x2,   2*REGSIZE(sp)
	lw	x3,   3*REGSIZE(sp)
	lw	x4,   4*REGSIZE(sp)
	lw	x5,   5*REGSIZE(sp)
	lw	x6,   6*REGSIZE(sp)
	lw	x7,   7*REGSIZE(sp)
	lw	x8,   8*REGSIZE(sp)
	lw	x9,   9*REGSIZE(sp)
	lw	x10, 10*REGSIZE(sp)
	lw	x11, 11*REGSIZE(sp)
	lw	x12, 12*REGSIZE(sp)
	lw	x13, 13*REGSIZE(sp)
	lw	x14, 14*REGSIZE(sp)
	lw	x15, 15*REGSIZE(sp)
	lw	x16, 16*REGSIZE(sp)
	lw	x17, 17*REGSIZE(sp)
	lw	x18, 18*REGSIZE(sp)
	lw	x19, 19*REGSIZE(sp)
	lw	x20, 20*REGSIZE(sp)
	lw	x21, 21*REGSIZE(sp)
	lw	x22, 22*REGSIZE(sp)
	lw	x23, 23*REGSIZE(sp)
	lw	x24, 24*REGSIZE(sp)
	lw	x25, 25*REGSIZE(sp)
	lw	x26, 26*REGSIZE(sp)
	lw	x27, 27*REGSIZE(sp)
	lw	x28, 28*REGSIZE(sp)
	lw	x29, 29*REGSIZE(sp)
	lw	x30, 30*REGSIZE(sp)
	lw	x31, 31*REGSIZE(sp)

    addi sp, sp, 32*REGSIZE
    mret

.weak handle_trap
handle_trap:
1:
  j 1b
	

