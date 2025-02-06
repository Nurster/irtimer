#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>
#include "globals.h"
#include "drivers/infrared/ir.h"
#include "drivers/serial/serial.h"
#include "tasks/irtask.h"

static void setupGpio(void) {
	gpio_set_mode(
		IR_TIMER_GPIO_BANK,
		GPIO_MODE_INPUT,
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
		IR_TIMER_GPIO_PIN
	);
	gpio_set_mode(
		IR_LED_GPIO_BANK,
		GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL,
		IR_LED_GPIO_PIN
	);
	gpio_set(
		IR_LED_GPIO_BANK,
		IR_LED_GPIO_PIN
	);

}

static void setupdDma(uint16_t *p_buf, uint8_t edgeCount) {
	dma_channel_reset(IR_DMA, IR_DMA_CHANNEL);
	dma_set_peripheral_address(IR_DMA, IR_DMA_CHANNEL, (uint32_t) &TIM_DMAR(IR_TIMER));
	dma_set_memory_address(IR_DMA, IR_DMA_CHANNEL, (uint32_t) p_buf);
	dma_set_priority(IR_DMA, IR_DMA_CHANNEL, DMA_CCR_PL_LOW);
	dma_set_read_from_peripheral(IR_DMA, IR_DMA_CHANNEL);
	dma_set_peripheral_size(IR_DMA, IR_DMA_CHANNEL, DMA_CCR_PSIZE_16BIT);
	dma_set_memory_size(IR_DMA, IR_DMA_CHANNEL, DMA_CCR_MSIZE_16BIT);
	dma_enable_memory_increment_mode(IR_DMA, IR_DMA_CHANNEL);
	dma_enable_circular_mode(IR_DMA, IR_DMA_CHANNEL);
	dma_set_number_of_data(IR_DMA, IR_DMA_CHANNEL, edgeCount);
}

void setupInfrared(uint16_t *p_buf, uint8_t edgeCount) {
	setupGpio();
	setupdDma(p_buf, edgeCount);

	rcc_periph_reset_pulse(RST_TIM4);
	/*
	 * clocked interally, upcounting
	 */
	timer_set_mode(
		IR_TIMER,
		TIM_CR1_CKD_CK_INT,
		TIM_CR1_CMS_EDGE,
		TIM_CR1_DIR_UP
	);

	/*
	 * count microseconds
	 */
	timer_set_prescaler(IR_TIMER, ((rcc_apb2_frequency) / 1000000));
	timer_continuous_mode(IR_TIMER);

	TIM_ARR(IR_TIMER) = IR_IDLE_THRESHOLD_US;

	/*
	 * route channels 3 and 4 to timer input 4 to capture rising and falling edges
	 */
	TIM_CCMR1(IR_TIMER) |= TIM_CCMR1_IC1F_CK_INT_N_8
			| TIM_CCMR1_IC2F_CK_INT_N_8
			| TIM_CCMR1_IC1PSC_OFF
			| TIM_CCMR1_IC2PSC_OFF
			| TIM_CCMR1_CC1S_IN_TI1
			| TIM_CCMR1_CC2S_IN_TI1;

	/*
	 * set slave mode control register to reset mode so each rising edge resets the counter register
	 */
	TIM_SMCR(IR_TIMER) |= TIM_SMCR_TS_TI1FP1 | TIM_SMCR_SMS_RM;

	/*
	 * configure channel 1 for rising and channel 2 for falling edges by reversing its polatity using CC2P
	 */
	TIM_CCER(IR_TIMER) |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC2P;

	TIM_DCR(IR_TIMER) |= IR_DMA_BURST_LENGTH << 8 /* get two values in total from ...*/
			| IR_DMA_BASE_ADDRESS << 0; /* ... first two counter registers */

	/*
	 * update registers and clear the UIF to achieve a defined state and prevent false trigger
	 */
	TIM_EGR(IR_TIMER) |= TIM_EGR_UG;
	TIM_SR(IR_TIMER) &= ~(TIM_SR_UIF);

	/*
	 * enable dma requests and arm timer for capturing
	 */
	TIM_DIER(IR_TIMER) |= TIM_DIER_CC2DE;

	DMA_CCR(IR_DMA, IR_DMA_CHANNEL) |= DMA_CCR_EN;
	TIM_CR1(IR_TIMER) |= TIM_CR1_CEN;

}

void irResetDmaCounter(uint8_t edgeCount) {
	DMA_CCR(IR_DMA, IR_DMA_CHANNEL) &= ~(DMA_CCR_EN);
	DMA_CNDTR(IR_DMA, IR_DMA_CHANNEL) = edgeCount;
	DMA_CCR(IR_DMA, IR_DMA_CHANNEL) |= DMA_CCR_EN;
}

bool irGenericCheckTime(const uint16_t *const p_capture, uint16_t timeBase,
		uint16_t timeMargin) {
	if ((*p_capture > (timeBase - timeMargin))
			&& (*p_capture < (timeBase + timeMargin))) {
		return true;
	} else {
		return false;
	}
}
;

bool irGenericFindSync(const uint16_t *const p_capture, uint8_t *const p_pos,
		uint16_t syncUs, uint8_t edgeCount) {
	for (uint8_t i = 0; i < edgeCount; i++) {
		if (irGenericCheckTime(&p_capture[i], syncUs, IR_SYNC_MARGIN_US)) {
			*p_pos = i;
			return true;
		}
	}
	return false;
}

void debugPrintCapture(const uint16_t *const p_capture, uint8_t *const p_pos,
		char *p_debug) {

	sprintf(p_debug, "IR: debug:\t pos: \t%d\t µS: \t%hd\r\n", *p_pos,
			p_capture[*p_pos]);
	printStringSerial(p_debug);
}
