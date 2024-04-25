#pragma once

#define RC5_IR_BASE_US 900
#define RC5_IR_PHASE_CHANGE_US RC5_IR_BASE_US * 2
#define RC5_IR_START_EDGE 1
#define RC5_IR_SYNC_NOT_FOUND -1
#define RC5_IR_START_BIT 0
#define RC5_IR_NUM_BITS 14
#define RC5_IR_NUM_EDGES 24
#define RC5_IR_KEYCODE_SEQUENCE_ERROR 0xffffL

#define rc5CheckLogicOne(p_capture, pos) \
	irGenericCheckEdgeTime((p_capture), (pos), IR_EDGE_FALLING, RC5_IR_BASE_US)

#define rc5CheckLogicZero(p_capture, pos) \
	irGenericCheckEdgeTime((p_capture), (pos), IR_EDGE_RISING, RC5_IR_BASE_US)

#define rc5CheckPhaseChangeLogicOne(p_capture, pos) \
	irGenericCheckEdgeTime((p_capture), (pos), IR_EDGE_FALLING, RC5_IR_PHASE_CHANGE_US)

#define rc5CheckPhaseChangeLogicZero(p_capture, pos) \
	irGenericCheckEdgeTime((p_capture), (pos), IR_EDGE_RISING, RC5_IR_PHASE_CHANGE_US)

#define rc5CheckTailRising rc5CheckLogicOne
#define rc5CheckTailFalling rc5CheckLogicZero

typedef enum {
	RC5_PHASE_LOGIC_ZERO,
	RC5_PHASE_LOGIC_ONE
} rc5Phase_t;

typedef union {
	uint16_t rc5Raw;
	struct {
		uint8_t rc5Toggle 	: 1;
		uint8_t rc5Address 	: 5;
		uint8_t rc5Key 		: 6;
	};
} rc5KeyCode_t;

uint16_t rc5Decode(const irCapture_t *const p_capture);
