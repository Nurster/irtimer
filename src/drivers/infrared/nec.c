#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/stm32/gpio.h"
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "drivers/infrared/ir.h"
#include "drivers/infrared/nec.h"
#include "drivers/serial/serial.h"
#include "tasks/irtask.h"

necKeyCode_t necGetCode(uint16_t *const p_capture) {

	uint8_t remainEdges = NEC_IR_KEYCODE_NUM_EDGES;
	uint8_t pos = 0;
	char debug[128];
	uint8_t debugCounter = 0;
	volatile necKeyCode_t keyCode = { .necRaw = 0 };
#ifdef NEC_IR_DEBUG
	debugPrintCapture(p_capture, &pos, debug);
#endif
	if (irGenericFindSync(p_capture, &pos, NEC_IR_SYNC_BASE_US, IR_MAX_EDGES)) {
		++pos;
	}

	if (necCheckSyncRepeat(&p_capture[pos])) {
		++pos;
		if (p_capture[pos] == 0) {
			++pos;
		}
		if (necCheckTail(&p_capture[pos])) {
			/*			debugPrintCapture(p_capture, &pos, debug); */
			keyCode.necRaw = NEC_IR_REPEATCODE;
			return keyCode;
		}
	}

	if (necCheckSyncKey(&p_capture[pos])) {
		++pos;
		do {
			debugCounter++;
			if (p_capture[pos] == 0) {
				/*
				 * wait for DMA to catch up
				 */
				vTaskDelay(pdMS_TO_TICKS(NEC_IR_KEYCODE_WAIT_DMA_MS));
			}
			if (p_capture[pos] < NEC_IR_EDGE_BOUNDARY
					&& necCheckTail(&p_capture[pos])) {
				/* debugPrintCapture(p_capture, &pos, debug); */
				++pos;
				continue;
			}
			if (p_capture[pos] > NEC_IR_EDGE_BOUNDARY
					&& necCheckKeyCodeLogicOne(&p_capture[pos])) {
				keyCode.necRaw >>= 1;
				keyCode.necRaw |= NEC_IR_KEYCODE_SHIFT_MASK;
				/*debugPrintCapture(p_capture, &pos, debug);*/
				++pos;
				continue;
			} else if (p_capture[pos] > NEC_IR_EDGE_BOUNDARY
					&& necCheckKeyCodeLogicZero(&p_capture[pos])) {
				keyCode.necRaw >>= 1;
				/*debugPrintCapture(p_capture, &pos, debug);*/
				++pos;
				continue;
			} else {
				keyCode.necRaw = NEC_IR_KEYCODE_SEQUENCE_ERROR;
				/*debugPrintCapture(p_capture, &pos, debug);*/
				break;
			}
		} while (remainEdges-- > 0);
	}
	return keyCode;
}
