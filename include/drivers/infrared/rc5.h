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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#define RC5_IR_BASE_US 850
#define RC5_IR_SYNC_US RC5_IR_BASE_US
#define RC5_IR_SYNC_OFFSET 3
#define RC5_IR_START_OFFSET 2
#define RC5_IR_MARGIN_DIVISOR 10
#define RC5_IR_SYNC_NOT_FOUND 255
#define RC5_IR_START_BIT 1
#define RC5_IR_NUM_BITS 14
#define RC5_IR_MAX_EDGES 24
#define RC5_IR_SINGLE_BIT_US RC5_IR_BASE_US * 2
#define RC5_IR_SINGLE_PHASE_CHANGE_US RC5_IR_BASE_US * 3
#define RC5_IR_DUAL_PHASE_CHANGE_US RC5_IR_BASE_US * 4
#define RC5_IR_SHIFT_MASK 0x1
#define RC5_IR_PHASE_SHIFT_MASK 0x2
#define RC5_IR_KEYCODE_SEQUENCE_ERROR 0xffffL
#define RC5_IR_KEYCODE_MASK 0x3F
#define RC5_IR_KEYCODE_WAIT_MS 72


#define rc5CheckBase(p_capture) \
	irGenericCheckTime(p_capture, RC5_IR_BASE_US, RC5_IR_BASE_US / RC5_IR_MARGIN_DIVISOR)

#define rc5CheckSync(p_capture) rc5CheckBase(p_capture)

#define rc5CheckSingleBit(p_capture) \
	irGenericCheckTime(p_capture, RC5_IR_SINGLE_BIT_US, RC5_IR_SINGLE_BIT_US / RC5_IR_MARGIN_DIVISOR)

#define rc5CheckSinglePhaseChange(p_capture) \
	irGenericCheckTime(p_capture, RC5_IR_SINGLE_PHASE_CHANGE_US, RC5_IR_SINGLE_PHASE_CHANGE_US / RC5_IR_MARGIN_DIVISOR)

#define rc5CheckDualPhaseChange(p_capture) \
	irGenericCheckTime(p_capture, RC5_IR_DUAL_PHASE_CHANGE_US, RC5_IR_DUAL_PHASE_CHANGE_US / RC5_IR_MARGIN_DIVISOR)

typedef enum {
	RC5_PHASE_LOGIC_ZERO, RC5_PHASE_LOGIC_ONE
} rc5Phase_t;

typedef union {
	uint16_t rc5Raw;
	struct {
		uint8_t rc5Key :6;
		uint8_t rc5Address :5;
		uint8_t rc5Toggle :1;
		uint8_t rc5Start :2;
	};
} rc5KeyCode_t;

rc5KeyCode_t rc5Decode(uint16_t *const p_capture);
