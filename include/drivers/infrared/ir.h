#pragma once

#define IR_TIMER TIM3
#define IR_MAX_EDGES 96
#define IR_EDGE_RISING 0
#define IR_EDGE_FALLING 1
#define IR_SIGNAL_PAUSE_NS 20 * 1000

typedef union {
	uint16_t irEdge;
	struct {
		uint16_t nanoSeconds	: 15;
		bool edgeType			: 1;
	};
} irEdge_t;

typedef struct {
	TickType_t irTimeStamp;
	irEdge_t irEdges[IR_MAX_EDGES];
} irCapture_t;

void setupInfrared(void);
