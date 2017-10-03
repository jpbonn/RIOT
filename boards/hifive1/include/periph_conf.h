/*
 * Copyright (C) 2017
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */


/**
 * @defgroup    boards_hifive HiFive
 * @ingroup     boards
 * @brief       Support for the HiFive RISC-V board
 * @{
 *
 * @file
 * @brief       Peripheral specific definitions for the HiFive RISC-V board
 *
 * @author      Ken Rabold <kenrabold@hotmail.com>
 */

#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Xtimer configuration
 * @{
 */
#define XTIMER_DEV                  (0)
#define XTIMER_CHAN                 (0)
#define XTIMER_WIDTH                (32)
#define XTIMER_HZ					(32768ul)
/** @} */


/**
 * @name    Timer configuration
 *
 * @{
 */
#define TIMER_NUMOF         		(1)

/**
 * @name    UART configuration
 *
 * @{
 */
#define UART_NUMOF          		(2)
#define UART0_RX_INTR_PRIORITY		(2)
#define UART1_RX_INTR_PRIORITY		(2)
/** @} */



#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
/** @} */
