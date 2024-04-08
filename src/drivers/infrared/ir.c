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

void setupInfrared(void) {
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
