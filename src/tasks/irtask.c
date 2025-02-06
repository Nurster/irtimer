#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>
#include "globals.h"
#include "tasks/irtask.h"
#include "drivers/serial/serial.h"
#include "drivers/infrared/ir.h"
#include "drivers/infrared/nec.h"
#include "drivers/infrared/rc5.h"
#include "drivers/display/display.h"
#include "drivers/display/st7789.h"

TaskHandle_t g_irTaskHandle = NULL;

void irTask(void *pvParameters __attribute__((unused))) {

	uint32_t irDmaInterruptStatusRegister = 0;
	uint16_t capture[IR_MAX_EDGES];
	char *taskName = pcTaskGetName(xTaskGetCurrentTaskHandle());
	char out[DEBUG_OUTPUT_CHAR_SIZE];
	volatile necKeyCode_t necCode;
	volatile rc5KeyCode_t rc5Code;
	volatile uint32_t color = 0xF;

	volatile pixel12Bit_t pixel = {
			.redFirst = 0xa,
			.greenFirst = 0xb,
			.blueFirst = 0xc,
			.redSecond = 0xd,
			.greenSecond = 0xe,
			.blueSecond = 0xf
	};
/*
	volatile pixel18Bit_t pixel = {
			.red = 0x1a,
			.green = 0x3b,
			.blue = 0x1c
	};
*/
	displayBuffer_t buf = {
			.p_buffer = (uint32_t*)&color,
			.startx = 0,
			.starty = 0,
			.width = DISPLAY_WIDTH,
			.height = DISPLAY_HEIGHT,
			.offsetx = DISPLAY_MEMORY_OFFSET_X,
			.offsety = DISPLAY_MEMORY_OFFSET_Y,
			.single = true
	};
#ifdef IR_DEBUG
	char debug[DEBUG_OUTPUT_CHAR_SIZE];
#endif

	setupInfrared(capture, IR_MAX_EDGES);
	sprintf(out, "\t%s\r\n", taskName);
	printStringSerial(out);

	while (1) {
		if (!(DMA_CNDTR(IR_DMA, IR_DMA_CHANNEL) & IR_MAX_EDGES)) { /* dma started running and decrements its remaining transfers */
			gpio_clear(IR_LED_GPIO_BANK, IR_LED_GPIO_PIN);
			vTaskDelay(pdMS_TO_TICKS(RC5_IR_KEYCODE_WAIT_MS)); /* wait for the code receive to be finished */
			rc5Code = rc5Decode(capture);
			if ((rc5Code.rc5Raw != 0) && (rc5Code.rc5Raw != 255)) {
				sprintf(out, "%lu\t%s: key data received: %d\r\n", xTaskGetTickCount(), taskName, rc5Code.rc5Key);
				printStringSerial(out);
				color = (color << 1) | (color >> 15);
				rc5Code.rc5Raw = 0;
				sendBufferToQueue(&buf);
			}
#ifdef IR_DEBUG
			for (uint8_t i = 0; i < IR_MAX_EDGES; i ++) {
				debugPrintCapture(capture, &i, debug);
			}
#endif
			memset(capture, 0, IR_MAX_EDGES * sizeof(*capture));
			irResetDmaCounter(IR_MAX_EDGES);
			gpio_set(IR_LED_GPIO_BANK, IR_LED_GPIO_PIN);

			/*debugPrintCapture(capture, 0, debug);*/
		}
	}
}
