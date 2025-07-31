#ifndef FREERTOS_MOCK_H
#define FREERTOS_MOCK_H

// Mock FreeRTOS header for native testing

#include <stdint.h>

// Mock FreeRTOS types
typedef void* TaskHandle_t;
typedef uint32_t TickType_t;
typedef uint32_t BaseType_t;
typedef uint32_t UBaseType_t;

// Mock constants
#define pdTRUE 1
#define pdFALSE 0
#define portMAX_DELAY 0xFFFFFFFF

// Mock functions
inline void vTaskDelay(TickType_t xTicksToDelay) {}
inline TickType_t xTaskGetTickCount() { return 0; }

#endif // FREERTOS_MOCK_H