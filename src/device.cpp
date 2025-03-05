#include <device.h>

/// @brief Global instance of the Device instantiated in the constructor used for static callback
Device *g_device_instance = nullptr;

/// @brief Device constructor, initialises the device and sets the bus, panel and light configurations
Device::Device()
{
    g_device_instance = this;

    {
        auto cfg = _bus_instance.config();
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

        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
    }

    {
        auto cfg = _panel_instance.config();

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

#ifdef INVERT // this is causing colour inversion on builds
        cfg.invert = true;
#endif

        _panel_instance.config(cfg);
    }

    {
        auto cfg = _light_instance.config();

        cfg.pin_bl = BL;
        cfg.invert = false;
        cfg.freq = 44100;
        cfg.pwm_channel = 1;

        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
}

/// @brief Initialises the device and setting various screen properties
void Device::prepare()
{
    SerialLogger().log_point("Device::prepare()", "Entry");

    // Initialise screen
    init();
    initDMA();
    startWrite();
    fillScreen(TFT_BLACK);
    setRotation(0);
    setBrightness(SCREEN_DEFAULT_BRIGHTNESS);

    lv_init();

    SerialLogger().log_point("Device::prepare()", "Display configuration...");
    
    // setup screen
    lv_display_t *display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(display, Device::display_flush_wrapper);
    lv_display_set_buffers(display, _lv_buffer[0], _lv_buffer[1], _lv_buffer_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
}

/// @brief static Display Flush Wrapper function
void Device::display_flush_wrapper(lv_display_t *display, const lv_area_t *area, unsigned char *data)
{
    if (g_device_instance != nullptr)
        g_device_instance->display_flush_callback(display, area, data);
}

/// @brief Display Flush Callback function
void Device::display_flush_callback(lv_display_t *display, const lv_area_t *area, unsigned char *data)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);
    lv_draw_sw_rgb565_swap(data, w * h);

    if (g_device_instance->getStartCount() == 0)
        g_device_instance->endWrite();

    g_device_instance->pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (uint16_t *)data);
    lv_disp_flush_ready(display);
}

/// @brief Device destructor to clean up global instance
Device::~Device()
{
    if (g_device_instance == this)
        g_device_instance = nullptr;
}