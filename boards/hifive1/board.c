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
#include "sifive/prci_driver.h"

void board_init(void)
{
    //	Init CPU
    cpu_init();

    //	Configure clocks
    //PRCI_use_default_clocks();
    PRCI_use_bypass_clock();

    //	Configure pin muxing for UART0
    GPIO_REG(GPIO_OUTPUT_VAL) |= IOF0_UART0_MASK;
    GPIO_REG(GPIO_OUTPUT_EN)  |= IOF0_UART0_MASK;
    GPIO_REG(GPIO_IOF_SEL)    &= ~IOF0_UART0_MASK;
    GPIO_REG(GPIO_IOF_EN)     |= IOF0_UART0_MASK;


	//	Initialize newlib-nano stubs
	nanostubs_init();

}
