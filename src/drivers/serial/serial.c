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
		USARTGPIO, 
		GPIO_MODE_OUTPUT_50_MHZ, 
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
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
