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
#include <libopencm3/stm32/f1/dma.h>
#include <libopencm3/cm3/nvic.h>
#include "globals.h"
#include "drivers/display/display.h"
#include "drivers/display/st7789.h"
#include "drivers/serial/serial.h"
#include "tasks/uitask.h"
#include "tasks/irtask.h"

void initDisplay(void) {
	gpio_clear(DISPLAY_SPI_BANK, DISPLAY_SPI_RS);
	vTaskDelay(1);
	gpio_set(DISPLAY_SPI_BANK, DISPLAY_SPI_RS);
	vTaskDelay(1);
	gpio_clear(DISPLAY_SPI_BANK, DISPLAY_SPI_DC);
	spi_enable(DISPLAY_SPI);
	sendSpiCommand(DISPLAY_CMD_SOFT_RESET);
	vTaskDelay(1);
	sendSpiCommand(DISPLAY_CMD_SLEEP_OUT);
	vTaskDelay(1);
	sendSpiCommand(DISPLAY_CMD_NORMAL_DISPLAY);
	sendSpiCommand(DISPLAY_CMD_INVERSION_OFF);
	sendSpiCommand(DISPLAY_CMD_IDLE_MODE_OFF);
	sendSpiCommand(DISPLAY_CMD_PIXEL_COLOR_CODING);
	sendSpiData(DISPLAY_COLOR_DEPTH);
	sendSpiCommand(DISPLAY_CMD_MEMORY_ACCESS_CONTROL);
	sendSpiData(DISPLAY_ROTATION | DISPLAY_PANEL_COLOR);
	sendSpiCommand(DISPLAY_CMD_DISPLAY_PANEL_ON);
	vTaskDelay(2);
	spi_disable(DISPLAY_SPI);
}

static void setMemoryWriteWindow(uint16_t xStart, uint16_t yStart,
		uint16_t width, uint16_t height) {
	SPI_CR1(DISPLAY_SPI) |= SPI_CR1_DFF_16BIT;
	sendSpiCommand(DISPLAY_CMD_COLUMN_ADDRESS_SET);
	sendSpiData(xStart);
	sendSpiData(xStart + width);
	sendSpiCommand(DISPLAY_CMD_ROW_ADDRESS_SET);
	sendSpiData(yStart);
	sendSpiData(yStart + height);
}

void sendBuffer(displayBuffer_t *p_buf) {
	spi_enable(DISPLAY_SPI);
	if (p_buf->start == true) {
		setMemoryWriteWindow(p_buf->startx + p_buf->offsetx,
				p_buf->starty + p_buf->offsety,
				p_buf->width - 1,
				p_buf->height - 1);
	    sendSpiCommand(DISPLAY_CMD_WRITE_MEMORY_START);
	} else {
	    sendSpiCommand(DISPLAY_CMD_WRITE_MEMORY_CONTINUE);
	}

    gpio_set(DISPLAY_SPI_BANK, DISPLAY_SPI_DC);

    if (p_buf->single == true) {
		dma_disable_memory_increment_mode(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL);
	} else {
		dma_enable_memory_increment_mode(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL);
	}

	sendSpiDma(p_buf->p_buffer, p_buf->dmaNumTransfersRemaining);
}

static void sendQueue(displayBuffer_t *p_buf) {
	char out[DEBUG_OUTPUT_CHAR_SIZE];

	printStringSerial(out);
	if (xQueueSendToBack(g_uiQueueHandle, p_buf, pdMS_TO_TICKS(100)) == errQUEUE_FULL) {
		snprintf(out, sizeof(out), "%lu\t%s: Queue is full with number of items: %lu of %lu\r\n",
				xTaskGetTickCount(),
				pcTaskGetName(xTaskGetCurrentTaskHandle()),
				uxQueueMessagesWaiting(g_uiQueueHandle),
				uxQueueGetQueueLength(g_uiQueueHandle));
		printStringSerial(out);
	}
}

void sendBufferToQueue(displayBuffer_t *p_buf) {

	uint8_t dmaBoundaryHits = 0;
	uint16_t dmaTail = 0;
#ifdef DISPLAY_COLOR_DEPTH
	#if DISPLAY_COLOR_DEPTH == DISPLAY_COLOR_DEPTH_12
		p_buf->dmaNumTransfersRemaining = ((p_buf->width * p_buf->height) * DISPLAY_BITS_PER_PIXEL) / DISPLAY_DMA_MSIZE;
	#elif DISPLAY_COLOR_DEPTH == DISPLAY_COLOR_DEPTH_16
		p_buf->dmaNumTransfersRemaining = p_buf->width * p_buf->height;
	#elif DISPLAY_COLOR_DEPTH == DISPLAY_COLOR_DEPTH_18
		p_buf->dmaNumTransfersRemaining = (p_buf->width * p_buf->height) * DISPLAY_COMPLETE_TRANSFER_NUM_PIXELS;
	#endif
#endif
	/*
	 * this is necessary due to DMA_NDT is only 16 bits wide
	 */
	dmaBoundaryHits = p_buf->dmaNumTransfersRemaining / DISPLAY_DMA_BOUNDARY;
	dmaTail = p_buf->dmaNumTransfersRemaining % DISPLAY_DMA_BOUNDARY;
	if (((dmaBoundaryHits == 0) && (dmaTail == 0))
			|| (p_buf->p_buffer == NULL)) {
		return;
	}
	/* send once to set window */
	p_buf->start = true;
	do {
		if (dmaBoundaryHits != 0) {
			p_buf->dmaNumTransfersRemaining = dmaBoundaryHits * DISPLAY_DMA_BOUNDARY;
			sendQueue(p_buf);
			if (p_buf->single == false) {
				p_buf->p_buffer += DISPLAY_DMA_BOUNDARY;
			}
			/*
			 * don't reset our position just yet because we need it
			 * to start from in pending transmissions
			 */
			p_buf->start = false;
		}
		if (dmaBoundaryHits == 0 && dmaTail != 0) {
			p_buf->dmaNumTransfersRemaining = dmaTail;
			dmaTail = 0;
			sendQueue(p_buf);
		}
	} while (dmaBoundaryHits -- > 0);
}



void clearScreen(volatile uint32_t *p_color) {
	/*
	 * Todo put real displayBuffer_t buffers into queue instead of pointers
	 * to preserve different settings for each transfer
	 */
	displayBuffer_t t_buf;

	t_buf.p_buffer = p_color;
	t_buf.startx = 0,
	t_buf.starty = 0,
	t_buf.width = DISPLAY_MEMORY_WIDTH,
	t_buf.height = DISPLAY_MEMORY_HEIGHT,
	t_buf.offsetx = 0;
	t_buf.offsety = 0;
	t_buf.single = true;

	sendBufferToQueue(&t_buf);
}


