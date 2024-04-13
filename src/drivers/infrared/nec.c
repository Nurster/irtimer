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

static bool necGetKeyCode(irCapture_t const* const p_capture, uint8_t pos) {
	return true;
}

bool necDecode(irCapture_t const* const p_capture, uint8_t *const p_keyCode) {

	if (p_capture == NULL || p_keyCode == NULL) {
		return false;
	}
	uint8_t pos = 0;

	pos = necFindSyncStart(p_capture);

	if (pos <= (IR_MAX_EDGES - NEC_IR_KEYCODE_NUM_EDGES)) {
		if (necCheckSyncKeyCode(p_capture, pos + 1)
				&& necCheckTail(p_capture, pos + 2)) {
				pos += 3;
				necGetKeyCode(p_capture, pos);
		}
	} else {
		if (necCheckSyncRepeat(p_capture, pos + 1)
				&& necCheckTail(p_capture, pos + 2)) {
				pos += 3;
				printf("repeat");
		}
	}

	return false;
}
