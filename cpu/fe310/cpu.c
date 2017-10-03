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
#include "periph_cpu.h"
#include "sifive/encoding.h"
#include "sifive/platform.h"
#include "sifive/plic_driver.h"


//	Default state of mstatus register
#define	MSTATUS_DEFAULT		(MSTATUS_MPP | MSTATUS_MPIE)


volatile int __in_isr = 0;

void trap_entry(void);
void thread_start(void);


//	PLIC external ISR function list
static external_isr_ptr_t	_ext_isrs[PLIC_NUM_INTERRUPTS];

//	NULL interrupt handler
void	null_isr(int num) {}

/**
 * @brief Initialize the CPU, set IRQ priorities, clocks
 */
void cpu_init(void)
{
    volatile uint64_t * mtimecmp    = (uint64_t*) (CLINT_CTRL_ADDR + CLINT_MTIMECMP);

    // Setup trap handler function
	write_csr(mtvec, &trap_entry);

	// Enable FPU if present
	if (read_csr(misa) & (1 << ('F' - 'A')))
	{
		write_csr(mstatus, MSTATUS_FS);	// allow FPU instructions without trapping
	    write_csr(fcsr, 0);				// initialize rounding mode, undefined at reset
	}

	//	Clear all interrupt enables
	write_csr(mie, 0);

	//	Initial PLIC external interrupt controller
	PLIC_init(PLIC_CTRL_ADDR, PLIC_NUM_INTERRUPTS, PLIC_NUM_PRIORITIES);

	//	Initialize ISR function list
	for (int i = 0; i < PLIC_NUM_INTERRUPTS; i++)
	{
		_ext_isrs[i] = null_isr;
	}

	//	Set mtimecmp to largest value to avoid spurious timer interrupts
	*mtimecmp = 0xFFFFFFFFFFFFFFFF;

	//	Enable SW, timer and external interrupts
	set_csr(mie, MIP_MSIP);
	set_csr(mie, MIP_MTIP);
	set_csr(mie, MIP_MEIP);

	// 	Set default state of mstatus
	set_csr(mstatus, MSTATUS_DEFAULT);
}



/**
 * @brief Enable all maskable interrupts
 */
unsigned int irq_arch_enable(void)
{
	// Enable all interrupts
	set_csr(mstatus, MSTATUS_MIE);
	return read_csr(mstatus);
}

/**
 * @brief Disable all maskable interrupts
 */
unsigned int irq_arch_disable(void)
{
	unsigned int state = read_csr(mstatus);

	// Disable all interrupts
	clear_csr(mstatus, MSTATUS_MIE);
    return state;
}

/**
 * @brief Restore the state of the IRQ flags
 */
void irq_arch_restore(unsigned int state)
{
	// Restore all interrupts to given state
	write_csr(mstatus, state);
}

/**
 * @brief See if the current context is inside an ISR
 */
int irq_arch_in(void)
{
    return __in_isr;
}


/**
 * @brief   Set External ISR callback
 */
void set_external_isr_cb(int intNum, external_isr_ptr_t cbFunc)
{
	if ((intNum > 0 ) && (intNum < PLIC_NUM_INTERRUPTS))
	{
		_ext_isrs[intNum] = cbFunc;
	}
}


/**
 * @brief External interrupt handler
 */
void external_isr(void)
{
	plic_source intNum = PLIC_claim_interrupt();

	if ((intNum > 0 ) && (intNum < PLIC_NUM_INTERRUPTS))
	{
		_ext_isrs[intNum]((uint32_t)intNum);
	}

	PLIC_complete_interrupt(intNum);
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

				//	Perform context switch
				sched_context_switch_request = 1;

				//	Reset interrupt
				set_csr(mie, MIP_MSIP);
				break;

			case IRQ_M_TIMER:
				//	Handle timer interrupt
				timer_isr();
				break;

			case IRQ_M_EXT:
				//	Handle external interrupt
				external_isr();
				break;

			default:
				//	Unknown interrupt
				break;
		}
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
    stk[32] = (unsigned int) task_func;
    stk[33] = (unsigned int) (MSTATUS_DEFAULT | MSTATUS_MIE);

    return (char*) stk;
}

void thread_arch_stack_print(void)
{
    int count = 0;
    uint32_t *sp = (uint32_t *)sched_active_thread->sp;

    printf("printing the current stack of thread %" PRIkernel_pid "\n",
           thread_getpid());

#ifdef DEVELHELP
    printf("thread name: %s\n", sched_active_thread->name);
    printf("stack start: 0x%08x\n", (unsigned int)(sched_active_thread->stack_start));
    printf("stack end  : 0x%08x\n", (unsigned int)(sched_active_thread->stack_start + sched_active_thread->stack_size));
#endif

    printf("  address:      data:\n");

    do {
        printf("  0x%08x:   0x%08x\n", (unsigned int)sp, (unsigned int)*sp);
        sp++;
        count++;
    } while (*sp != STACK_MARKER);

    printf("current stack size: %i words\n", count);
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
    return NULL;
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
	//	Use SW intr to schedule context switch
	CLINT_REG(CLINT_MSIP) = 1;
}

