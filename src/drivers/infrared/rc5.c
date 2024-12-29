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
#include "drivers/infrared/rc5.h"
#include "drivers/serial/serial.h"
#include "tasks/irtask.h"
/*#define RC5_IR_DEBUG*/

static uint8_t rc5FindSyncStart(uint16_t *const p_capture) {

	uint8_t pos = 0;

	if (p_capture == NULL) {
		return 0;
	}

	do {
		if (rc5CheckBase(&p_capture[pos])) {
			 if (rc5CheckSync(&p_capture[pos + 1])) {
					return ++ pos;
			 }
		}
	} while (pos ++ < (IR_MAX_EDGES - RC5_IR_MAX_EDGES));
	/* no falling edge found whatsoever */
	return RC5_IR_SYNC_NOT_FOUND;
}

rc5KeyCode_t rc5Decode(uint16_t *const p_capture) {

	rc5Phase_t phase = RC5_PHASE_LOGIC_ONE; /* all falling edges are interpreted as logic ones from the start */
	uint8_t shiftCount = 0;
	uint8_t pos = rc5FindSyncStart(p_capture) + RC5_IR_START_OFFSET;  /* step through array to find first sync pair */
#ifdef RC5_IR_DEBUG
	char debug[128];
	uint8_t debugCounter = 0;
#endif

	volatile rc5KeyCode_t keyCode = {
			.rc5Raw = 0
	};
	if (p_capture == NULL || pos == 0) {
		return keyCode;
	} else {
		keyCode.rc5Raw = (1 << 0); /* first Bit is always logic one */
		shiftCount = 1;
	}

	do {
#ifdef RC5_IR_DEBUG
		debugPrintCapture(p_capture, &pos, debug);
#endif
		if (phase == RC5_PHASE_LOGIC_ONE) {
			if (rc5CheckSingleBit(&p_capture[pos])) {
				keyCode.rc5Raw = (keyCode.rc5Raw << 1) | RC5_IR_SHIFT_MASK;
				shiftCount ++;
				continue;
			}

			if (rc5CheckSinglePhaseChange(&p_capture[pos])){
				keyCode.rc5Raw = (keyCode.rc5Raw << 2) | RC5_IR_PHASE_SHIFT_MASK;
				shiftCount += 2;
				phase = RC5_PHASE_LOGIC_ZERO;
				continue;
			}

			if (rc5CheckDualPhaseChange(&p_capture[pos])) {
				keyCode.rc5Raw = (keyCode.rc5Raw << 2) | RC5_IR_PHASE_SHIFT_MASK;
				shiftCount += 2;
				continue;
			}
			if (rc5CheckLastSingleBit(&p_capture[pos - 1])) {
				keyCode.rc5Raw = (keyCode.rc5Raw << 1) | RC5_IR_SHIFT_MASK;
				shiftCount ++;
				continue;
			}
			if (rc5CheckLastPhaseChange(&p_capture[pos - 1])) {
				keyCode.rc5Raw = (keyCode.rc5Raw << 2) | RC5_IR_PHASE_SHIFT_MASK;
				shiftCount += 2;
				continue;
			}
		}

		if (phase == RC5_PHASE_LOGIC_ZERO) {

			if (rc5CheckSingleBit(&p_capture[pos])) {
				keyCode.rc5Raw = (keyCode.rc5Raw << 1);
				shiftCount ++;
				continue;
			}

			if (rc5CheckSinglePhaseChange(&p_capture[pos])){
				keyCode.rc5Raw = (keyCode.rc5Raw << 1);
				shiftCount ++;
				phase = RC5_PHASE_LOGIC_ONE;
				continue;
			}

			if (rc5CheckDualPhaseChange(&p_capture[pos])) { /* appears never to occur */
				keyCode.rc5Raw = (keyCode.rc5Raw << 2) | RC5_IR_PHASE_SHIFT_MASK;
				shiftCount += 2;
				continue;
			}

			if (rc5CheckLastSingleBit(&p_capture[pos - 1])) {
				keyCode.rc5Raw = (keyCode.rc5Raw << 1);
				shiftCount ++;
				continue;
			}
			if (rc5CheckLastPhaseChange(&p_capture[pos - 1])) {
				keyCode.rc5Raw = (keyCode.rc5Raw << 1) | RC5_IR_PHASE_SHIFT_MASK;
				shiftCount ++;
				continue;
			}
		}
		/* none of the timings matched so we're out */
		keyCode.rc5Raw = RC5_IR_KEYCODE_SEQUENCE_ERROR;
		return keyCode;
	} while ((shiftCount < (RC5_IR_NUM_BITS))
			&& ((pos += 2) < (IR_MAX_EDGES))); /* 	only get the second timer register since
													it contains the sum of first plus its own */

	if ((pos > IR_MAX_EDGES) || (shiftCount != RC5_IR_NUM_BITS)) {
		keyCode.rc5Raw = RC5_IR_KEYCODE_SEQUENCE_ERROR;
		return keyCode;
	}

	/* stretch into 16 Bit while maintaining keycode at right most position */
	keyCode.rc5Raw = ((keyCode.rc5Raw & ~(RC5_IR_KEYCODE_MASK)) << 2) | (keyCode.rc5Raw & RC5_IR_KEYCODE_MASK);
	return keyCode;
}
