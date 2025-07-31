#ifndef NVS_FLASH_MOCK_H
#define NVS_FLASH_MOCK_H

// Mock nvs_flash header for native testing (ESP32 NVS library)

#include <stdint.h>

// Mock ESP error types
typedef int32_t esp_err_t;

#define ESP_OK          0       /*!< esp_err_t value indicating success (no error) */
#define ESP_FAIL        -1      /*!< Generic esp_err_t code indicating failure */

// Mock NVS functions
inline esp_err_t nvs_flash_init(void) { return ESP_OK; }
inline esp_err_t nvs_flash_erase(void) { return ESP_OK; }
inline esp_err_t nvs_flash_init_partition(const char* partition_label) { return ESP_OK; }

#endif // NVS_FLASH_MOCK_H