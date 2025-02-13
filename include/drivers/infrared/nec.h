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
#pragma once

#define NEC_IR_SYNC_BASE_US 9000
#define NEC_IR_SYNC_OFFSET 3
#define NEC_IR_START_OFFSET 4
#define NEC_IR_MARGIN_DIVISOR 10
#define NEC_IR_SYNC_KEYCODE_US NEC_IR_SYNC_BASE_US / 2
#define NEC_IR_SYNC_REPEAT_US NEC_IR_SYNC_BASE_US / 4
#define NEC_IR_SYNC_NOT_FOUND 0
#define NEC_IR_NUM_EDGES 68
#define NEC_IR_KEYCODE_BASE_US NEC_IR_SYNC_BASE_US / 18
#define NEC_IR_KEYCODE_WAIT_DMA_MS 140
#define NEC_IR_KEYCODE_NUM_BITS 32
#define NEC_IR_KEYCODE_NUM_EDGES NEC_IR_KEYCODE_NUM_BITS * 2
#define NEC_IR_KEYCODE_LOGIC_ZERO_US NEC_IR_KEYCODE_BASE_US
#define NEC_IR_KEYCODE_LOGIC_ONE_US NEC_IR_KEYCODE_BASE_US * 3
#define NEC_IR_KEYCODE_SHIFT_MASK 0x80000000UL
#define NEC_IR_KEYCODE_SEQUENCE_ERROR 0xffffffffUL
#define NEC_IR_KEYCODE_DATA_INTEGRITY_ERROR 0xfffffff0UL
#define NEC_IR_REPEATCODE 0xdeadbeefUL

#define necCheckBase(p_capture) \
	irGenericCheckTime(p_capture, NEC_IR_SYNC_BASE_US, NEC_IR_SYNC_BASE_US / NEC_IR_MARGIN_DIVISOR)

#define necCheckSyncKey(p_capture) \
	irGenericCheckTime(p_capture, NEC_IR_SYNC_KEYCODE_US, NEC_IR_SYNC_KEYCODE_US / NEC_IR_MARGIN_DIVISOR)

#define necCheckSyncRepeat(p_capture) \
	irGenericCheckTime(p_capture, NEC_IR_SYNC_REPEAT_US, NEC_IR_SYNC_REPEAT_US / NEC_IR_MARGIN_DIVISOR)

#define necCheckTail(p_capture) \
	irGenericCheckTime(p_capture, NEC_IR_KEYCODE_BASE_US, NEC_IR_SYNC_BASE_US / (NEC_IR_MARGIN_DIVISOR / 2))

#define necCheckKeyCodeLogicZero(p_capture) \
	irGenericCheckTime(p_capture, NEC_IR_KEYCODE_LOGIC_ZERO_US + NEC_IR_KEYCODE_BASE_US,\
			(NEC_IR_KEYCODE_LOGIC_ZERO_US + NEC_IR_KEYCODE_BASE_US) / (NEC_IR_MARGIN_DIVISOR / 2))

#define necCheckKeyCodeLogicOne(p_capture) \
	irGenericCheckTime(p_capture, NEC_IR_KEYCODE_LOGIC_ONE_US + NEC_IR_KEYCODE_BASE_US, \
			(NEC_IR_KEYCODE_LOGIC_ONE_US + NEC_IR_KEYCODE_BASE_US) / (NEC_IR_MARGIN_DIVISOR / 2))


typedef union {
	uint32_t necRaw;
	struct {
		uint16_t necAddress : 16;
		uint8_t necKey : 8;
		uint8_t necKeyInverted : 8;
	};
} necKeyCode_t;

necKeyCode_t necDecode(uint16_t *const p_capture);

