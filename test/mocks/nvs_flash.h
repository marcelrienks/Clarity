#pragma once

#ifdef UNIT_TESTING

// Mock ESP32 NVS functions
typedef int esp_err_t;
#define ESP_OK 0

extern "C" {
    esp_err_t nvs_flash_erase();
}

#endif // UNIT_TESTING