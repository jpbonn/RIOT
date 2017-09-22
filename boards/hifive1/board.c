/*
 * Copyright (C) 2017
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     boards_hifive HiFive
 * @{
 *
 * @file
 * @brief       Support for the HiFive RISC-V board
 *
 * @author      Ken Rabold <kenrabold@hotmail.com>
 *
 * @}
 */

#include <stdio.h>
#include <errno.h>

#include "cpu.h"
#include "board.h"
#include "sifive/encoding.h"
#include "sifive/platform.h"

void board_init(void)
{
    //	Init CPU
    cpu_init();


    // Make sure the HFROSC is on before the next line:
    PRCI_REG(PRCI_HFROSCCFG) |= ROSC_EN(1);
    // Run off 16 MHz Crystal for accuracy. Note that the
    // first line is
    PRCI_REG(PRCI_PLLCFG) = (PLL_REFSEL(1) | PLL_BYPASS(1));
    PRCI_REG(PRCI_PLLCFG) |= (PLL_SEL(1));
    // Turn off HFROSC to save power
    PRCI_REG(PRCI_HFROSCCFG) &= ~(ROSC_EN(1));

    // Configure UART to print
    GPIO_REG(GPIO_OUTPUT_VAL) |= IOF0_UART0_MASK;
    GPIO_REG(GPIO_OUTPUT_EN)  |= IOF0_UART0_MASK;
    GPIO_REG(GPIO_IOF_SEL)    &= ~IOF0_UART0_MASK;
    GPIO_REG(GPIO_IOF_EN)     |= IOF0_UART0_MASK;


	//	Initialize newlib-nano stubs
	nanostubs_init();

}
