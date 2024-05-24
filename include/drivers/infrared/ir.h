#pragma once

#define IR_TIMER TIM4
#define IR_TIMER_GPIO_BANK GPIO_BANK_TIM4
#define IR_TIMER_GPIO_PIN GPIO_TIM4_CH1
#define IR_MAX_EDGES 96
#define IR_SYNC_MARGIN_US 140
#define IR_SYNC_NOT_FOUND 255
#define IR_IDLE_THRESHOLD_US 16 * 1000
#define IR_DMA_CHANNEL_RISING
#define IR_DMA_CHANNEL_FALLING
#define IR_DMA_BURST_LENGTH 1 /* 2 transfers for both CC register values */
#define IR_DMA_BASE_ADDRESS 0xD /* offset of CCR1 */


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

void setupInfrared(uint16_t *p_buf);

bool irGenericFindSync
					(const uint16_t *const p_capture,
					uint8_t *const p_pos,
					uint16_t syncUs);

bool irGenericCheckTime
				(const uint16_t *const p_capture,
				uint16_t timeBase,
				uint16_t timeMargin);
