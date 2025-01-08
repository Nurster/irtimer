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
    sendSpiCommand(0x01); /* soft reset */
	vTaskDelay(1);
    sendSpiCommand(0x11); /* sleep out */
	vTaskDelay(1);
	sendSpiCommand(0x13); /* normal display, use full area */
	sendSpiCommand(0x21); /* inversion off */
    sendSpiCommand(0x38); /* idle mode off */
	sendSpiCommand(0x3a); /* set pixel color coding */
    sendSpiData(DISPLAY_COLOR_DEPTH);
    sendSpiCommand(0x36); /* memory access control */
    sendSpiData((DISPLAY_ROTATION << 5) | (DISPLAY_PANEL_COLOR << 3)); /* scan mode and panel subpixel order*/
    sendSpiCommand(0x29); /* display panel switch on */
    vTaskDelay(2);
    spi_disable(DISPLAY_SPI);
}


void setMemoryWriteWindow(uint16_t xStart, uint16_t yStart, uint16_t width, uint16_t height) {

		SPI_CR1(DISPLAY_SPI) |= SPI_CR1_DFF_16BIT;
        sendSpiCommand(0x2a);

        sendSpiData(xStart + DISPLAY_MEMORY_OFFSET_X);
        sendSpiData(xStart + width + DISPLAY_MEMORY_OFFSET_X);

        sendSpiCommand(0x2b);
        sendSpiData(yStart + DISPLAY_MEMORY_OFFSET_Y);
        sendSpiData(yStart + height + DISPLAY_MEMORY_OFFSET_Y);

}

void writeMemoryStart(void) {
    sendSpiCommand(0x2c);
}

void sendBuffer(displayBuffer_t *p_buf) {
	spi_enable(DISPLAY_SPI);
	setMemoryWriteWindow(p_buf->startx, p_buf->starty, p_buf->width - 1, p_buf->height - 1);
	writeMemoryStart();
    gpio_set(DISPLAY_SPI_BANK, DISPLAY_SPI_DC); /* set to data */
    /*DMA_CCR(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL) |= DMA_CCR_CIRC;*/
    dma_set_memory_size(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL, DMA_CCR_MSIZE_16BIT);
	dma_set_peripheral_size(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL, DMA_CCR_PSIZE_16BIT);

	if (p_buf->single == true) {
		dma_disable_memory_increment_mode(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL);
	} else {
		dma_enable_memory_increment_mode(DISPLAY_SPI_DMA, DISPLAY_SPI_DMA_CHANNEL);
	}
	sendSpiDma(p_buf->p_buffer, p_buf->width * p_buf->height);
}

void clearScreen(displayBuffer_t *p_buf) {


	p_buf->single = true;
	if (xQueueSendToBack(g_uiQueueHandle, p_buf, pdMS_TO_TICKS(100)) == errQUEUE_FULL) {
		printStringSerial("Queue is full!\r\n");
	}
}

