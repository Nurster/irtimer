#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stream_buffer.h>
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/stm32/gpio.h"
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "globals.h"
#include "tasks/irtask.h"
#include "drivers/infrared/ir.h"
#include "drivers/serial/serial.h"
#include "drivers/infrared/nec.h"
#include "drivers/infrared/rc5.h"

TaskHandle_t g_irTaskHandle = NULL;

void irTask(void *pvParameters __attribute__((unused))) {
/*	uint8_t debugCounter = 0; */
	uint16_t capture[IR_MAX_EDGES];
	uint8_t pos = 0;
	volatile necKeyCode_t necCode;
	volatile rc5KeyCode_t rc5Code;

	setupInfrared(capture);
	printStringSerial("\tinfrared\r\n");
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(100));

		necCode = necGetCode(capture, &pos);
		if ((necCode.necRaw != 0)
				&& (necCode.necRaw != IR_SYNC_NOT_FOUND)) {
			xTaskNotify(g_uiTaskHandle, (uint32_t)necCode.necRaw, eSetValueWithOverwrite);
			necCode.necRaw = 0;
		}
	}
}
