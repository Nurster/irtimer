#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stream_buffer.h>
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/stm32/gpio.h"
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "tasks/irtask.h"
#include "drivers/infrared/ir.h"
#include "drivers/serial/serial.h"
#include "drivers/infrared/nec.h"
#include "drivers/infrared/rc5.h"

TaskHandle_t g_irTaskHandle = NULL;
irCapture_t irCaptureIn;

void irTask(void *pvParameters __attribute__((unused))) {
/*	uint8_t debugCounter = 0; */
/*	volatile necKeyCode_t necCode; */
	volatile rc5KeyCode_t rc5Code;
	TickType_t notifyTimeStamp = 0;
	char debugOut[128];
	setupInfrared();
	while (1) {

		if (xTaskNotifyWait(0, 0, &notifyTimeStamp, pdMS_TO_TICKS(IR_NOTIFY_WAIT_MAX_MS)) == pdPASS) {
			/*
			 * necCode.necRaw = necDecode(&irCaptureIn);
			 */
			rc5Code.rc5Raw = rc5Decode(&irCaptureIn);
			memset(&irCaptureIn, 0, sizeof(irCapture_t));
		} else	{
			printStringSerial("\r\nIR nothing to do...\r\n");
		}

	}
}

void tim3_isr(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	/*
	 * causes debugger to hang frequently
	 *
	if (irCaptureIn.irTimeStamp != 0) {
		return;
	}
	*/

	if ((TIM_SR(IR_TIMER) & TIM_SR_UIF)
			|| (irCaptureIn.irEdgeCount == IR_MAX_EDGES)) {
		/*
		 * either maximum edges got captured
		 * or timer overflowed if no more edges arrived within timer period
		 *
		 * disable timer to arm for next input capture sequence
		 */

		TIM_CR1(IR_TIMER) &= ~(TIM_CR1_CEN);
		TIM_CNT(IR_TIMER) = 0x0;
		/*
		 * put capture timestamp into capture and taskNofification
		 * for later use
		 */
		irCaptureIn.irTimeStamp = xTaskGetTickCountFromISR();
		/*
		 * wake up irTask to process new data
		 */
		xTaskNotifyFromISR(g_irTaskHandle, xTaskGetTickCountFromISR(), eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		TIM_SR(IR_TIMER) &= ~(TIM_SR_UIF);
	}

	/*
	 * TIM_SR_CC3 is set to latch on rising edges
	 */
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

	/*
	 * TIM_SR_CC4 is set to latch on falling edges
	 */
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
		/*
		 * ISR was processed to slow as to clear the capture registers
		 * and new values arrived while they were sill occupied with old ones
		 *
		 * clear overcapture registers
		 */
		TIM_SR(IR_TIMER) &= ~(TIM_SR_CC3OF | TIM_SR_CC4OF);
	}
}
