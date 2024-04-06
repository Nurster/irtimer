#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/stm32/gpio.h"
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "tasks/uitask.h"

TaskHandle_t g_uiTaskHandle = NULL;

void uiTask(void *pvParameters __attribute__((unused))) {
	while (2) {
	    vTaskDelay(pdMS_TO_TICKS(20));
	}
}
