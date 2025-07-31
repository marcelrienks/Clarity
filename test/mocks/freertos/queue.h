#ifndef FREERTOS_QUEUE_MOCK_H
#define FREERTOS_QUEUE_MOCK_H

// Mock FreeRTOS queue header for native testing

#include "FreeRTOS.h"

// Mock queue types
typedef void* QueueHandle_t;

// Mock constants
#define portMAX_DELAY 0xFFFFFFFF

// Mock queue functions
inline QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize) { return nullptr; }
inline BaseType_t xQueueSend(QueueHandle_t xQueue, const void* pvItemToQueue, TickType_t xTicksToWait) { return pdTRUE; }
inline BaseType_t xQueueReceive(QueueHandle_t xQueue, void* pvBuffer, TickType_t xTicksToWait) { return pdFALSE; }
inline void vQueueDelete(QueueHandle_t xQueue) {}

#endif // FREERTOS_QUEUE_MOCK_H