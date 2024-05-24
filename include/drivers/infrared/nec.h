#pragma once

#define NEC_IR_SYNC_NUM_EDGES 4
#define NEC_IR_SYNC_BASE_US 9000
#define NEC_IR_SYNC_KEYCODE_US NEC_IR_SYNC_BASE_US / 2
#define NEC_IR_SYNC_REPEAT_US NEC_IR_SYNC_BASE_US / 4
#define NEC_IR_SYNC_MARGIN_US 160
#define NEC_IR_SYNC_NOT_FOUND 0

#define NEC_IR_EDGE_BOUNDARY NEC_IR_SYNC_BASE_US / 10

#define NEC_IR_KEYCODE_WAIT_MS 10
#define NEC_IR_KEYCODE_NUM_BITS 32
#define NEC_IR_KEYCODE_NUM_EDGES NEC_IR_KEYCODE_NUM_BITS * 2
#define NEC_IR_KEYCODE_START_EDGE 4
#define NEC_IR_KEYCODE_BASE_US 560
#define NEC_IR_KEYCODE_MARGIN_US NEC_IR_SYNC_MARGIN_US
#define NEC_IR_KEYCODE_LOGIC_ZERO_US NEC_IR_KEYCODE_BASE_US
#define NEC_IR_KEYCODE_LOGIC_ONE_US NEC_IR_KEYCODE_BASE_US * 3
#define NEC_IR_KEYCODE_SHIFT_MASK 0x80000000UL
#define NEC_IR_KEYCODE_SEQUENCE_ERROR 0xffffffffUL

#define NEC_IR_REPEATCODE 0xdeadbeefUL

#define necCheckSyncRepeat(p_capture) \
	irGenericCheckTime(p_capture, NEC_IR_SYNC_BASE_US + NEC_IR_SYNC_REPEAT_US, NEC_IR_SYNC_MARGIN_US * 2)

#define necCheckSyncKey(p_capture) \
	irGenericCheckTime(p_capture, NEC_IR_SYNC_BASE_US + NEC_IR_SYNC_KEYCODE_US, NEC_IR_SYNC_MARGIN_US * 2)

#define necCheckTail(p_capture) \
	irGenericCheckTime(p_capture, NEC_IR_KEYCODE_BASE_US, NEC_IR_SYNC_MARGIN_US)

#define necCheckKeyCodeLogicZero(p_capture) \
	irGenericCheckTime(p_capture, NEC_IR_KEYCODE_LOGIC_ZERO_US + NEC_IR_KEYCODE_BASE_US, NEC_IR_SYNC_MARGIN_US * 2)

#define necCheckKeyCodeLogicOne(p_capture) \
	irGenericCheckTime(p_capture, NEC_IR_KEYCODE_LOGIC_ONE_US + NEC_IR_KEYCODE_BASE_US, NEC_IR_SYNC_MARGIN_US * 2)


typedef union {
	uint32_t necRaw;
	struct {
		uint16_t necAddress 	: 16;
		uint8_t necKey 			: 8;
		uint8_t necKeyInverted 	: 8;
	};
} necKeyCode_t;

necKeyCode_t necGetCode(uint16_t *const p_capture, uint8_t *const p_pos);

