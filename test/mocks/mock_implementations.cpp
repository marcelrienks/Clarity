#ifdef UNIT_TESTING

#include "Arduino.h"
#include "nvs_flash.h"

// Mock Arduino global instances
MockSerial Serial;
MockSPI SPI;

// Mock ESP32 functions implementation
extern "C" {
    esp_err_t nvs_flash_erase() { 
        return ESP_OK; 
    }
    
    esp_err_t nvs_flash_init() {
        return ESP_OK;
    }
}

#endif // UNIT_TESTING