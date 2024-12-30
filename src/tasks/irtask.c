#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stream_buffer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>
#include "globals.h"
#include "tasks/irtask.h"
#include "drivers/infrared/ir.h"
#include "drivers/serial/serial.h"
#include "drivers/infrared/nec.h"
#include "drivers/infrared/rc5.h"

TaskHandle_t g_irTaskHandle = NULL;

void irTask(void *pvParameters __attribute__((unused))) {
	uint32_t irDmaInterruptStatusRegister = 0;
	uint16_t capture[IR_MAX_EDGES];
	volatile necKeyCode_t necCode;
	volatile rc5KeyCode_t rc5Code;
#ifdef IR_DEBUG
	char debug[128];
#endif

	setupInfrared(capture, IR_MAX_EDGES);
	printStringSerial("\tinfrared\r\n");
	while (1) {
		if (DMA1_CNDTR4 != IR_MAX_EDGES) { /* dma started running and decrements its remaining transfers */
			gpio_clear(IR_LED_GPIO_BANK, IR_LED_GPIO_PIN);
			vTaskDelay(pdMS_TO_TICKS(RC5_IR_KEYCODE_WAIT_MS)); /* wait for the code receive to be finished */
			rc5Code = rc5Decode(capture);
			if ((rc5Code.rc5Raw != 0)
					&& (rc5Code.rc5Raw != 255)) {
				xTaskNotify(g_uiTaskHandle, (uint32_t)rc5Code.rc5Raw, eSetValueWithOverwrite);
				rc5Code.rc5Raw = 0;
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
