#ifndef LOVYANGFX_HPP_MOCK
#define LOVYANGFX_HPP_MOCK

// Mock LovyanGFX header for native testing
// This provides stub implementations for LovyanGFX functionality

#include <stdint.h>

namespace lgfx {
    namespace v1 {
        
        // Mock Bus SPI configuration
        struct Bus_SPI {
            struct config_t {
                uint32_t freq_write = 80000000;
                uint32_t freq_read = 20000000;
                uint8_t spi_mode = 0;
                bool spi_3wire = true;
                bool use_lock = true;
                uint8_t dma_channel = 3;
                int8_t pin_sclk = 18;
                int8_t pin_mosi = 23;
                int8_t pin_miso = -1;
                int8_t pin_dc = 16;
            };
            
            config_t config() { return cfg; }
            void config(const config_t& new_cfg) { cfg = new_cfg; }
            
        private:
            config_t cfg;
        };
        
        // Mock Panel GC9A01 configuration
        struct Panel_GC9A01 {
            struct config_t {
                int8_t pin_cs = 22;
                int8_t pin_rst = 4;
                int8_t pin_busy = -1;
                uint16_t memory_width = 240;
                uint16_t memory_height = 240;
                uint16_t panel_width = 240;
                uint16_t panel_height = 240;
                uint8_t offset_x = 0;
                uint8_t offset_y = 0;
                uint8_t offset_rotation = 0;
                uint8_t dummy_read_pixel = 8;
                uint8_t dummy_read_bits = 1;
                bool readable = false;
                bool rgb_order = false;
                bool dlen_16bit = false;
                bool bus_shared = false;
                bool invert = false;
            };
            
            config_t config() { return cfg; }
            void config(const config_t& new_cfg) { cfg = new_cfg; }
            
            void setBus(Bus_SPI* bus) { bus_ = bus; }
            bool init() { return bus_ != nullptr; }
            void startWrite() {}
            void endWrite() {}
            void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const void* data) {}
            
        private:
            config_t cfg;
            Bus_SPI* bus_ = nullptr;
        };
        
        // Mock Light PWM configuration
        struct Light_PWM {
            struct config_t {
                int8_t pin = 3;
                uint8_t pwm_channel = 1;
                uint32_t freq = 1200;
                bool invert = false;
            };
            
            config_t config() { return cfg; }
            void config(const config_t& new_cfg) { cfg = new_cfg; }
            void setBrightness(uint8_t brightness) {}
            
        private:
            config_t cfg;
        };
        
        // Mock LGFX class
        class LGFX_Device {
        public:
            Bus_SPI _bus_instance;
            Panel_GC9A01 _panel_instance;
            Light_PWM _light_instance;
            
            LGFX_Device() {
                _panel_instance.setBus(&_bus_instance);
            }
            
            bool init() { return _panel_instance.init(); }
            void startWrite() { _panel_instance.startWrite(); }
            void endWrite() { _panel_instance.endWrite(); }
            void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const void* data) {
                _panel_instance.pushImage(x, y, w, h, data);
            }
        };
        
    } // namespace v1
} // namespace lgfx

// Type aliases for compatibility
using LGFX = lgfx::v1::LGFX_Device;

// Also create the alias in the lgfx namespace for IDevice inheritance
namespace lgfx {
    using LGFX_Device = v1::LGFX_Device;
}

// Mock constants
const uint32_t SPI2_HOST = 2;
const uint8_t SPI_DMA_CH_AUTO = 3;

#endif // LOVYANGFX_HPP_MOCK