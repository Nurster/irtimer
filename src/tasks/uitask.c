#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "drivers/serial/serial.h"
#include "globals.h"
#include "tasks/uitask.h"
#include "drivers/infrared/ir.h"
#include "drivers/infrared/nec.h"
TaskHandle_t g_uiTaskHandle = NULL;
necKeyCode_t necCode;
char keyOut[128];

void uiTask(void *pvParameters __attribute__((unused))) {
	while (2) {
		if (xTaskNotifyWait(0, 0, &necCode.necRaw, pdMS_TO_TICKS(1000)) == pdPASS) {
			sprintf(keyOut, "\r\nUI:\tKeyCode received: %d\r\n", necCode.necKey);
			printStringSerial(keyOut);
			necCode.necRaw = 0;
		} else	{
		/*printStringSerial("\r\nUI:\tNothing to do...\r\n");*/
		}
	}
}
