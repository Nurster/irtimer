#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/stm32/gpio.h"
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "globals.h"
#include "drivers/infrared/ir.h"
#include "drivers/infrared/nec.h"
#include "drivers/serial/serial.h"
#include "tasks/irtask.h"


static bool necCheckBoundary(uint8_t *p_pos) {
	if (*p_pos >= (IR_MAX_EDGES)) {
		*p_pos = 0;
		return true;
	} else {
		return false;
	}
}

static void debugPrintCapture (const uint16_t *const p_capture, uint8_t *p_pos, char *p_debug) {

	sprintf(p_debug, "IR: debug:\t pos: \t%d\t µS: \t%hd\r\n", *p_pos, p_capture[*p_pos]);
	printStringSerial(p_debug);
}


necKeyCode_t necGetCode(uint16_t *const p_capture, uint8_t *const p_pos) {

	uint8_t remainEdges = NEC_IR_KEYCODE_NUM_EDGES;
	char debug[128];
	uint8_t debugCounter = 0;
	volatile necKeyCode_t keyCode = {
			.necRaw = 0
	};

	if (p_capture[*p_pos + 1] != 0) {
/*		debugPrintCapture(p_capture, &*p_pos, debug); */
		irGenericFindSync(p_capture, p_pos, NEC_IR_SYNC_BASE_US);
		++ *p_pos;
		necCheckBoundary(p_pos);
	} else {
		keyCode.necRaw = 0;
		return keyCode;
	}

	if (necCheckSyncRepeat(&p_capture[*p_pos])) {
		++ *p_pos;
		necCheckBoundary(p_pos);
		if (p_capture[*p_pos] == 0) {
			++ *p_pos;
			necCheckBoundary(p_pos);
/*			vTaskDelay(pdMS_TO_TICKS(NEC_IR_KEYCODE_WAIT_MS)); */
		}
		if (necCheckTail(&p_capture[*p_pos])) {
/*			debugPrintCapture(p_capture, &*p_pos, debug); */
			keyCode.necRaw = NEC_IR_REPEATCODE;

			return keyCode;
		}
	}

	if (necCheckSyncKey(&p_capture[*p_pos])) {
		++ *p_pos;
		do {
			debugCounter ++;
			necCheckBoundary(p_pos);
			if (p_capture[*p_pos] == 0) {
				/* wait for DMA to catch up
				 * */
				vTaskDelay(pdMS_TO_TICKS(NEC_IR_KEYCODE_WAIT_MS));
			}
			if (p_capture[*p_pos] < NEC_IR_EDGE_BOUNDARY
					&& necCheckTail(&p_capture[*p_pos])) {
				/*debugPrintCapture(p_capture, &*p_pos, debug);*/
				++ *p_pos;
				continue;
			}
			if (p_capture[*p_pos] > NEC_IR_EDGE_BOUNDARY
				&& necCheckKeyCodeLogicOne(&p_capture[*p_pos])) {
				keyCode.necRaw >>= 1;
				keyCode.necRaw |= NEC_IR_KEYCODE_SHIFT_MASK;
				/*debugPrintCapture(p_capture, &*p_pos, debug);*/
				++ *p_pos;
				continue;
			}
			else if (p_capture[*p_pos] > NEC_IR_EDGE_BOUNDARY
				&& necCheckKeyCodeLogicZero(&p_capture[*p_pos])) {
				keyCode.necRaw >>= 1;
				/*debugPrintCapture(p_capture, &*p_pos, debug);*/
				++ *p_pos;
				continue;
			}
			else {
				keyCode.necRaw = NEC_IR_KEYCODE_SEQUENCE_ERROR;
				/*debugPrintCapture(p_capture, &*p_pos, debug);*/
				break;
			}
		} while (remainEdges -- > 0);
	}
	return keyCode;
}
