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

static uint8_t rc5FindSyncStart(irCapture_t const* const p_capture) {

	if (p_capture == NULL) {
		return 0;
	}

	uint8_t pos = 0;

	do {
		if ((p_capture->irEdges[pos].irEdgeType == IR_EDGE_FALLING)
				&& (p_capture->irTimeStamp > 0)) {
			vTaskDelay(1);
			return pos;
			break;
		}
	} while (pos ++ < (IR_MAX_EDGES - RC5_IR_NUM_EDGES));
	return RC5_IR_SYNC_NOT_FOUND;
}

uint16_t rc5Decode(irCapture_t const* const p_capture) {
	if (p_capture == NULL) {
		return false;
	}

	uint16_t keyCode = 1 << RC5_IR_START_BIT;
	rc5Phase_t phase = RC5_PHASE_LOGIC_ONE;
	uint8_t bitPos = 1;
	uint8_t pos = rc5FindSyncStart(p_capture) + RC5_IR_START_EDGE;

	do {
		if (phase == RC5_PHASE_LOGIC_ZERO &&
				rc5CheckLogicZero(p_capture, pos)) {
			/*
			 * no need to shift zeros since they are already present through initialization
			 * check for timings only
			 */
			bitPos ++;
			continue;
		}

		if (phase == RC5_PHASE_LOGIC_ZERO &&
				rc5CheckPhaseChangeLogicOne(p_capture, pos)){
			keyCode |= (1 << bitPos);
			bitPos ++;
			phase = RC5_PHASE_LOGIC_ONE;
			continue;
		}

		if (phase == RC5_PHASE_LOGIC_ZERO &&
				rc5CheckTailRising(p_capture, pos)) {
			continue;
		}

		if (phase == RC5_PHASE_LOGIC_ONE &&
				rc5CheckLogicOne(p_capture,pos)) {
			keyCode |= (1 << bitPos);
			bitPos ++;
			continue;
		}

		if (phase == RC5_PHASE_LOGIC_ONE &&
				rc5CheckPhaseChangeLogicZero(p_capture, pos)){

			bitPos ++;
			phase = RC5_PHASE_LOGIC_ZERO;
			continue;
		}

		if (phase == RC5_PHASE_LOGIC_ONE &&
				rc5CheckTailFalling(p_capture, pos)) {
			continue;
		}
		/*
		 * If we got here none of the edge timings matched
		 * and the whole sequence may be corrupted.
		 * Bail out and fill the keycode with the appropriate errorno.
		 */
		return RC5_IR_KEYCODE_SEQUENCE_ERROR;
		break;
	} while (pos ++ < RC5_IR_NUM_EDGES);
	vTaskDelay(1);
	return keyCode;
}
