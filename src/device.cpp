#include <device.h>

Device::Device(void)
{
    {
        auto busConfig = _busInstance.config();
        busConfig.spi_host = SPI;
        busConfig.spi_mode = 0;
        busConfig.freq_write = 80000000;
        busConfig.freq_read = 20000000;
        busConfig.spi_3wire = true;
        busConfig.use_lock = true;
        busConfig.dma_channel = SPI_DMA_CH_AUTO;
        busConfig.pin_sclk = SCLK;
        busConfig.pin_mosi = MOSI;
        busConfig.pin_miso = MISO;
        busConfig.pin_dc = DC;

        _busInstance.config(busConfig);
        _panelInstance.setBus(&_busInstance);
    }

    {
        auto panelConfig = _panelInstance.config();
        panelConfig.pin_cs = CS;
        panelConfig.pin_rst = RST;
        panelConfig.pin_busy = -1;
        panelConfig.memory_width = SCREEN_WIDTH;
        panelConfig.memory_height = SCREEN_HEIGHT;
        panelConfig.panel_width = SCREEN_WIDTH;
        panelConfig.panel_height = SCREEN_HEIGHT;
        panelConfig.offset_x = SCREEN_OFFSET_X;
        panelConfig.offset_y = SCREEN_OFFSET_Y;
        panelConfig.offset_rotation = 0;
        panelConfig.dummy_read_pixel = 8;
        panelConfig.dummy_read_bits = 1;
        panelConfig.readable = false;
        panelConfig.invert = true;
        panelConfig.rgb_order = SCREEN_RGB_ORDER;
        panelConfig.dlen_16bit = false;
        panelConfig.bus_shared = false;

        _panelInstance.config(panelConfig);
    }

    {
        auto lightConfig = _lightInstance.config();
        lightConfig.pin_bl = BL;
        lightConfig.invert = false;
        lightConfig.freq = 44100;
        lightConfig.pwm_channel = 1;

        _lightInstance.config(lightConfig);
        _panelInstance.setLight(&_lightInstance);
    }

    setPanel(&_panelInstance);
}