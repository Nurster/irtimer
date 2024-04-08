#pragma once

#define IR_TIMER TIM3
#define IR_MAX_EDGES 96
#define IR_SIGNAL_PAUSE_NS 20 * 1000

typedef enum {
	IR_EDGE_RISING,
	IR_EDGE_FALLING
} irEdgeType_t;

typedef union {
	uint16_t irEdge;
	struct {
		uint16_t irNanoSeconds	: 15;
		irEdgeType_t irEdgeType	: 1;
	};
} irEdge_t;

typedef struct {
	TickType_t irTimeStamp;
	uint8_t irEdgeCount;
	irEdge_t irEdges[IR_MAX_EDGES];
} irCapture_t;

void setupInfrared(void);
