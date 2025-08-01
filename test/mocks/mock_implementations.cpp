#ifdef UNIT_TESTING

#include "Arduino.h"
#include "nvs_flash.h"

// Mock ESP32 functions implementation
extern "C" {
    esp_err_t nvs_flash_erase() { 
        return ESP_OK; 
    }
}

#endif // UNIT_TESTING