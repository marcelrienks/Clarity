#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#define LGFX_USE_V1

#include "interfaces/i_device.h"
#include "managers/style_manager.h"
#include "managers/preference_manager.h"

#include <LovyanGFX.hpp>
#include <lvgl.h>

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

class Device : public IDevice
{
public:
    lv_obj_t *Screen;

    static Device &get_instance();
    void prepare() override;

private:
    lgfx::Panel_GC9A01 _panel_instance; // Waveshare Round 1.28inch LCD Display Module
    lgfx::Light_PWM _light_instance;
    lgfx::Bus_SPI _bus_instance;

    const static unsigned int _lv_buffer_size = (SCREEN_WIDTH * 60 * sizeof(lv_color_t)); // Dual buffers at 1/4 screen height
    uint8_t _lv_buffer[2][_lv_buffer_size];

    Device();

    static void display_flush_callback(lv_display_t *display, const lv_area_t *area, unsigned char *data);
};