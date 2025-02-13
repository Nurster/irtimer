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
#define IR_DEBUG
#define NEC_IR_DECODE
TaskHandle_t g_irTaskHandle = NULL;

void irTask(void *pvParameters __attribute__((unused))) {

	uint32_t irDmaInterruptStatusRegister = 0;
	uint16_t capture[IR_MAX_EDGES];
	char *taskName = pcTaskGetName(xTaskGetCurrentTaskHandle());
	char out[DEBUG_OUTPUT_CHAR_SIZE];
	volatile necKeyCode_t necCode;
	volatile rc5KeyCode_t rc5Code;
	volatile uint32_t color = 0xFF;

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
	snprintf(out, sizeof(out), "\t%s\r\n", taskName);
	printStringSerial(out);

	while (1) {
		/* dma started running and decrements its remaining transfers */
		if (DMA_CNDTR(IR_DMA, IR_DMA_CHANNEL) < IR_MAX_EDGES) {
			gpio_clear(IR_LED_GPIO_BANK, IR_LED_GPIO_PIN);
			/* wait for the code receive to be advanced further */
			vTaskDelay(pdMS_TO_TICKS(NEC_IR_KEYCODE_WAIT_DMA_MS));
#ifdef NEC_IR_DECODE
			necCode = necDecode(capture);
			if ((necCode.necRaw != 0) && (necCode.necRaw != 255)) {
				snprintf(out, sizeof(out), "%lu\t%s: NEC key data received: %d\r\n", xTaskGetTickCount(), taskName, necCode.necKey);
				printStringSerial(out);

				necCode.necRaw = 0;
				color = (color << 1) | (color >> 15);
				sendBufferToQueue(&buf);

			}
#endif
#ifdef IR_DEBUG_PRINT
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
