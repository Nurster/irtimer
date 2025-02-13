/*
 * (c) 2025 Nurster
 * https://github.com/Nurster
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * or version 3 as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <stdio.h>
#include "drivers/serial/serial.h"

void printStringSerial(char *string) {
	while (*string) {
		usart_send_blocking(USART, *string);
		string++;
	}
}

static void setupGpio(void) {
	gpio_set_mode(
	USARTGPIO, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
	USARTRX | USARTTX);
}

static void setupUsart(void) {
	usart_set_baudrate(USART, USART_SPEED);
	usart_set_databits(USART, USART_DATABITS);
	usart_set_parity(USART, USART_PARITY);
	usart_set_stopbits(USART, USART_STOPBITS);
	usart_set_mode(USART, USART_MODE_TX_RX);
	usart_set_flow_control(USART, USART_FLOWCONTROL_NONE);
	usart_enable(USART);
}

void setupSerial(void) {
	setupGpio();
	setupUsart();
}
