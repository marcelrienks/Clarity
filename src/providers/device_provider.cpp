#include "providers/device_provider.h"
#include "providers/gpio_provider.h"
#include "providers/lvgl_display_provider.h"
#include <esp32-hal-log.h>

// Constructors and Destructors
/// @brief DeviceProvider constructor, initialises the device and sets the bus, panel and light configurations
DeviceProvider::DeviceProvider()
{
    log_v("DeviceProvider() constructor called");
    {
        auto cfg = busInstance_.config();
        cfg.spi_host = SPI;
        cfg.spi_mode = 0;
        cfg.freq_write = 80000000;
        cfg.freq_read = 20000000;
        cfg.spi_3wire = true;
        cfg.use_lock = true;
        cfg.dma_channel = SPI_DMA_CH_AUTO;
        cfg.pin_sclk = SCLK;
        cfg.pin_mosi = MOSI;
        cfg.pin_miso = MISO;
        cfg.pin_dc = DC;

        busInstance_.config(cfg);
        panelInstance_.setBus(&busInstance_);
    }

    {
        auto cfg = panelInstance_.config();

        cfg.pin_cs = CS;
        cfg.pin_rst = RST;
        cfg.pin_busy = -1;

        cfg.memory_width = SCREEN_WIDTH;
        cfg.memory_height = SCREEN_HEIGHT;
        cfg.panel_width = SCREEN_WIDTH;
        cfg.panel_height = SCREEN_HEIGHT;
        cfg.offset_x = SCREEN_OFFSET_X;
        cfg.offset_y = SCREEN_OFFSET_Y;
        cfg.offset_rotation = 0;
        cfg.dummy_read_pixel = 8;
        cfg.dummy_read_bits = 1;
        cfg.readable = false;
        cfg.rgb_order = SCREEN_RGB_ORDER;
        cfg.dlen_16bit = false;
        cfg.bus_shared = false;

#ifdef INVERT
        cfg.invert = true; // Inverts all colours, background and contents, waveshare 1.28" requires true
#endif

        panelInstance_.config(cfg);
    }

    {
        auto cfg = lightInstance_.config();

        cfg.pin_bl = BL;
        cfg.invert = false;
        cfg.freq = 44100;
        cfg.pwm_channel = 1;

        lightInstance_.config(cfg);
        panelInstance_.setLight(&lightInstance_);
    }

    setPanel(&panelInstance_);
}

// Core Functionality Methods
/// @brief Prepares the device for use by initialising the screen, setting up the display and LVGL
void DeviceProvider::prepare()
{
    log_v("prepare() called");
    log_i("init...");

    init();
    initDMA();
    startWrite();
    setRotation(0);

#ifdef WOKWI_EMULATOR
    // Enable horizontal mirroring to work around wokwi emulator limitation
    // The emulator uses a square ILI9341 display instead of the round GC9A01 target display
    // and renders the image horizontally inverted, also need to correct RGB/BGR order
    writeCommand(0x36); // Memory Access Control
    writeData(0x48);    // Set MX bit (0x40) + BGR bit (0x08) for horizontal mirror with correct colors
#endif

    setBrightness(SCREEN_DEFAULT_BRIGHTNESS);

    lv_init();
    log_i("Display configuration...");

    lv_display_t *display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_user_data(display, this);
    lv_display_set_flush_cb(display, DeviceProvider::display_flush_callback);
    lv_display_set_buffers(display, lvBuffer_[0], lvBuffer_[1], LV_BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Create main screen for display provider
    screen = lv_screen_active();
    
    log_i("DeviceProvider hardware initialization completed successfully");
}

// Static Methods
/// @brief Static Display Flush Wrapper function for LVGL display driver
/// @param display LVGL display object
/// @param area Screen area to flush
/// @param data Pixel data buffer to display
void DeviceProvider::display_flush_callback(lv_display_t *display, const lv_area_t *area, unsigned char *data)
{
    log_v("display_flush_callback() called");
    DeviceProvider *deviceInstance = (DeviceProvider *)lv_display_get_user_data(display);

    uint32_t width = lv_area_get_width(area);
    uint32_t height = lv_area_get_height(area);
    lv_draw_sw_rgb565_swap(data, width * height);

    if (deviceInstance->getStartCount() == 0)
    {
        deviceInstance->endWrite();
    }

    deviceInstance->pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1,
                                 (uint16_t *)data);
    lv_disp_flush_ready(display);
}