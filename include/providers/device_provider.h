#pragma once

#define LGFX_USE_V1

#include <LovyanGFX.hpp>
#include <lvgl.h>
#include "interfaces/i_device_provider.h"

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
 * @class DeviceProvider
 * @brief Hardware abstraction provider for ESP32 with GC9A01 display
 *
 * @details This class provides the concrete implementation of the display provider interface,
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
class DeviceProvider : public lgfx::LGFX_Device, public IDeviceProvider
{
public:
    // ========== Constructors and Destructor ==========
    /**
     * @brief Construct DeviceProvider and configure hardware components
     * @details Sets up SPI bus, GC9A01 panel, and backlight configurations
     */
    DeviceProvider();
    DeviceProvider(const DeviceProvider &) = delete;
    DeviceProvider &operator=(const DeviceProvider &) = delete;
    /**
     * @brief Default destructor
     */
    ~DeviceProvider() = default;

    // ========== Public Interface Methods ==========
    /**
     * @brief Initialize display hardware and LVGL subsystem
     * @details Performs full display initialization, LVGL setup, and screen creation
     */
    void prepare() override;

    /**
     * @brief Get the active LVGL screen object
     * @return Pointer to the main LVGL screen object
     */
    lv_obj_t* GetScreen() const override;

    /**
     * @brief Check if display provider is ready for use
     * @return true if display is initialized and screen is available
     */
    bool IsReady() const override;

private:
    // ========== Static Methods ==========
    /**
     * @brief LVGL display flush callback for rendering pixel data to hardware
     * @param display LVGL display object
     * @param area Screen area to render
     * @param data Pixel data buffer to flush to display
     * @details Handles RGB565 byte swapping and DMA transfer to GC9A01
     */
    static void display_flush_callback(lv_display_t *display, const lv_area_t *area, unsigned char *data);

    // ========== Private Data Members ==========
    lv_obj_t *screen_;

    lgfx::Panel_GC9A01 panelInstance_; // Waveshare Round 1.28inch LCD Display Module
    lgfx::Light_PWM lightInstance_;
    lgfx::Bus_SPI busInstance_;

    // Optimized buffer configuration for memory efficiency
    // Using 40 lines (1/6 of screen) reduces memory usage from ~57KB to ~38KB
    // while maintaining smooth rendering performance
    static constexpr unsigned int BUFFER_LINE_COUNT = 40; // Reduced from 60 for memory optimization
    static constexpr unsigned int LV_BUFFER_SIZE =
        (SCREEN_WIDTH * BUFFER_LINE_COUNT * sizeof(lv_color_t));
    uint8_t lvBuffer_[2][LV_BUFFER_SIZE];
};