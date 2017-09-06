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

#include "arch/thread_arch.h"
#include "arch/irq_arch.h"
#include "sched.h"
#include "thread.h"
#include "irq.h"
#include "cpu.h"
#include "encoding.h"
#include "platform.h"



extern uint32_t _sp;
extern uint32_t _isr_sp;

extern uint32_t	__stack_size;
extern uint32_t	__isr_stack_size;

volatile int __in_isr = 0;

void trap_entry(void);


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

	//	Enable SW interrupt (for thread context switching)
	set_csr(mie, MIP_MSIP);
}

void _init(void)
{
}

void _fini(void)
{
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
unsigned int handle_trap(unsigned int mcause, unsigned int epc, unsigned int sp)
{
	__in_isr = 1;

	//	Check for INT or TRAP
	if(mcause & MCAUSE_INT)
	{
		//	Cause is an interrupt - determine type
		switch(mcause & MCAUSE_INT)
		{
			case IRQ_M_SOFT:
				//	Software interrupt - for thread scheduling
				clear_csr(mie, MIP_MSIP);
				CLINT_REG(CLINT_MSIP) = 0;

				//	Reset interrupt
				set_csr(mie, MIP_MSIP);
				break;

			case IRQ_M_TIMER:
				//	Timer interrupt
				clear_csr(mie, MIP_MTIP);

				//	Reset interrupt
				set_csr(mie, MIP_MTIP);
				break;

			case IRQ_M_EXT:
				//	External interrupt
				clear_csr(mie, MIP_MEIP);

				//	Reset interrupt
				set_csr(mie, MIP_MEIP);
				break;

			default:
				//	Unknown interrupt
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

    //	Return SP for running thread
    sp = (unsigned int) sched_active_thread->sp;
	__in_isr = 0;
	return sp;
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
    uint32_t *stk;
    stk = (uint32_t *)((uintptr_t)stack_start + stack_size);

    /* adjust to 32 bit boundary by clearing the last two bits in the address */
    stk = (uint32_t *)(((uint32_t)stk) & ~((uint32_t)0x3));

    /* stack start marker */
    stk--;
    *stk = STACK_MARKER;

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
    return (void *)&_isr_sp;
}

void thread_arch_start_threading(void)
{
	//	Initialize threading
    sched_active_thread = sched_threads[0];
    sched_run();
    CLINT_REG(CLINT_MSIP) = 1;

    UNREACHABLE();
}

void thread_arch_yield(void)
{
	//	Trigger a SW interrupt to schedule a new thread
	CLINT_REG(CLINT_MSIP) = 1;
}

