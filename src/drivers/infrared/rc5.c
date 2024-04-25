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

static uint8_t rc5FindSyncStart(const irCapture_t *const p_capture) {

	uint8_t pos = 0;

	if (p_capture == NULL) {
		return 0;
	}

	do {
		if ((p_capture->irEdges[pos].irEdgeType == IR_EDGE_FALLING)
				&& (p_capture->irTimeStamp > 0)) {
			return pos;
		}
	} while (pos ++ < (IR_MAX_EDGES - RC5_IR_NUM_EDGES));
	/* no falling edge found whatsoever */
	return RC5_IR_SYNC_NOT_FOUND;
}

uint16_t rc5Decode(const irCapture_t *const p_capture) {

	uint16_t keyCode = 1 << RC5_IR_START_BIT;
	rc5Phase_t phase = RC5_PHASE_LOGIC_ONE;
	uint8_t bitPos = RC5_IR_START_BIT + 1; /* begin from 1 since start bit at position 0 is always 1 */
	uint8_t start = rc5FindSyncStart(p_capture); /* step through array to find first falling edge */
	uint8_t pos = start + RC5_IR_START_EDGE;

	if (p_capture == NULL) {
		return 0;
	}

	do {
		if (phase == RC5_PHASE_LOGIC_ZERO) {
			if (rc5CheckLogicZero(p_capture, pos)) {
				/*
				 * no need to shift in zeros since they are
				 * already present through initialization
				 * check for timings only
				 */
				bitPos ++;
				continue;
			}

			if (rc5CheckPhaseChangeLogicOne(p_capture, pos)){
				keyCode |= (1 << bitPos);
				bitPos ++;
				phase = RC5_PHASE_LOGIC_ONE;
				continue;
			}

			if (rc5CheckTailRising(p_capture, pos)) {
				continue;
			}
		}

		if (phase == RC5_PHASE_LOGIC_ONE) {
			if (rc5CheckLogicOne(p_capture,pos)) {
				keyCode |= (1 << bitPos);
				bitPos ++;
				continue;
			}

			if (rc5CheckPhaseChangeLogicZero(p_capture, pos)){
				/*
				 * not shifting zeros
				 */
				bitPos ++;
				phase = RC5_PHASE_LOGIC_ZERO;
				continue;
			}

			if (rc5CheckTailFalling(p_capture, pos)) {
				continue;
			}
		}
		/*
		 * If we got here none of the edge timings matched
		 * and the sequence ended or may be corrupted.
		 */
		break;
	} while (pos ++ < (start + (p_capture->irEdgeCount - 1)));
	return keyCode;
}
