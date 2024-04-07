#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/stm32/gpio.h"
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "globals.h"
#include "drivers/infrared/ir.h"
#include "tasks/irtask.h"

static irCapture_t irCapture;
static uint8_t irCaptureCounter = 0;

void setupInfrared(void) {
	memset(&irCapture,0, sizeof(irCapture_t));

	nvic_enable_irq(NVIC_TIM3_IRQ);
	/*nvic_set_priority(NVIC_TIM3_IRQ, 128);*/
	rcc_periph_reset_pulse(RST_TIM3);
	timer_set_mode(
	  IR_TIMER,
	  TIM_CR1_CKD_CK_INT,
	  TIM_CR1_CMS_EDGE,
	  TIM_CR1_DIR_UP
  );

  /* count microseconds */
  timer_set_prescaler(IR_TIMER, ((rcc_apb2_frequency) / 1000000));
  /*
   * either continuous or one sho8934t mode does not make a difference,
   * since the timer is switched on and of in the ISR
   */
  timer_continuous_mode(IR_TIMER);

  TIM_CCMR2(IR_TIMER)
  	  |= TIM_CCMR2_IC3F_CK_INT_N_8
	  | TIM_CCMR2_IC4F_CK_INT_N_8
	  | TIM_CCMR2_IC3PSC_OFF
	  | TIM_CCMR2_IC4PSC_OFF
	  | TIM_CCMR2_CC3S_IN_TI4
	  | TIM_CCMR2_CC4S_IN_TI4
	  ;

  TIM_CCER(IR_TIMER)
  	  |= TIM_CCER_CC3E
	  | TIM_CCER_CC4E
	  | TIM_CCER_CC4P
	  ;

  /* update registers before enabling interupts to prevent false trigger */
  TIM_EGR(IR_TIMER) |= TIM_EGR_UG;
  TIM_SR(IR_TIMER) &= ~(TIM_SR_UIF);

 /* finally enable irqs to arm timer for capturing */
  TIM_DIER(IR_TIMER)
  	  |= TIM_DIER_UIE
	  | TIM_DIER_CC3IE
	  | TIM_DIER_CC4IE
	  ;
}

void tim3_isr(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if ((TIM_SR(IR_TIMER) & TIM_SR_UIF) || (irCaptureCounter == IR_MAX_EDGES)) {
		/*
		 * either maximum edges got captured
		 * or timer overflowed if no more edges arrived within timer period
		 *
		 * disable timer to arm for next input capture sequence and
		 */
		/* reset pointer to beginning of capture array */
		TIM_CR1(IR_TIMER) &= ~(TIM_CR1_CEN);
		TIM_CNT(IR_TIMER) = 0x0;
		irCaptureCounter = 0;
		TIM_SR(IR_TIMER) &= ~(TIM_SR_UIF);
		irCapture.irTimeStamp = xTaskGetTickCountFromISR();
		xTaskNotifyFromISR(g_irTaskHandle, irCapture.irTimeStamp, eSetValueWithoutOverwrite, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

	/* TIM_SR_CC3 is set to latch on rising edges */
	if (TIM_SR(IR_TIMER) & TIM_SR_CC3IF) {
		TIM_CR1(IR_TIMER) &= ~(TIM_CR1_CEN);
		TIM_SR(IR_TIMER) &= ~(TIM_SR_CC3IF);
		/* store time passed since last latch */
		irCapture.irEdges[irCaptureCounter].edgeType = IR_EDGE_RISING;
		irCapture.irEdges[irCaptureCounter].nanoSeconds = TIM_CCR3(IR_TIMER);
		irCaptureCounter ++;
		TIM_CNT(IR_TIMER) = 0x0;
		TIM_CR1(IR_TIMER) |= TIM_CR1_CEN;
	}

	/* TIM_SR_CC3 is set to latch on falling edges */
	if (TIM_SR(IR_TIMER) & TIM_SR_CC4IF) {
		TIM_CR1(IR_TIMER) &= ~(TIM_CR1_CEN);
		TIM_SR(IR_TIMER) &= ~(TIM_SR_CC4IF);
		irCapture.irEdges[irCaptureCounter].edgeType = IR_EDGE_FALLING;
		irCapture.irEdges[irCaptureCounter].nanoSeconds = TIM_CCR4(IR_TIMER);
		irCaptureCounter ++;
		TIM_CNT(IR_TIMER) = 0x0;
		TIM_CR1(IR_TIMER) |= TIM_CR1_CEN;
	}

	if (TIM_SR(IR_TIMER) & (TIM_SR_CC3OF | TIM_SR_CC4OF)) {
		TIM_SR(IR_TIMER) &= ~(TIM_SR_CC3OF | TIM_SR_CC4OF);
	}
}
