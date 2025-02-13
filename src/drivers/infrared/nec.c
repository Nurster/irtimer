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
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/stm32/gpio.h"
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "drivers/infrared/ir.h"
#include "drivers/infrared/nec.h"
#include "drivers/serial/serial.h"
#include "tasks/irtask.h"
/*#define NEC_IR_DEBUG*/

static uint8_t necFindSync(uint16_t *const p_capture) {

	uint8_t pos = 0;

	if (p_capture == NULL) {
		return 0;
	}

	do {
		if (necCheckBase(&p_capture[pos])) {
			return pos;
		}
	} while (pos ++ < (IR_MAX_EDGES - NEC_IR_NUM_EDGES));
	return NEC_IR_SYNC_NOT_FOUND;
}

necKeyCode_t necDecode(uint16_t *const p_capture) {

	uint8_t remainEdges = NEC_IR_KEYCODE_NUM_EDGES;
	uint8_t pos = necFindSync(p_capture);
	uint16_t check = 0;
	char debug[128];
	volatile necKeyCode_t keyCode = { .necRaw = 0 };

	pos += NEC_IR_SYNC_OFFSET;
	if (necCheckSyncRepeat(&p_capture[pos])) {
		if (necCheckTail(&p_capture[pos - 1])) {
			keyCode.necRaw = NEC_IR_REPEATCODE;
			return keyCode;
		}
	}
	if (necCheckSyncKey(&p_capture[pos])) {
		/* step out of sync area */
		pos ++;
		do {
#ifdef NEC_IR_DEBUG
			debugPrintCapture(p_capture, &pos, debug);
#endif
			check = p_capture[pos] + p_capture[pos + 1];
			if (necCheckKeyCodeLogicOne(&check)) {
				keyCode.necRaw >>= 1;
				keyCode.necRaw |= NEC_IR_KEYCODE_SHIFT_MASK;
				continue;
			} else if ( necCheckKeyCodeLogicZero(&check)) {
				keyCode.necRaw >>= 1;
				continue;
			} else {
				keyCode.necRaw = NEC_IR_KEYCODE_SEQUENCE_ERROR;
				break;
			}
		} while (((remainEdges -= 2) > 0) && ((pos += 2) < (IR_MAX_EDGES)));
	}
	if ((keyCode.necKey ^ keyCode.necKeyInverted) == 0) {
		keyCode.necRaw = NEC_IR_KEYCODE_DATA_INTEGRITY_ERROR;
	}
	return keyCode;
}
