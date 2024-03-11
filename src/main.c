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
#include <FreeRTOS.h>
#include <task.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <stdio.h>
#include "main.h"

#include "include/uitask.h"

TaskHandle_t demoHandle;
TaskHandle_t taskTwoHandle;

/*
void sys_tick_handler(void) {
    sysTickCounter++;
}
*/

static void delayMillis(uint32_t millis) {
	/*
    uint32_t start = sysTickCounter;
    while (sysTickCounter - start < millis);
    */

	/*vTaskStartScheduler();*/
	vTaskDelay(pdMS_TO_TICKS(millis));

}

static void setupClock(void) {
#ifdef STM32F0    
    rcc_clock_setup_in_hse_8mhz_out_48mhz();
#elif STM32F1
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
#elif STM32F4
    rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_84MHZ]);
#endif
	rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_TIM3);
}

static void setupTimer(void) {
    nvic_enable_irq(NVIC_TIM3_IRQ);

	rcc_periph_reset_pulse(RST_TIM3);

	timer_set_mode(
		TIMER,
		TIM_CR1_CKD_CK_INT,
		TIM_CR1_CMS_EDGE,
		TIM_CR1_DIR_UP
	);

	// Timer für vertikale Synchronisation
	timer_set_prescaler(TIMER, ((rcc_apb2_frequency) / 1000)); // Millisekunden zählen
	timer_continuous_mode(TIMER); // immer wieder von vorn anfangen
	timer_set_period(TIMER, DISPLAY_MEMORY_HEIGHT); 
//  timer_set_oc_value(TIMER, TIM_OC1, DISPLAY_MEMORY_HEIGHT); 
//	timer_enable_irq(TIMER, TIM_DIER_CC1IE);
	timer_enable_irq(TIMER, TIM_DIER_UIE);
	timer_enable_counter(TIMER);
}


static void setupGpio(void) {

#ifdef STM32F1
    gpio_set_mode(
		DISPLAY_GPIO,
		GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
		SCK | MOSI
	);
    gpio_set_mode(
		DISPLAY_GPIO,
		GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL,
		RS | DC | CS
	);
#endif
    
#ifdef STM32F0
    gpio_mode_setup(
		DISPLAY_GPIO,
        GPIO_MODE_AF,
        GPIO_PUPD_PULLUP,
		SCK | MOSI
	);
    gpio_mode_setup(
		DISPLAY_GPIO,
        GPIO_MODE_OUTPUT,
		GPIO_PUPD_PULLUP,
		RS | DC | CS
	);

    gpio_mode_setup(
        GPIOB,
        GPIO_MODE_OUTPUT,
        GPIO_PUPD_PULLUP,
        GPIO1
    );
#endif
}

static void setupSpi(void) {
	/*spi_reset(SPI1);*/
#ifdef STM32F0    
    
	spi_init_master(
		SPI1,
		SPI_CR1_BAUDRATE_FPCLK_DIV_2,
		SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
		SPI_CR1_CPHA_CLK_TRANSITION_2,
		SPI_CR1_MSBFIRST
	);
#else
	spi_init_master(
		SPI1,
		SPI_CR1_BAUDRATE_FPCLK_DIV_2,
		SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
		SPI_CR1_CPHA_CLK_TRANSITION_2,
		SPI_CR1_DFF_8BIT,
		SPI_CR1_MSBFIRST
	);
#endif
    spi_set_bidirectional_transmit_only_mode(SPI1);
	spi_enable_software_slave_management(SPI1);
    //spi_enable_ss_output(SPI1);
	spi_set_nss_high(SPI1);
	spi_enable(SPI1);
}

static void hardReset(void) {
	gpio_clear(DISPLAY_GPIO, RS);
	delayMillis(1);
	gpio_set(DISPLAY_GPIO, RS);
	delayMillis(1);
}

static inline void waitTxDone(void) {
    //while (!(SPI_SR(SPI1) & SPI_SR_TXE));
    while ((SPI_SR(SPI1) & SPI_SR_BSY));
}

static void initDisplay(void) {
	//gpio_set(DISPLAY_GPIO, RS | DC | CS);
	hardReset();
	beginTransmissionSpi();
		sendCommand(0x01); // Soft Reset
		delayMillis(1);
		sendCommand(0x11); // Sleep Out
		delayMillis(1);
        sendCommand(0x13); // Normal Display ON
		sendCommand(0x20); // Inversion abschalten 


        setRotation(DISPLAY_ROTATION);


#if DISPLAY_COLOR_DEPTH == 16
        sendCommand(0x3a); // Pixelformat wÃƒÂ¤hlen
        sendData(0x05); // 16 Bit rgb-5-6-5
#elif DISPLAY_COLOR_DEPTH == 18
        sendCommand(0x3a); // Pixelformat wÃƒÂ¤hlen
        sendData(0x06); // 18 Bit rgb-6-6-6
#endif
		sendCommand(0x29); // Display On
	 endTransmissionSpi();	
}

void setRotation(uint8_t rotation) {
    switch (rotation) {
        case 1: rotation = 0; break;
        case 2: rotation = 3; break;
        case 3: rotation = 6; break;
        case 4: rotation = 5; break;
        default: rotation = 0; break;
    }    
    rotation <<= 5;
    rotation |= (4 << DISPLAY_PANEL_RGB_BGR);
    sendCommand(0x36); // Memory Access Control
    sendData(rotation);
}

void beginTransmissionSpi(void) {
	gpio_clear(DISPLAY_GPIO, CS);
}

void endTransmissionSpi(void) {
    waitTxDone();
	gpio_set(DISPLAY_GPIO, CS);
}

void sendCommand(uint8_t command) {
    gpio_clear(DISPLAY_GPIO, DC);
#ifdef STM32F0    
	spi_send8(SPI1, command);
#else
    spi_send(SPI1, command);
#endif
    waitTxDone();
}

void sendData(uint8_t data) {
	gpio_set(DISPLAY_GPIO, DC);
#ifdef STM32F0    
	spi_send8(SPI1, data);
#else
    spi_send(SPI1, data);
#endif
    waitTxDone();
}

void sendData16(uint16_t dataWord) {
	gpio_set(DISPLAY_GPIO, DC);
#ifdef STM32F0    
    spi_send8(SPI1, dataWord >> 8);
    spi_send8(SPI1, dataWord);
#else
    spi_send(SPI1, dataWord >> 8);
    spi_send(SPI1, dataWord);
#endif    
    waitTxDone();
}

void setMemoryWriteWindow(uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd, uint8_t xOffset, uint8_t yOffset) {

/*
        sendCommand(0x2a); // Spaltenanfang und ende
        sendData(0x00); // Oberes Byte der Startadresse
        sendData(xStart + xOffset); // Unteres Byte
        sendData(0x00); // Oberes Byte der Endadresse ist immer 0x00
        sendData(xEnd + xOffset); // unteres Byte

        sendCommand(0x2b); // Zeilenbeginn und -ende
        sendData(0x00); // Oberes Byte der Startadresse
        sendData(yStart + yOffset); // Unteres Byte
        sendData(0x00); // Oberes Byte der Endadresse ist immer 0x00
        sendData(yEnd + yOffset); // unteres Byte
*/
        sendCommand(0x2a); // Spaltenanfang und ende
        sendData16(xStart + xOffset);
        sendData16(xEnd + xOffset);

        sendCommand(0x2b); // Zeilenbeginn und -ende
        sendData16(yStart + yOffset);
        sendData16(yEnd + yOffset);

}

void writeMemoryStart(void) {
    sendCommand(0x2C);
}

void writePixel(uint8_t redByte, uint8_t greenByte, uint8_t blueByte) {
#if DISPLAY_COLOR_DEPTH == 18 // r-g-b 6-6-6
    // Da das MSB der sechs relevanten Bit an erste Stelle stehen muss, wird es zwei Stellen nach links verschoben
    // die rechts Ã¼brig geblieben zwei Bit werden nicht ausgewertet
    sendData(redByte << 2);
    sendData(greenByte << 2);
    sendData(blueByte << 2);
#elif DISPLAY_COLOR_DEPTH == 16 // r-g-b 5-6-5
	// rotes Byte (5 LSB-Bits relevant) 11 Stellen ganz nach links und grünes (6 LSB-Bits relevant) 5 Stellen nach links in die Mitte. 
	// Das blaue Byte (5 LSB-Bits relevant) bleibt stehen. Alles per ODER verknüpft ergibt das rgb-5-6-5-Format (16 Bit).
	sendData16((redByte << 11) | (greenByte << 5) | ( blueByte));
#endif
}

void fillMemory(uint16_t xEnd, uint16_t yEnd, uint8_t redByte, uint8_t greenByte, uint8_t blueByte) {
    uint32_t fillCounter = xEnd * yEnd;
    writeMemoryStart();
    do {
        writePixel(redByte, greenByte, blueByte);
        fillCounter--;
    } while (fillCounter > 0);
    
}

void clearScreen(void) {
    setMemoryWriteWindow(0, DISPLAY_WIDTH - 1, 0, DISPLAY_MEMORY_HEIGHT - 1, DISPLAY_X_OFFSET, 0);
    fillMemory(DISPLAY_WIDTH , DISPLAY_MEMORY_HEIGHT , 0, 0, 0);
}

void setPixel(uint16_t xWord,  uint16_t yWord, uint8_t redByte, uint8_t greenByte, uint8_t blueByte) {
    setMemoryWriteWindow(xWord, xWord, yWord, yWord, DISPLAY_X_OFFSET, DISPLAY_Y_OFFSET);
    writeMemoryStart();
    writePixel(redByte, greenByte, blueByte);
}

void setVerticalScrolling(uint16_t topFixedArea, uint16_t bottomFixedArea) {
    //topFixedArea += DISPLAY_Y_OFFSET;
    //bottomFixedArea += DISPLAY_Y_OFFSET;
    uint16_t scrollArea = DISPLAY_MEMORY_HEIGHT - bottomFixedArea - topFixedArea;

    sendCommand(0x33); // Vertical Scrolling Definition
    sendData16(topFixedArea);
    sendData16(scrollArea);
    sendData16(bottomFixedArea);
}

void setVericalScrollingStartAddress(uint16_t startAddress) {
    sendCommand(0x37); // Vertical Scrolling Start Address
    sendData16(startAddress);
}

void drawEllipse(int xm, int ym, int a, int b, uint8_t redByte, uint8_t greenByte, uint8_t blueByte) {
   int dx = 0, dy = b; /* im I. Quadranten von links oben nach rechts unten */
   long a2 = a*a, b2 = b*b;
   long err = b2-(2*b-1)*a2, e2; /* Fehler im 1. Schritt */

   do {
       setPixel(xm+dx, ym+dy, redByte, greenByte, blueByte); /* I. Quadrant */
       setPixel(xm-dx, ym+dy, redByte, greenByte, blueByte); /* II. Quadrant */
       setPixel(xm-dx, ym-dy, redByte, greenByte, blueByte); /* III. Quadrant */
       setPixel(xm+dx, ym-dy, redByte, greenByte, blueByte); /* IV. Quadrant */

       e2 = 2*err;
       if (e2 <  (2*dx+1)*b2) { dx++; err += (2*dx+1)*b2; }
       if (e2 > -(2*dy-1)*a2) { dy--; err -= (2*dy-1)*a2; }
   } while (dy >= 0);

   while (dx++ < a) { /* fehlerhafter Abbruch bei flachen Ellipsen (b=1) */
       setPixel(xm+dx, ym, redByte, greenByte, blueByte); /* -> Spitze der Ellipse vollenden */
       setPixel(xm-dx, ym, redByte, greenByte, blueByte);
   }
}

void demoDisplay(void){

	uint8_t test = 0xff;
	uint8_t *p_test = &test;

	/*printf ("Test is %d at address %p", *p_test, p_test);*/


	beginTransmissionSpi();
	clearScreen();
    setVerticalScrolling(0, 0);
    setVericalScrollingStartAddress(0);    
    drawEllipse(DISPLAY_WIDTH >> 2, DISPLAY_MEMORY_HEIGHT >> 2, DISPLAY_WIDTH >> 3, DISPLAY_WIDTH >> 3, 0, 63, 0);
    drawEllipse(DISPLAY_WIDTH - (DISPLAY_WIDTH >> 2), DISPLAY_MEMORY_HEIGHT >> 2, DISPLAY_WIDTH >> 3, DISPLAY_WIDTH >> 3, 63, 0, 0);
    drawEllipse(DISPLAY_WIDTH >> 2, DISPLAY_MEMORY_HEIGHT - (DISPLAY_MEMORY_HEIGHT >> 2), DISPLAY_WIDTH >> 3, DISPLAY_WIDTH >> 3, 0, 0, 63);
    drawEllipse(DISPLAY_WIDTH - (DISPLAY_WIDTH >> 2), DISPLAY_MEMORY_HEIGHT - (DISPLAY_MEMORY_HEIGHT >> 2), DISPLAY_WIDTH >> 3, DISPLAY_WIDTH >> 3, 63, 63, 0);
    sendCommand(0x39);
    //uint8_t startAddress = DISPLAY_HEIGHT >> 1;
    /*
    for (uint8_t i = 0; i < DISPLAY_MEMORY_HEIGHT; i++) {
        //uint8_t randValue = rand();
        setMemoryWriteWindow(0, (DISPLAY_WIDTH >> 0) - 1, i, DISPLAY_HEIGHT - 1);
        fillMemory(DISPLAY_WIDTH - 1, 1, i >> 2, i >> 1, i >> 1);
    }
    */
   //clearScreen();
//    setVerticalScrolling(0, 0);
//    setVericalScrollingStartAddress(0);
    /*
    setMemoryWriteWindow(0, DISPLAY_WIDTH - 1, 0, (DISPLAY_Y_OFFSET << 1) - 1, DISPLAY_X_OFFSET, 0);
    fillMemory(DISPLAY_WIDTH, (DISPLAY_Y_OFFSET << 1), 63, 0, 0);

    setMemoryWriteWindow(0, DISPLAY_WIDTH - 1, 80, 80 + (DISPLAY_Y_OFFSET << 1) - 1, DISPLAY_X_OFFSET, 0);
    fillMemory(DISPLAY_WIDTH, (DISPLAY_Y_OFFSET << 1), 0, 63, 0);

    setMemoryWriteWindow(0, DISPLAY_WIDTH - 1, 160, 160 + (DISPLAY_Y_OFFSET << 1) - 1, DISPLAY_X_OFFSET, 0);
    fillMemory(DISPLAY_WIDTH, (DISPLAY_Y_OFFSET << 1), 0, 0, 63);

    setMemoryWriteWindow(0, DISPLAY_WIDTH - 1, 240, 240 + (DISPLAY_Y_OFFSET << 1) - 1, DISPLAY_X_OFFSET, 0);
    fillMemory(DISPLAY_WIDTH, (DISPLAY_Y_OFFSET << 1), 63, 63, 0);
    */

/*
while(1) {
    uint16_t drawBehindScrollLineCounter = 0;
    for (uint16_t i = 0; i < DISPLAY_MEMORY_HEIGHT; i++) {
        if (drawBehindScrollLineCounter == (DISPLAY_Y_OFFSET * 2) || i == 0) {
            setMemoryWriteWindow(0, DISPLAY_WIDTH - 1, i, i + (DISPLAY_Y_OFFSET * 2) - 1, DISPLAY_X_OFFSET, 0);
            fillMemory(DISPLAY_WIDTH, (DISPLAY_Y_OFFSET << 1), 24, i >> 1, i >> 4);
            drawBehindScrollLineCounter = 0;
        }
        drawBehindScrollLineCounter++;
        setVericalScrollingStartAddress(i + (DISPLAY_Y_OFFSET) + 1);
        delayMillis(16);      
    }
}
*/

    while(1) {
        for (uint16_t i = 0; i <= DISPLAY_MEMORY_HEIGHT; i++) {
            setVericalScrollingStartAddress(i);
            delayMillis(48);
        }
    }

/*
    uint16_t startAddress = DISPLAY_MEMORY_HEIGHT >> 1;
    while (1) {
        for(double i = 0; i < 2 * PI; i += 0.01) {
            setVericalScrollingStartAddress((startAddress + DISPLAY_Y_OFFSET) + (sin(i) * (startAddress >> 0)));
            delayMillis(8);
        }
    }
*/
	endTransmissionSpi();
}

void tim3_isr(void) {
/*	
	if (timer_get_flag(TIMER, TIM_SR_CC1IF)) {
		timer_clear_flag(TIMER, TIM_SR_CC1IF);		
		gpio_clear(GPIOC, GPIO13);
	}
*/
	if (timer_get_flag(TIMER, TIM_SR_UIF)) {
		timer_clear_flag(TIMER, TIM_SR_UIF);
        gpio_toggle(DISPLAY_GPIO, CS);
		//setVericalScrollingStartAddress(timer_get_counter(TIMER));
	}
}
/*
void vApplicationStackOverflowHook(
	TaskHandle_t xTask __attribute__((unused)),
    char *pcTaskName __attribute__((unused))) {

	while(1);
}
*/
void taskTwo(void *pvParameters __attribute__((unused))) {
	/*printf("Hello World!");*/
	while (2) {

	}
}

void demoTask(void *pvParameters __attribute__((unused))) {
	initDisplay();
	while (2) {

	}
}

int main(void) {

	volatile BaseType_t createResult;

	setupClock();
	setupGpio();
	setupSpi();
    /*setupSysTick();*/
	createResult = xTaskCreate(demoTask, "Demo", 200, NULL, 0, &demoHandle);
	createResult = xTaskCreate(taskTwo, "Two", 200, NULL, 0, &taskTwoHandle);

	/* printf("createResult: %d", (uint8_t)createResult);*/

	/*demoDisplay();*/
    setupTimer();
	/*NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );*/
	vTaskStartScheduler();

	return 0;
}

