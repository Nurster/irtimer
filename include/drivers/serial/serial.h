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
