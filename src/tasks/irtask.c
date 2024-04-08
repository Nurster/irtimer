#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/stm32/gpio.h"
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "tasks/irtask.h"
#include "drivers/infrared/ir.h"

TaskHandle_t g_irTaskHandle = NULL;
irCapture_t irCapture;

void irTask(void *pvParameters __attribute__((unused))) {
	memset(&irCapture,0, sizeof(irCapture_t));
	setupInfrared();
	while (1) {
		if (xTaskNotifyWait(0, 0, NULL, portMAX_DELAY) == pdPASS) {
		    vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}
}

void tim3_isr(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if ((TIM_SR(IR_TIMER) & TIM_SR_UIF) || (irCapture.irEdgeCount == IR_MAX_EDGES)) {
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
		irCapture.irTimeStamp = xTaskGetTickCountFromISR();
		xTaskNotifyFromISR(g_irTaskHandle, irCapture.irEdgeCount, eSetValueWithoutOverwrite, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		irCapture.irEdgeCount = 0;
		TIM_SR(IR_TIMER) &= ~(TIM_SR_UIF);
	}

	/* TIM_SR_CC3 is set to latch on rising edges */
	if (TIM_SR(IR_TIMER) & TIM_SR_CC3IF) {
		TIM_CR1(IR_TIMER) &= ~(TIM_CR1_CEN);
		TIM_SR(IR_TIMER) &= ~(TIM_SR_CC3IF);
		/* store time passed since last latch */
		irCapture.irEdges[irCapture.irEdgeCount].irEdgeType = IR_EDGE_RISING;
		irCapture.irEdges[irCapture.irEdgeCount].irNanoSeconds = TIM_CCR3(IR_TIMER);
		irCapture.irEdgeCount ++;
		TIM_CNT(IR_TIMER) = 0x0;
		TIM_CR1(IR_TIMER) |= TIM_CR1_CEN;
	}

	/* TIM_SR_CC3 is set to latch on falling edges */
	if (TIM_SR(IR_TIMER) & TIM_SR_CC4IF) {
		TIM_CR1(IR_TIMER) &= ~(TIM_CR1_CEN);
		TIM_SR(IR_TIMER) &= ~(TIM_SR_CC4IF);
		irCapture.irEdges[irCapture.irEdgeCount].irEdgeType = IR_EDGE_FALLING;
		irCapture.irEdges[irCapture.irEdgeCount].irNanoSeconds = TIM_CCR4(IR_TIMER);
		irCapture.irEdgeCount ++;
		TIM_CNT(IR_TIMER) = 0x0;
		TIM_CR1(IR_TIMER) |= TIM_CR1_CEN;
	}

	if (TIM_SR(IR_TIMER) & (TIM_SR_CC3OF | TIM_SR_CC4OF)) {
		TIM_SR(IR_TIMER) &= ~(TIM_SR_CC3OF | TIM_SR_CC4OF);
	}
}
