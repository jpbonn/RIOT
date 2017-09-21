/*
 * Copyright (C) 2017
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_fe310
 * @{
 *
 * @file        cpu.c
 * @brief       Implementation of the CPU initialization for SiFive FE310
 *
 * @author      
 * @}
 */

#include <stdio.h>
#include <errno.h>

#include "arch/thread_arch.h"
#include "arch/irq_arch.h"
#include "sched.h"
#include "thread.h"
#include "irq.h"
#include "cpu.h"
#include "sifive/encoding.h"
#include "sifive/platform.h"



extern uint32_t _sp;
extern uint32_t	__stack_size;

volatile int __in_isr = 0;

void trap_entry(void);
void thread_start(void);
void thread_switch(void);


/**
 * @brief Initialize the CPU, set IRQ priorities, clocks
 */
void cpu_init(void)
{
	// Setup trap handler function
	write_csr(mtvec, &trap_entry);

	// Enable FPU if present
	if (read_csr(misa) & (1 << ('F' - 'A')))
	{
		write_csr(mstatus, MSTATUS_FS);	// allow FPU instructions without trapping
	    write_csr(fcsr, 0);				// initialize rounding mode, undefined at reset
	}

	//	Initialize newlib-nano stubs
	nanostubs_init();

	//	Enable SW interrupt (for thread context switching)
	write_csr(mie, 0);
}



/**
 * @brief Enable all maskable interrupts
 */
unsigned int irq_arch_enable(void)
{
	// Enable all interrupts
	return set_csr(mstatus, MSTATUS_MIE);
}

/**
 * @brief Disable all maskable interrupts
 */
unsigned int irq_arch_disable(void)
{
	// Disable all interrupts
	return clear_csr(mstatus, MSTATUS_MIE);
}

/**
 * @brief Restore the state of the IRQ flags
 */
void irq_arch_restore(unsigned int state)
{
	// Restore all interrupts to given state
	set_csr(mstatus, state);
}

/**
 * @brief See if the current context is inside an ISR
 */
int irq_arch_in(void)
{
    return __in_isr;
}

/**
 * @brief Global trap and interrupt handler
 */
void handle_trap(unsigned int mcause, unsigned int epc, unsigned int sp, unsigned int mstatus)
{
	__in_isr = 1;

	//	Check for INT or TRAP
	if((mcause & MCAUSE_INT) == MCAUSE_INT)
	{
		//	Cause is an interrupt - determine type
		switch(mcause & MCAUSE_CAUSE)
		{
			case IRQ_M_SOFT:
				//	Software interrupt
				clear_csr(mie, MIP_MSIP);
				CLINT_REG(CLINT_MSIP) = 0;

				//	Reset interrupt
				set_csr(mie, MIP_MSIP);
				break;

			case IRQ_M_TIMER:
				//	Timer interrupt
				clear_csr(mie, MIP_MTIP);

				//	Handle timer interrupt
				timer_isr();

				//	Reset interrupt
				set_csr(mie, MIP_MTIP);
				break;

			case IRQ_M_EXT:
				//	External interrupt
				clear_csr(mie, MIP_MEIP);

				//	Handle external interrupt

				//	Reset interrupt
				set_csr(mie, MIP_MEIP);
				break;

			default:
				//	Unknown interrupt - panic!
				while(1);
				break;
		}

	}
	else
	{
		//	Cause is an exception trap - panic!
		while(1);
	}

	//	Check for a context switch
    if (sched_context_switch_request) {
        sched_run();
    }

    //	ISR done
    __in_isr = 0;
}


/**
 * @brief   Noticeable marker marking the beginning of a stack segment
 *
 * This marker is used e.g. by *thread_arch_start_threading* to identify the
 * stacks beginning.
 */
#define STACK_MARKER                (0x77777777)


char *thread_arch_stack_init(thread_task_func_t task_func,
                             void *arg,
                             void *stack_start,
                             int stack_size)
{
    uint32_t *stk, *stkFrame;

    //	Create the initial stack frame for this thread function
    stk = (uint32_t *)((uintptr_t)stack_start + stack_size);

    //	adjust to 32 bit boundary by clearing the last two bits in the address
    stk = (uint32_t *)(((uint32_t)stk) & ~((uint32_t)0x3));

    //	stack start marker
    stk--;
    *stk = STACK_MARKER;
    stkFrame = stk;
    stk -= 34;

    //	set all values of frame to 0
    for(int i=0; i<34; i++)
    	stk[i] = 0;

    //	set return address (x1 reg = ra) to be the task_exit function
    stk[1] = (unsigned int) sched_task_exit;

    //	set stack ptr (x2 reg = sp) to this initial stack frame
    stk[2] = (unsigned int) stkFrame;

    //	set a0 to given task args
    stk[10] = (unsigned int) arg;

    //	set initial mstatus and mepc
    stk[32] = (unsigned int) (MSTATUS_MPP);
    stk[33] = (unsigned int) task_func;

    return (char*) stk;
}

void thread_arch_stack_print(void)
{
    int count = 0;
    uint32_t *sp = (uint32_t *)sched_active_thread->sp;

    printf("printing the current stack of thread %" PRIkernel_pid "\n",
           thread_getpid());
    printf("  address:      data:\n");

    do {
        printf("  0x%08x:   0x%08x\n", (unsigned int)sp, (unsigned int)*sp);
        sp++;
        count++;
    } while (*sp != STACK_MARKER);

    printf("current stack size: %i byte\n", count);
}


int thread_arch_isr_stack_usage(void)
{
    return 0;
}

void *thread_arch_isr_stack_pointer(void)
{
    return NULL;
}

void *thread_arch_isr_stack_start(void)
{
	// ISR stack is carved out in LD file
    return (void *)&_sp;
}

void thread_arch_start_threading(void)
{
	//	Initialize threading
    sched_run();
    irq_arch_enable();
    thread_start();
    UNREACHABLE();
}

void thread_arch_yield(void)
{
	//	Switch to new thread
	thread_switch();
    UNREACHABLE();
}

