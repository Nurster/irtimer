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

static void setupGpio(void) {
	  gpio_set_mode(
	      GPIO_BANK_TIM3_CH3,
	      GPIO_MODE_INPUT,
	      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
	      GPIO_TIM3_CH4
	      );
	}

void setupInfrared(void) {
	setupGpio();
	nvic_enable_irq(NVIC_TIM3_IRQ);
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

  TIM_ARR(IR_TIMER) = IR_IDLE_THRESHOLD_US;

  /*
   * route channels 3 and 4 to timer input 4 to capture rising and falling edges
   */
  TIM_CCMR2(IR_TIMER)
  	  |= TIM_CCMR2_IC3F_CK_INT_N_8
	  | TIM_CCMR2_IC4F_CK_INT_N_8
	  | TIM_CCMR2_IC3PSC_OFF
	  | TIM_CCMR2_IC4PSC_OFF
	  | TIM_CCMR2_CC3S_IN_TI4
	  | TIM_CCMR2_CC4S_IN_TI4;

  /*
   * configure channel 3 for rising and channel 4 for falling edges by reversing its polatity using CC4P
   */
  TIM_CCER(IR_TIMER)
  	  |= TIM_CCER_CC3E
	  | TIM_CCER_CC4E
	  | TIM_CCER_CC4P;

  /*
   * update registers and clear the UIF before enabling interupts to prevent false trigger
   */
  TIM_EGR(IR_TIMER) |= TIM_EGR_UG;
  TIM_SR(IR_TIMER) &= ~(TIM_SR_UIF);

 /*
  * finally enable irqs to arm timer for capturing
  */
  TIM_DIER(IR_TIMER)
  	  |= TIM_DIER_UIE
	  | TIM_DIER_CC3IE
	  | TIM_DIER_CC4IE;
}

bool irGenericCheckEdgeTime \
				(irCapture_t const* const p_capture, \
				uint8_t pos, \
				irEdgeType_t edgeType, \
				uint16_t timeBase) {
	if ((p_capture->irEdges[pos].irEdgeType == edgeType)
			&& (p_capture->irEdges[pos].irMicroSeconds > (timeBase - IR_SYNC_MARGIN_US))
			&& (p_capture->irEdges[pos].irMicroSeconds < (timeBase + IR_SYNC_MARGIN_US))) {
		return true;
	} else {
		return false;
	}
}
