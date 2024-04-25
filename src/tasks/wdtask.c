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

TaskHandle_t g_wdTaskHandle;

static void setupIndependentWatchdog(void) {
	IWDG_KR = IWDG_KR_UNLOCK; /* enable write access to prescale and reload register */
	IWDG_PR = WD_PRESCALER_BITS;
	IWDG_RLR = WD_PERIOD;
	IWDG_KR = IWDG_KR_RESET;
	IWDG_KR = IWDG_KR_START;
}

void wdTask(void *pvParameters __attribute__((unused))) {
	setupIndependentWatchdog();
	while (1) {
		IWDG_KR = IWDG_KR_RESET;
		vTaskDelay(pdMS_TO_TICKS(WD_RESET_INTERVAL_MS));
	}
}

