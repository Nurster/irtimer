#pragma once

#define USART USART1
#define USARTGPIO GPIOA
#define USARTTX GPIO9
#define USARTRX GPIO10

#define USART_SPEED 115200
#define USART_DATABITS 8
#define USART_PARITY USART_PARITY_NONE
#define USART_STOPBITS USART_CR2_STOPBITS_1

void printStringSerial(char *string);
void setupSerial(void);
