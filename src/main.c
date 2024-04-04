/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
 * Copyright (C) 2012 Karl Palsson <karlp@tweak.net.au>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

//#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/stm32/gpio.h"
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "main.h"
#include "include/uitask.h"

TaskHandle_t demoHandle;
TaskHandle_t taskTwoHandle;

static irCapture_t irCaptures[IR_MAX_EDGES];
static irCapture_t *p_irCapture;

static void setupClock(void) {
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_TIM3);
}

static void setupTimer(void) {

	memset(&irCaptures,0, sizeof(irCapture_t));
	p_irCapture = &irCaptures[0];

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
   * either continuous or one shot mode does not make a difference,
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

  TIM_DIER(IR_TIMER)
  	  |= TIM_DIER_UIE
	  | TIM_DIER_CC3IE
	  | TIM_DIER_CC4IE
	  ;

  TIM_CR1(IR_TIMER) |= TIM_CR1_CEN;

}


static void setupGpio(void) {
  gpio_set_mode(
      GPIO_BANK_TIM3_CH3,
      GPIO_MODE_INPUT,
      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
      GPIO_TIM3_CH4
      );
}

void tim3_isr(void) {

	/* continue from beginning of array if end is reached */
	if (p_irCapture == &irCaptures[IR_MAX_EDGES - 1]) {
		p_irCapture = &irCaptures[0];
	}

	/* TIM_SR_CC3 is set to latch on rising edges */
	if ((TIM_SR(IR_TIMER) & TIM_SR_CC3IF)) {
		TIM_SR(IR_TIMER) &= ~(TIM_SR_CC3IF);
		/* store time passed since last latch */

		p_irCapture->field.edgeType = IR_EDGE_RISING;
		p_irCapture->field.nanoseconds = TIM_CCR3(IR_TIMER);
		p_irCapture ++;
		TIM_CR1(IR_TIMER) |= TIM_CR1_CEN;
		TIM_CNT(IR_TIMER) = 0x0;


	}

	/* TIM_SR_CC3 is set to latch on falling edges */
	if (TIM_SR(IR_TIMER) & TIM_SR_CC4IF) {
		TIM_SR(IR_TIMER) &= ~(TIM_SR_CC4IF);

		p_irCapture->field.edgeType = IR_EDGE_FALLING;
		p_irCapture->field.nanoseconds = TIM_CCR4(IR_TIMER);
		p_irCapture ++;
		TIM_CR1(IR_TIMER) |= TIM_CR1_CEN;
		TIM_CNT(IR_TIMER) = 0x0;

	}

	if (TIM_SR(IR_TIMER) & TIM_SR_UIF) {

		/*
		 * timer overflowed due to no keypress
		 * disable timer to arm for next input capture sequence
		 * set pointer to beginning of capture array
		 */
		TIM_SR(IR_TIMER) &= ~(TIM_SR_UIF);

		p_irCapture = &irCaptures[0];
		TIM_CNT(IR_TIMER) = 0x0;
		TIM_CR1(IR_TIMER) &= ~(TIM_CR1_CEN);
	}

	/* TIM_CNT(IR_TIMER) = 0x0; */

}

void taskTwo(void *pvParameters __attribute__((unused))) {
	/*printf("Hello World!");*/
	while (2) {
	    vTaskDelay(pdMS_TO_TICKS(20));

	}
}

void demoTask(void *pvParameters __attribute__((unused))) {
	while (2) {
	    vTaskDelay(pdMS_TO_TICKS(20));
	}
}

int main(void) {

	volatile BaseType_t createResult;

	setupClock();
	setupGpio();
	createResult = xTaskCreate(demoTask, "Demo", 200, NULL, 0, &demoHandle);
	createResult = xTaskCreate(taskTwo, "Two", 200, NULL, 0, &taskTwoHandle);
	setupTimer();
	vTaskStartScheduler();

	return 0;
}

