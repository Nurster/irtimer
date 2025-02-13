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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
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
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_DMA1);
    rcc_periph_clock_enable(RCC_TIM4);
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_SPI1);
}

int main(void) {

	volatile BaseType_t createResult;
	setupClock();
	setupSerial();
	printStringSerial("Creating tasks...\r\n");
	createResult = xTaskCreate(irTask, "Infrared Parser", 800, NULL, 0, &g_irTaskHandle);
	configASSERT(createResult);
	createResult = xTaskCreate(uiTask, "User Interface", 2000, NULL, 1, &g_uiTaskHandle);
	configASSERT(createResult);
	createResult = xTaskCreate(wdTask, "Watchdog", 200, NULL, 2, &g_wdTaskHandle);
	configASSERT(createResult);
	vTaskStartScheduler();

	return 0;
}

