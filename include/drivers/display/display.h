#pragma once

#define DISPLAY_SPI SPI1
#define DISPLAY_SPI_BANK GPIO_BANK_SPI1_NSS
#define DISPLAY_SPI_MOSI GPIO_SPI1_MOSI
#define DISPLAY_SPI_CLK GPIO_SPI1_SCK
#define DISPLAY_SPI_EN GPIO_SPI1_NSS
#define DISPLAY_SPI_DC GPIO3
#define DISPLAY_SPI_RS GPIO2

#define DISPLAY_SPI_DMA DMA1
#define DISPLAY_SPI_DMA_CHANNEL DMA_CHANNEL3

typedef struct {
	volatile uint32_t *p_buffer;
	uint16_t startx;
    uint16_t starty;
	uint16_t height;
	uint16_t width;
	uint16_t offsetx;
	uint16_t offsety;
	uint32_t dmaTransfersRemaining;
	bool setMemoryWindow;
	bool single;
} displayBuffer_t;

#include "drivers/display/st7789.h"
void setupDisplay(void);
void sendSpiDma(volatile uint32_t *p_buf, uint16_t len);
void finishSpiDma(void);
void sendSpiCommand(uint8_t command);
void sendSpiData(uint16_t data);
void sendSpiData16(uint16_t data);
void initDisplay(void);
void clearScreen(volatile uint32_t *p_color);
void sendBuffer(displayBuffer_t *p_buf);

