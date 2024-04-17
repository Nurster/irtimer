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

static uint8_t necFindSyncStart(irCapture_t const* const p_capture) {

	if (p_capture == NULL) {
		return 0;
	}

	uint8_t pos = 0;

	do {
		if (necCheckSyncStart(p_capture, pos) == true) {
			return pos;
			break;
		}
	} while (pos ++ < (IR_MAX_EDGES - NEC_IR_SYNC_NUM_EDGES));

	return NEC_IR_SYNC_NOT_FOUND;
}

static uint32_t necGetKeyCode(irCapture_t const* const p_capture, uint8_t start) {
	if (p_capture == NULL) {
		return false;
	}

	uint32_t keyCode = 0;
	uint8_t bitPos = 0;
	uint8_t pos = start;

	do {
		if ((necCheckKeyCodeLogicOne(p_capture, pos) == true)
				&& (necCheckTail(p_capture, pos + 1) == true)) {
			keyCode |= (1 << bitPos);
			continue;
		}
		if ((necCheckKeyCodeLogicZero(p_capture, pos) == true)
				&& (necCheckTail(p_capture, pos + 1) == true)) {
			/*
			 * Check for timings only.
			 * No need to fill in zeros as they are
			 * already present through initialization.
			 */
			continue;
		}
		/*
		 * If we got here none of the edge timings matched
		 * and the whole sequence may be corrupted.
		 * Bail out and fill the keycode with the appropriate errorno.
		 */
		return NEC_IR_KEYCODE_SEQUENCE_ERROR;
		break;
	} while ((pos += 2) < (start + NEC_IR_KEYCODE_NUM_EDGES) \
			&& (bitPos ++ < NEC_IR_KEYCODE_NUM_BITS));
	return keyCode;
}



uint32_t necDecode(irCapture_t const* const p_capture) {

	if (p_capture == NULL) {
		return false;
	}
	uint32_t keyCode = 0;
	uint8_t pos = 0;

	pos = necFindSyncStart(p_capture);

	if (pos <= (IR_MAX_EDGES - NEC_IR_KEYCODE_NUM_EDGES)) {
		if (necCheckSyncKeyCode(p_capture, pos + 1)
				&& necCheckTail(p_capture, pos + 2)) {
				/* advance to first data position to begin parsing of keycode */
				pos += NEC_IR_KEYCODE_START_EDGE - 1;
				keyCode = necGetKeyCode(p_capture, pos);
				printStringSerial("key");
				return keyCode;
		}
	}
	if (necCheckSyncRepeat(p_capture, pos + 1)
			&& necCheckTail(p_capture, pos + 2)) {
			keyCode = NEC_IR_REPEATCODE;
			printStringSerial("repeat");
			return keyCode;
	}
	return 0;
}
