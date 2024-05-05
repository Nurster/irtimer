#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/stm32/gpio.h"
#include <libopencm3/stm32/timer.h>
#include "libopencm3/stm32/dma.h"
#include <libopencm3/cm3/nvic.h>
#include "globals.h"
#include "drivers/infrared/ir.h"
#include "tasks/irtask.h"

static void setupGpio(void) {
	  gpio_set_mode(
		IR_TIMER_GPIO_BANK,
		GPIO_MODE_INPUT,
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
		IR_TIMER_GPIO_PIN
		);
	}

static void setupDma(uint16_t *buf) {
	/*
    dma_channel_reset(DMA1, DMA_CHANNEL1);
    dma_set_peripheral_address(DMA1, DMA_CHANNEL1, (uint32_t)&TIM_DMAR(IR_TIMER));
    dma_set_memory_address(DMA1, DMA_CHANNEL1, (uint32_t)buf);
    dma_set_priority(DMA1, DMA_CHANNEL1, DMA_CCR_PL_LOW);
    dma_set_read_from_peripheral(DMA1, DMA_CHANNEL1);
    dma_set_memory_size(DMA1, DMA_CHANNEL1, DMA_CCR_MSIZE_16BIT);
    dma_set_peripheral_size(DMA1, DMA_CHANNEL1, DMA_CCR_PSIZE_16BIT);
    dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL1);
    dma_set_number_of_data(DMA1, DMA_CHANNEL1, IR_MAX_EDGES + 1);
    */


    dma_channel_reset(DMA1, DMA_CHANNEL4);
    dma_set_peripheral_address(DMA1, DMA_CHANNEL4, (uint32_t)&TIM_DMAR(IR_TIMER));
    dma_set_memory_address(DMA1, DMA_CHANNEL4, (uint32_t)buf);
    dma_set_priority(DMA1, DMA_CHANNEL4, DMA_CCR_PL_LOW);
    dma_set_read_from_peripheral(DMA1, DMA_CHANNEL4);
    dma_set_memory_size(DMA1, DMA_CHANNEL4, DMA_CCR_MSIZE_16BIT);
    dma_set_peripheral_size(DMA1, DMA_CHANNEL4, DMA_CCR_PSIZE_16BIT);
    dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL4);
    dma_enable_circular_mode(DMA1, DMA_CHANNEL4);
    dma_set_number_of_data(DMA1, DMA_CHANNEL4, IR_MAX_EDGES + 1);
    dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL4);
}

void setupInfrared(uint16_t *buf) {
	setupGpio();
	setupDma(buf);
	nvic_enable_irq(NVIC_DMA1_CHANNEL4_IRQ);
	/*nvic_enable_irq(NVIC_TIM4_IRQ);*/
	rcc_periph_reset_pulse(RST_TIM4);
	timer_set_mode(
	  IR_TIMER,
	  TIM_CR1_CKD_CK_INT,
	  TIM_CR1_CMS_EDGE,
	  TIM_CR1_DIR_UP
  );

  /* count microseconds */
  timer_set_prescaler(IR_TIMER, ((rcc_apb2_frequency) / 1000000));
  timer_continuous_mode(IR_TIMER);

  TIM_ARR(IR_TIMER) = IR_IDLE_THRESHOLD_US;

  /*
   * fire an update even only on overflow
   */
  TIM_CR1(IR_TIMER) |= TIM_CR1_URS;

  /*
   * route channels 3 and 4 to timer input 4 to capture rising and falling edges
   */
  TIM_CCMR1(IR_TIMER)
  	  |= TIM_CCMR1_IC1F_CK_INT_N_8
	  | TIM_CCMR1_IC2F_CK_INT_N_8
	  | TIM_CCMR1_IC1PSC_OFF
	  | TIM_CCMR1_IC2PSC_OFF
	  | TIM_CCMR1_CC1S_IN_TI1
	  | TIM_CCMR1_CC2S_IN_TI1;


  TIM_SMCR(IR_TIMER)
  	  |= TIM_SMCR_TS_TI1FP1
	  | TIM_SMCR_SMS_RM;

  /*
   * configure channel 1 for rising and channel 2 for falling edges by reversing its polatity using CC2P
   */
  TIM_CCER(IR_TIMER)
  	  |= TIM_CCER_CC1E
	  | TIM_CCER_CC2E
	  | TIM_CCER_CC2P;

  TIM_DCR(IR_TIMER)
  	  |= IR_DMA_BURST_LENGTH << 8
	  | IR_DMA_BASE_ADDRESS << 0;

  /*
   * update registers and clear the UIF before enabling interupts to prevent false trigger
   */
  TIM_EGR(IR_TIMER) |= TIM_EGR_UG;
  TIM_SR(IR_TIMER) &= ~(TIM_SR_UIF);

 /*
  *  enable dma requests and arm timer for capturing
  */
  TIM_DIER(IR_TIMER)
  	  |= TIM_DIER_CC1DE
  	  | TIM_DIER_CC2DE
	  | TIM_DIER_UIE;

  /*DMA1_CCR1 |= DMA_CCR_EN;*/
  DMA1_CCR4 |= DMA_CCR_EN;

  TIM_CR1(IR_TIMER) |= TIM_CR1_CEN;

}

void dma1_channel1_isr(void) {
    if (DMA1_ISR & DMA_ISR_TCIF1) {
        DMA1_IFCR |= DMA_IFCR_CTCIF1;
        /*dma_disable_channel(DMA1, DMA_CHANNEL1);*/
    }
}

void dma1_channel4_isr(void) {
    if (DMA1_ISR & DMA_ISR_TCIF4) {
        DMA1_IFCR |= DMA_IFCR_CTCIF4;
        /*dma_disable_channel(DMA1, DMA_CHANNEL1);*/
    }
}
bool irGenericCheckEdgeTime
				(irCapture_t const* const p_capture,
				uint8_t pos,
				irEdgeType_t edgeType,
				uint16_t timeBase) {
	if ((p_capture->irEdges[pos].irEdgeType == edgeType)
			&& (p_capture->irEdges[pos].irMicroSeconds > (timeBase - IR_SYNC_MARGIN_US))
			&& (p_capture->irEdges[pos].irMicroSeconds < (timeBase + IR_SYNC_MARGIN_US))) {
		return true;
	} else {
		return false;
	}
}
