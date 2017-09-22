/*
 * Copyright (C) 2017
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    boards_hifive
 * @ingroup     boards
 * @brief       Support for the HiFive RISC-V board
 * @{
 *
 * @file
 * @brief       Board specific definitions for the HiFive RISC-V board
 *
 * @author      Ken Rabold <kenrabold@hotmail.com>
 */

#ifndef BOARD_H
#define BOARD_H


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief   Initialize board specific hardware, including clock, LEDs and std-IO
 */
void board_init(void);


#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
/** @} */
