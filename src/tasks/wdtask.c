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
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stream_buffer.h>
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/stm32/gpio.h"
#include <libopencm3/stm32/iwdg.h>
#include <libopencm3/cm3/nvic.h>
#include "tasks/wdtask.h"
#include "drivers/serial/serial.h"


TaskHandle_t g_wdTaskHandle;

static void setupIndependentWatchdog(void) {
	IWDG_KR = IWDG_KR_UNLOCK; /* enable write access to prescale and reload register */
	IWDG_PR = WD_PRESCALER_BITS;
	IWDG_RLR = WD_PERIOD;
	IWDG_KR = IWDG_KR_RESET;
	IWDG_KR = IWDG_KR_START;
}

void wdTask(void *pvParameters __attribute__((unused))) {
	char *taskName = pcTaskGetName(xTaskGetCurrentTaskHandle());
	char out[128];
	setupIndependentWatchdog();
	sprintf(out, "\t%s\r\n", taskName);
	printStringSerial(out);
	while (1) {
		IWDG_KR = IWDG_KR_RESET;
		vTaskDelay(pdMS_TO_TICKS(WD_RESET_INTERVAL_MS));
	}
}

