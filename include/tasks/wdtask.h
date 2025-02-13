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

#define WD_LSI_FREQUENCY_HZ 40 * 1000
#define WD_PRESCALER_DIVIDER_BASE 4
#define WD_PRESCALER_BITS 3
#define WD_PRESCALER WD_PRESCALER_DIVIDER_BASE << WD_PRESCALER_BITS
#define WD_PERIOD_BASE_MS ((int)(WD_LSI_FREQUENCY_HZ) / (WD_PRESCALER))
#define WD_PERIOD_MS 3 * 1000
#define WD_PERIOD WD_PERIOD_BASE_MS * (WD_PERIOD_MS / 1000)
#define WD_RESET_INTERVAL_MS 1 * 1000

void wdTask(void *pvParameters __attribute__((unused)));
