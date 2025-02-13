/*
 * (c) 2025 Nurster
 * https://github.com/Nurster
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * or version 3 as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>
#include "globals.h"
#include "drivers/display/display.h"
#include "drivers/display/st7789.h"
#include "drivers/serial/serial.h"
#include "tasks/uitask.h"
#include "tasks/irtask.h"

static inline void waitSpiTxBufferEmpty(void) {
	/* wait until SPI-TX-Puffer is empty */
    while (!(SPI_SR(DISPLAY_SPI) & SPI_SR_TXE));
}

static inline void waitSpiTxTransferDone(void) {
	/* wait for SPI to finish transfer */
    while ((SPI_SR(DISPLAY_SPI) & SPI_SR_BSY));
}

void dma1_channel3_isr (void) {
	/* needed to tell uiTask when dma transfer is finished */
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint32_t irDmaInterruptStatusRegister = (uint32_t)DMA1_ISR;

	if (DMA1_ISR & DMA_ISR_TCIF3) {
		DMA1_IFCR |= DMA_IFCR_CGIF3;
		xTaskNotifyIndexedFromISR(g_uiTaskHandle, UI_INDEX_DISPLAY_DMA_TRANSFER_COMPLETE, irDmaInterruptStatusRegister, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

static void setupGpio(void) {
	gpio_set_mode(
		GPIO_BANK_SPI1_NSS,
		GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
		DISPLAY_SPI_MOSI | DISPLAY_SPI_CLK | DISPLAY_SPI_EN
	);
	gpio_set_mode(
		GPIO_BANK_SPI1_NSS,
		GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL,
		DISPLAY_SPI_DC | DISPLAY_SPI_RS
	);

}

static void setupSpi(void) {
	/* spi_reset(SPI1); */
	spi_init_master(
		SPI1,
		SPI_CR1_BAUDRATE_FPCLK_DIV_128,
		SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
		SPI_CR1_CPHA_CLK_TRANSITION_2,
		SPI_CR1_DFF_8BIT,
		SPI_CR1_MSBFIRST
	);
    spi_set_unidirectional_mode(DISPLAY_SPI);
    spi_enable_ss_output(DISPLAY_SPI);
	spi_set_nss_high(DISPLAY_SPI);
}

static void setupdDma(void) {
	dma_channel_reset(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL);
	dma_set_peripheral_address(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL, (uint32_t)&SPI_DR(DISPLAY_SPI));
	dma_set_priority(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL, DMA_CCR_PL_LOW);
	dma_set_read_from_memory(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL);
	dma_set_peripheral_size(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL, DISPLAY_DMA_CCR_PSIZE);
	dma_set_memory_size(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL, DISPLAY_DMA_CCR_MSIZE);
	dma_enable_memory_increment_mode(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL);
	dma_disable_peripheral_increment_mode(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL);
	dma_enable_transfer_complete_interrupt(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL);
	nvic_enable_irq(NVIC_DMA1_CHANNEL3_IRQ);
}

void sendSpiDma(volatile uint32_t *p_buf, uint16_t len) {
	/*ToDo
	 *
	 * NDT register is only 16 bit wide. For a full transfer of 320*240=
	 */
    dma_set_memory_address(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL, (uint32_t)p_buf);
    dma_set_number_of_data(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL, len);
    spi_enable_tx_dma(DISPLAY_SPI);
    dma_enable_channel(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL);
}

void finishSpiDma(void) {
    waitSpiTxBufferEmpty();
    waitSpiTxTransferDone();
    spi_disable_tx_dma(DISPLAY_SPI);
    dma_disable_channel(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL);
    spi_disable(DISPLAY_SPI);
}

void sendSpiCommand(uint8_t command) {
	waitSpiTxBufferEmpty();
    gpio_clear(DISPLAY_SPI_BANK, DISPLAY_SPI_DC);
    SPI_DR(DISPLAY_SPI) = command;
	waitSpiTxTransferDone();
}

void sendSpiData(uint16_t data) {
	waitSpiTxBufferEmpty();
    gpio_set(DISPLAY_SPI_BANK, DISPLAY_SPI_DC);
    SPI_DR(DISPLAY_SPI) = data;
	waitSpiTxTransferDone();
}

void sendSpiData16(uint16_t data) {
	sendSpiData(data >> 8);
	sendSpiData(data);
}

void setupDisplay(void) {
	setupGpio();
	setupSpi();
	setupdDma();
	initDisplay(); /* implemented in the driver file */
}
