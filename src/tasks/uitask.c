#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>
#include "drivers/serial/serial.h"
#include "globals.h"
#include "tasks/uitask.h"
#include "drivers/display/display.h"
#include "drivers/infrared/ir.h"
#include "drivers/infrared/nec.h"
#include "drivers/infrared/rc5.h"
#undef UI_DEBUG

TaskHandle_t g_uiTaskHandle = NULL;
QueueHandle_t g_uiQueueHandle = NULL;

void uiTask(void *pvParameters __attribute__((unused))) {

	necKeyCode_t necCode;
	rc5KeyCode_t rc5Code;
	bool dmaLock = false;
	char out[DEBUG_OUTPUT_CHAR_SIZE];
	char *taskName = pcTaskGetName(xTaskGetCurrentTaskHandle());
	volatile uint32_t color = 0x0;
	displayBuffer_t buf = {
			.p_buffer = &color,
			/*
			.startx = 0,
			.starty = 0,
			.width = DISPLAY_MEMORY_HEIGHT,
			.height = DISPLAY_MEMORY_WIDTH,
			.offsetx = DISPLAY_MEMORY_OFFSET_X,
			.offsety = DISPLAY_MEMORY_OFFSET_Y,
			.single = true
			*/
	};

	sprintf(out, "\t%s\r\n", taskName);
	printStringSerial(out);

	g_uiQueueHandle = xQueueCreate(UI_QUEUE_SIZE, sizeof(displayBuffer_t));
	vQueueAddToRegistry(g_uiQueueHandle, "UI SPI DMA Queue");
	configASSERT(g_uiQueueHandle == NULL);

	setupDisplay();
	clearScreen(&color);
	while (1) {
		/* Todo decide whether a mutex or a variable is useful here */
		switch (dmaLock) {
		case false:
			if (xQueueReceive(g_uiQueueHandle, &buf, pdMS_TO_TICKS(portMAX_DELAY)) == pdPASS) {
				dmaLock = true;
				sendBuffer(&buf);
#ifdef UI_DEBUG
				snprintf(out, sizeof(out), "%lu\t%s: found item in queue, remaining: %lu of %lu\r\n",
						xTaskGetTickCount(),
						taskName,
						uxQueueMessagesWaiting(g_uiQueueHandle),
						uxQueueGetQueueLength(g_uiQueueHandle));
				printStringSerial(out);
#endif
			}
			break;
		case true:
			if (xTaskNotifyWaitIndexed(UI_INDEX_DISPLAY_DMA_TRANSFER_COMPLETE, 0, 0, 0, pdMS_TO_TICKS(UI_DMA_TIMEOUT_MS)) == pdPASS) {
				finishSpiDma();
				dmaLock = false;
			} else {
				snprintf(out, sizeof(out), "%lu\t%s: DMA transfer not complete after %d ms!\r\n", xTaskGetTickCount(), taskName, UI_DMA_TIMEOUT_MS);
				printStringSerial(out);
			}
		}
	}
}
