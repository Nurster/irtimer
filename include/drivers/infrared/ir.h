#pragma once

#define IR_TIMER TIM3
#define IR_QUEUE_LENGTH 5
#define IR_QUEUE_BLOCK_MS 1000
#define IR_MAX_EDGES 96
#define IR_SYNC_MARGIN_US 130
#define IR_IDLE_THRESHOLD_US 16 * 1000


typedef enum {
	IR_EDGE_RISING,
	IR_EDGE_FALLING
} irEdgeType_t;

typedef union {
	uint16_t irEdge;
	struct {
		uint16_t irMicroSeconds	: 15;
		irEdgeType_t irEdgeType	: 1;
	};
} irEdge_t;

typedef struct {
	TickType_t irTimeStamp;
	uint8_t irEdgeCount;
	irEdge_t irEdges[IR_MAX_EDGES];
} irCapture_t;

void setupInfrared(void);
bool irGenericCheckTimePassed \
				(irCapture_t const* const p_capture, \
				uint8_t pos, \
				irEdgeType_t edgeType, \
				uint16_t timeBase);
