/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
 * Copyright (C) 2012 Karl Palsson <karlp@tweak.net.au>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "include/globals.h"
#include "main.h"
#include "drivers/serial/serial.h"
#include "tasks/wdtask.h"
#include "tasks/uitask.h"
#include "tasks/irtask.h"


static void setupClock(void) {
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_DMA1);
    rcc_periph_clock_enable(RCC_TIM4);
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_USART2);

}

int main(void) {

	volatile BaseType_t createResult;
	setupClock();
	setupSerial();
	printStringSerial("Creating tasks...\r\n");
	createResult = xTaskCreate(wdTask, "Watchdog", 200, NULL, 0, &g_wdTaskHandle);
	createResult = xTaskCreate(uiTask, "User Interface", 400, NULL, 0, &g_uiTaskHandle);
	createResult = xTaskCreate(irTask, "Infrared Parser", 800, NULL, 0, &g_irTaskHandle);
	vTaskStartScheduler();

	return 0;
}

