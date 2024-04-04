#pragma once

#define IR_TIMER TIM3
#define IR_MAX_EDGES 72
#define IR_EDGE_RISING 0
#define IR_EDGE_FALLING 1

typedef union {
	uint16_t capture;
	struct {
		uint16_t nanoSeconds	: 15;
		bool edgeType			: 1;
	} field;
} irCapture_t;

void demoTask(void *pvParameters __attribute__((unused)));
void taskTwo(void *pvParameters __attribute__((unused)));
