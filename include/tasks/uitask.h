#pragma once

#define UI_QUEUE_SIZE 5
#define UI_INDEX_DISPLAY_DMA_TRANSFER_COMPLETE 1
#define UI_INDEX_DISPLAY_DMA_TRANSFER_READY 2
#define UI_INDEX_IR 3
#define UI_QUEUE_TIMEOUT_MS 1000
#define UI_DMA_TIMEOUT_MS 2000

void uiTask(void *pvParameters __attribute__((unused)));
