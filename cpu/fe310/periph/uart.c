/*
 * Copyright 2017
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_fe310
 * @{
 *
 * @file        timer.c
 * @brief       Low-level timer implementation
 *
 * @author
 * @}
 */

#include <stdlib.h>
#include <unistd.h>

#include "cpu.h"
#include "periph_cpu.h"
#include "periph_conf.h"
#include "periph/uart.h"
#include "sifive/encoding.h"
#include "sifive/platform.h"

/**
 * @brief   Allocate memory to store the callback functions
 */
static uart_isr_ctx_t isr_ctx[UART_NUMOF];


int uart_init(uart_t dev, uint32_t baudrate, uart_rx_cb_t rx_cb, void *arg)
{
    //	Check for valid UART dev
    if (dev >= UART_NUMOF) {
        return UART_NODEV;
    }

    //	Save interrupt callback context
    isr_ctx[dev].rx_cb = rx_cb;
    isr_ctx[dev].arg = arg;

    //	Power on the device
    uart_poweron(dev);

    //	Enable UART 8-N-1 at given baudrate
    if(dev == 0)
    {
		// 115200 Baud Rate at 16MHz clk
		UART0_REG(UART_REG_DIV) = 138;
		UART0_REG(UART_REG_TXCTRL) = UART_TXEN;

		//	Enable RX is there is a callback
		if(rx_cb != NULL)
			UART0_REG(UART_REG_RXCTRL) = UART_RXEN;
    }

    if(dev == 1)
    {
		// 115200 Baud Rate at 16MHz clk
		UART1_REG(UART_REG_DIV) = 138;
		UART1_REG(UART_REG_TXCTRL) = UART_TXEN;

		//	Enable RX is there is a callback
		if(rx_cb != NULL)
			UART1_REG(UART_REG_RXCTRL) = UART_RXEN;
    }

    return UART_OK;
}

void uart_write(uart_t dev, const uint8_t *data, size_t len)
{
	if(dev == 0)
	{
		for (size_t i = 0; i < len; i++)
		{
			//	Wait for FIFO to empty
			while ((UART0_REG(UART_REG_TXFIFO) & UART_TXFIFO_FULL) == UART_TXFIFO_FULL);

			//	Write a byte
			UART0_REG(UART_REG_TXFIFO) = data[i];
		}
	}

	if(dev == 1)
	{
		for (size_t i = 0; i < len; i++)
		{
			//	Wait for FIFO to empty
			while ((UART1_REG(UART_REG_TXFIFO) & UART_TXFIFO_FULL) == UART_TXFIFO_FULL);

			//	Write a byte
			UART1_REG(UART_REG_TXFIFO) = data[i];
		}
	}
}

void uart_poweron(uart_t dev)
{
}

void uart_poweroff(uart_t dev)
{
}


