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
#pragma once

#define IR_TIMER TIM4
#define IR_TIMER_GPIO_BANK GPIO_BANK_TIM4
#define IR_TIMER_GPIO_PIN GPIO_TIM4_CH1
#define IR_LED_GPIO_BANK GPIOC
#define IR_LED_GPIO_PIN GPIO13
#define IR_MAX_EDGES 72
#define IR_SYNC_MARGIN_US 140
#define IR_SYNC_NOT_FOUND 255
#define IR_IDLE_THRESHOLD_US 16 * 1000
#define IR_DMA_CHANNEL DMA_CHANNEL1
#define IR_DMA DMA1
#define IR_DMA_BURST_LENGTH 1 /* 0b1 = 2 transfers for both CC register values */
#define IR_DMA_BASE_ADDRESS 0xD /* offset of CCR1 */

void setupInfrared(uint16_t *p_buf, uint8_t edgeCount);
void irResetDmaCounter(uint8_t edgeCount);
void irResetTimer(void);

bool irGenericCheckTime
		(const uint16_t *const p_capture,
		uint16_t timeBase,
		uint16_t timeMargin);
void debugPrintCapture
		(const uint16_t *const p_capture,
		uint8_t *const p_pos,
		char *p_debug);
