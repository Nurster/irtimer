#pragma once

#define NEC_IR_SYNC_NUM_EDGES 4
#define NEC_IR_SYNC_BASE_US 9000
#define NEC_IR_SYNC_KEYCODE_US NEC_IR_SYNC_BASE_US / 2
#define NEC_IR_SYNC_REPEAT_US NEC_IR_SYNC_BASE_US / 4
#define NEC_IR_SYNC_MARGIN_US 100
#define NEC_IR_SYNC_NOT_FOUND 0

#define NEC_IR_KEYCODE_NUM_BITS 32
#define NEC_IR_KEYCODE_NUM_EDGES NEC_IR_KEYCODE_NUM_BITS * 2
#define NEC_IR_KEYCODE_START_EDGE 4
#define NEC_IR_KEYCODE_BASE_US 560
#define NEC_IR_KEYCODE_MARGIN_US NEC_IR_SYNC_MARGIN_US
#define NEC_IR_KEYCODE_LOGIC_ZERO_US NEC_IR_KEYCODE_BASE_US
#define NEC_IR_KEYCODE_LOGIC_ONE_US NEC_IR_KEYCODE_BASE_US * 3
#define NEC_IR_KEYCODE_SEQUENCE_ERROR 0xffffffff

#define NEC_IR_REPEATCODE 255

#define necCheckSyncStart(p_capture, pos) \
	irGenericCheckEdgeTime((p_capture), (pos), IR_EDGE_RISING, NEC_IR_SYNC_BASE_US)

#define necCheckSyncKeyCode(p_capture, pos) \
	irGenericCheckEdgeTime((p_capture), (pos), IR_EDGE_FALLING, NEC_IR_SYNC_KEYCODE_US)

#define necCheckSyncRepeat(p_capture, pos) \
	irGenericCheckEdgeTime((p_capture), (pos), IR_EDGE_FALLING, NEC_IR_SYNC_REPEAT_US)

#define necCheckTail(p_capture, pos) \
	irGenericCheckEdgeTime((p_capture), (pos), IR_EDGE_RISING, NEC_IR_KEYCODE_BASE_US)

#define necCheckKeyCodeLogicZero(p_capture, pos) \
	irGenericCheckEdgeTime((p_capture), (pos), IR_EDGE_FALLING, NEC_IR_KEYCODE_LOGIC_ZERO_US)

#define necCheckKeyCodeLogicOne(p_capture, pos) \
	irGenericCheckEdgeTime((p_capture), (pos), IR_EDGE_FALLING, NEC_IR_KEYCODE_LOGIC_ONE_US)


uint32_t necDecode(irCapture_t const* const p_capture);
