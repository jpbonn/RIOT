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

#include "board.h"
#include "cpu.h"

void board_init(void)
{
    /* initialize the CPU */
    cpu_init();
}
