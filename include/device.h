#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#define LGFX_USE_V1

#include "interfaces/i_device.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include "managers/style_manager.h"
#include "managers/preference_manager.h"

#include <LovyanGFX.hpp>
#include <lvgl.h>
#include <memory>

// Screen
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define SCREEN_OFFSET_X 0
#define SCREEN_OFFSET_Y 0
#define SCREEN_RGB_ORDER false
#define SCREEN_DEFAULT_BRIGHTNESS 100

// Display
#define SPI SPI2_HOST

// Pins
#define SCLK 18
#define MOSI 23
#define MISO -1
#define DC 16
#define CS 22
#define RST 4
#define BL 3
#define BUZZER -1

/**
 * @class Device
 * @brief Hardware abstraction layer for ESP32 with GC9A01 display
 * 
 * @details This class provides the concrete implementation of the IDevice interface,
 * managing the physical display hardware and LVGL integration. It handles the SPI communication
 * with the GC9A01 display controller and provides display buffer management.
 * 
 * @hardware_target ESP32-WROOM-32 with NodeMCU-32S development board
 * @display_target 1.28" round GC9A01 240x240 display (Waveshare compatible)
 * @interface SPI2_HOST with hardware-defined pins
 * @buffer_strategy Dual 60-line buffers for smooth rendering
 * 
 * @design_pattern Dependency Injectable - managed through service container
 * @thread_safety LVGL display callbacks are thread-safe
 * @memory_usage ~57KB for dual display buffers (240*60*2*2 bytes)
 * 
 * @context This is the main hardware interface that all panels and components
 * render to. The display is 240x240 pixels with a round form factor.
 */
class Device : public IDevice
{
public:
    // Constructors and Destructors
    Device();
    Device(const Device &) = delete;
    Device &operator=(const Device &) = delete;

    // Core Functionality Methods
    void prepare() override;
    

    // Public Data Members
    lv_obj_t *screen;

private:

    // Static Methods
    static void display_flush_callback(lv_display_t *display, const lv_area_t *area, unsigned char *data);

    // Instance Data Members
    lgfx::Panel_GC9A01 panelInstance_; // Waveshare Round 1.28inch LCD Display Module
    lgfx::Light_PWM lightInstance_;
    lgfx::Bus_SPI busInstance_;

    const static unsigned int LV_BUFFER_SIZE = (SCREEN_WIDTH * 60 * sizeof(lv_color_t)); // Dual buffers at 1/4 screen height
    uint8_t lvBuffer_[2][LV_BUFFER_SIZE];
};