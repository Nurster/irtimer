#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stream_buffer.h>
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/stm32/gpio.h"
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "tasks/irtask.h"
#include "drivers/infrared/ir.h"
#include "drivers/serial/serial.h"
#include "drivers/infrared/nec.h"

TaskHandle_t g_irTaskHandle = NULL;
irCapture_t irCaptureIn;
irCapture_t irCaptureOut;
irCapture_t irCaptureOutDebug[IR_DEBUG_SLOTS];
QueueHandle_t irQueueHandle;
StreamBufferHandle_t irStreamBuffer;

void irTask(void *pvParameters __attribute__((unused))) {
/*	uint8_t debugCounter = 0; */
	uint8_t keyCode = 0;
	char debugOut[128];
	uint8_t queueMessagesWaiting = 0;
	irQueueHandle = xQueueCreate(IR_QUEUE_LENGTH, sizeof(irCapture_t));
	setupInfrared();
	while (1) {

		if (xQueueReceive(irQueueHandle, &irCaptureOut, IR_QUEUE_BLOCK_MS) == pdPASS) {
/*
			queueMessagesWaiting = (uint8_t)uxQueueMessagesWaiting(irQueueHandle);

			sprintf(debugOut, "\r\nirCaptureOut.edgeCount: \t%d\r\ntimeStamp: \t%lu\r\nqueueMessagesWaiting: \t%d\r\n", irCaptureOut.irEdgeCount, (uint32_t)irCaptureOut.irTimeStamp, queueMessagesWaiting);
			printStringSerial(debugOut);

			vTaskDelay(pdMS_TO_TICKS(50));

			if (debugCounter < IR_DEBUG_SLOTS) {
				debugCounter ++;

			} else {
				debugCounter = 0;
			}
*/
			necDecode(&irCaptureOut, &keyCode);
		} else {

/*
		if (xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(IR_NOTIFY_WAIT_MAX_MS)) == pdPASS) {
			necDecode(&irCaptureIn, &keyCode);
		} else	{
*/
			printStringSerial("\r\nIR nothing to do...\r\n");
			memset(&irCaptureOut, 0xffffffff, sizeof(irCaptureOut));
/* 			debugCounter = 0; */
		}

	}
}

void tim3_isr(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint8_t clearCounter = 0;
	if ((TIM_SR(IR_TIMER) & TIM_SR_UIF)
			|| (irCaptureIn.irEdgeCount == IR_MAX_EDGES)) {
		/*
		 * either maximum edges got captured
		 * or timer overflowed if no more edges arrived within timer period
		 *
		 * disable timer to arm for next input capture sequence and
		 */
		/* reset pointer to beginning of capture array */
		TIM_CR1(IR_TIMER) &= ~(TIM_CR1_CEN);
		TIM_CNT(IR_TIMER) = 0x0;
		/* put capture timestamp into capture and taskNofification for later compare */
		irCaptureIn.irTimeStamp = xTaskGetTickCountFromISR();
		xQueueSendToBackFromISR(irQueueHandle, &irCaptureIn, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		/*memset(&irCaptureIn,0, sizeof(irCapture_t));*/
		irCaptureIn.irEdgeCount = 0;
		while (clearCounter < IR_MAX_EDGES) {
			irCaptureIn.irEdges[clearCounter].irEdge = 0x0000;
			clearCounter ++;
		}
		TIM_SR(IR_TIMER) &= ~(TIM_SR_UIF);
	}

	/* TIM_SR_CC3 is set to latch on rising edges */
	if (TIM_SR(IR_TIMER) & TIM_SR_CC3IF) {
		TIM_CR1(IR_TIMER) &= ~(TIM_CR1_CEN);
		TIM_SR(IR_TIMER) &= ~(TIM_SR_CC3IF);
		/* store time passed since last latch */
		irCaptureIn.irEdges[irCaptureIn.irEdgeCount].irEdgeType = IR_EDGE_RISING;
		irCaptureIn.irEdges[irCaptureIn.irEdgeCount].irMicroSeconds = TIM_CCR3(IR_TIMER);
		irCaptureIn.irEdgeCount ++;
		TIM_CNT(IR_TIMER) = 0x0;
		TIM_CR1(IR_TIMER) |= TIM_CR1_CEN;
	}

	/* TIM_SR_CC3 is set to latch on falling edges */
	if (TIM_SR(IR_TIMER) & TIM_SR_CC4IF) {
		TIM_CR1(IR_TIMER) &= ~(TIM_CR1_CEN);
		TIM_SR(IR_TIMER) &= ~(TIM_SR_CC4IF);
		irCaptureIn.irEdges[irCaptureIn.irEdgeCount].irEdgeType = IR_EDGE_FALLING;
		irCaptureIn.irEdges[irCaptureIn.irEdgeCount].irMicroSeconds = TIM_CCR4(IR_TIMER);
		irCaptureIn.irEdgeCount ++;
		TIM_CNT(IR_TIMER) = 0x0;
		TIM_CR1(IR_TIMER) |= TIM_CR1_CEN;
	}

	if (TIM_SR(IR_TIMER) & (TIM_SR_CC3OF | TIM_SR_CC4OF)) {
		TIM_SR(IR_TIMER) &= ~(TIM_SR_CC3OF | TIM_SR_CC4OF);
	}
}
