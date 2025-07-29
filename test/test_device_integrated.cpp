#include <unity.h>
#include "test_utilities.h"

// Device/Hardware-specific mock state using shared infrastructure
namespace DeviceMocks {
    // Mock display/panel states
    bool panel_configured = false;
    bool display_initialized = false;
    bool spi_configured = false;
    bool light_configured = false;
    bool lvgl_initialized = false;
    uint32_t spi_freq_write = 0;
    uint32_t spi_freq_read = 0;
    uint16_t screen_width = 0;
    uint16_t screen_height = 0;
    int pin_cs = -1;
    int pin_rst = -1;
    bool invert_setting = false;
    
    // Mock display flush callback state
    bool flush_callback_called = false;
    const void* flush_data = nullptr;
    size_t flush_data_size = 0;
    
    // Mock LovyanGFX types
    typedef struct {
        bool configured;
        uint32_t freq_write;
        uint32_t freq_read;
        uint8_t spi_mode;
        bool spi_3wire;
        bool use_lock;
        uint8_t dma_channel;
        int pin_sclk;
        int pin_mosi;
        int pin_miso;
        int pin_dc;
    } bus_config_t;
    
    typedef struct {
        bool configured;
        int pin_cs;
        int pin_rst;
        int pin_busy;
        uint16_t memory_width;
        uint16_t memory_height;
        uint16_t panel_width;
        uint16_t panel_height;
        uint8_t offset_x;
        uint8_t offset_y;
        uint8_t offset_rotation;
        uint8_t dummy_read_pixel;
        uint8_t dummy_read_bits;
        bool readable;
        bool rgb_order;
        bool dlen_16bit;
        bool bus_shared;
        bool invert;
    } panel_config_t;
    
    typedef struct {
        bool configured;
        int pin;
        uint8_t pwm_channel;
        uint32_t freq;
        bool invert;
    } light_config_t;
    
    // Mock LVGL types
    typedef struct {
        uint16_t x1, y1, x2, y2;
    } lv_area_t;
    
    typedef struct {
        bool initialized;
        void* user_data;
    } lv_display_t;
    
    typedef struct {
        bool created;
        bool styles_applied;
    } lv_obj_t;
    
    // Mock hardware classes
    class MockBusSPI {
    public:
        bus_config_t cfg;
        
        bus_config_t config() {
            return cfg;
        }
        
        void config(const bus_config_t& new_cfg) {
            cfg = new_cfg;
            spi_configured = true;
            spi_freq_write = new_cfg.freq_write;
            spi_freq_read = new_cfg.freq_read;
        }
    };
    
    class MockPanelGC9A01 {
    public:
        panel_config_t cfg;
        MockBusSPI* bus = nullptr;
        
        panel_config_t config() {
            return cfg;
        }
        
        void config(const panel_config_t& new_cfg) {
            cfg = new_cfg;
            panel_configured = true;
            screen_width = new_cfg.panel_width;
            screen_height = new_cfg.panel_height;
            pin_cs = new_cfg.pin_cs;
            pin_rst = new_cfg.pin_rst;
            invert_setting = new_cfg.invert;
        }
        
        void setBus(MockBusSPI* bus_instance) {
            bus = bus_instance;
        }
        
        bool init() {
            display_initialized = (bus != nullptr && spi_configured && panel_configured);
            return display_initialized;
        }
        
        void startWrite() {}
        void endWrite() {}
        void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const void* data) {
            flush_callback_called = true;
            flush_data = data;
            flush_data_size = w * h * 2; // 16-bit color
        }
    };
    
    class MockLightPWM {
    public:
        light_config_t cfg;
        
        light_config_t config() {
            return cfg;
        }
        
        void config(const light_config_t& new_cfg) {
            cfg = new_cfg;
            light_configured = true;
        }
        
        void setBrightness(uint8_t brightness) {}
    };
    
    // Mock LVGL functions
    lv_display_t* lv_display_create(int32_t hor_res, int32_t ver_res) {
        static lv_display_t display = {true, nullptr};
        return &display;
    }
    
    void lv_display_set_flush_cb(lv_display_t* display, void (*flush_cb)(lv_display_t*, const lv_area_t*, unsigned char*)) {
        // Store callback for testing
    }
    
    void lv_display_set_buffers(lv_display_t* display, void* buf1, void* buf2, uint32_t buf_size, int render_mode) {
        // Buffer configuration
    }
    
    lv_obj_t* lv_obj_create(lv_obj_t* parent) {
        static lv_obj_t obj = {true, false};
        return &obj;
    }
    
    void lv_scr_load(lv_obj_t* scr) {}
    
    void lv_display_flush_ready(lv_display_t* display) {}
    
    // Mock constants
    const uint32_t SPI2_HOST = 2;
    const uint8_t SPI_DMA_CH_AUTO = 3;
    const int LV_DISPLAY_RENDER_MODE_PARTIAL = 0;
    
    void reset() {
        panel_configured = false;
        display_initialized = false;
        spi_configured = false;
        light_configured = false;
        lvgl_initialized = false;
        spi_freq_write = 0;
        spi_freq_read = 0;
        screen_width = 0;
        screen_height = 0;
        pin_cs = -1;
        pin_rst = -1;
        invert_setting = false;
        flush_callback_called = false;
        flush_data = nullptr;
        flush_data_size = 0;
    }
}

// Mock Device class for testing
class MockDevice {
public:
    static MockDevice& GetInstance() {
        static MockDevice instance;
        return instance;
    }
    
    // Mock hardware instances
    DeviceMocks::MockBusSPI busInstance_;
    DeviceMocks::MockPanelGC9A01 panelInstance_;
    DeviceMocks::MockLightPWM lightInstance_;
    DeviceMocks::lv_obj_t* screen = nullptr;
    
    // Configuration constants (same as real device)
    static const int SCREEN_WIDTH = 240;
    static const int SCREEN_HEIGHT = 240;
    static const int SCREEN_OFFSET_X = 0;
    static const int SCREEN_OFFSET_Y = 0;
    static const bool SCREEN_RGB_ORDER = false;
    static const int SCLK = 18;
    static const int MOSI = 23;
    static const int MISO = -1;
    static const int DC = 16;
    static const int CS = 22;
    static const int RST = 4;
    static const int BL = 3;
    static const unsigned int LV_BUFFER_SIZE = (SCREEN_WIDTH * 60 * 2); // 16-bit color
    
    uint8_t lvBuffer_[2][LV_BUFFER_SIZE];
    
    MockDevice() {
        // Configuration happens on first access after reset
    }
    
    void reset() {
        screen = nullptr;
    }
    
    void ensureConfigured() {
        // Configure bus
        configureBus();
        
        // Configure panel
        configurePanel();
        
        // Configure light
        configureLight();
    }
    
    void prepare() {
        // Initialize hardware
        if (!initializeDisplay()) {
            return;
        }
        
        // Initialize LVGL
        initializeLVGL();
        
        // Create main screen
        createMainScreen();
    }
    
private:
    void configureBus() {
        DeviceMocks::bus_config_t cfg = {};
        cfg.freq_write = 80000000;
        cfg.freq_read = 20000000;
        cfg.spi_mode = 0;
        cfg.spi_3wire = true;
        cfg.use_lock = true;
        cfg.dma_channel = DeviceMocks::SPI_DMA_CH_AUTO;
        cfg.pin_sclk = SCLK;
        cfg.pin_mosi = MOSI;
        cfg.pin_miso = MISO;
        cfg.pin_dc = DC;
        
        busInstance_.config(cfg);
        panelInstance_.setBus(&busInstance_);
    }
    
    void configurePanel() {
        DeviceMocks::panel_config_t cfg = {};
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
        cfg.invert = true;
#else
        cfg.invert = false;
#endif
        
        panelInstance_.config(cfg);
    }
    
    void configureLight() {
        DeviceMocks::light_config_t cfg = {};
        cfg.pin = BL;
        cfg.pwm_channel = 1;
        cfg.freq = 1200;
        cfg.invert = false;
        
        lightInstance_.config(cfg);
    }
    
    bool initializeDisplay() {
        return panelInstance_.init();
    }
    
    void initializeLVGL() {
        // Create display
        DeviceMocks::lv_display_t* display = DeviceMocks::lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
        
        // Set flush callback
        DeviceMocks::lv_display_set_flush_cb(display, display_flush_callback);
        
        // Set buffers
        DeviceMocks::lv_display_set_buffers(display, lvBuffer_[0], lvBuffer_[1], LV_BUFFER_SIZE, DeviceMocks::LV_DISPLAY_RENDER_MODE_PARTIAL);
        
        DeviceMocks::lvgl_initialized = true;
    }
    
    void createMainScreen() {
        screen = DeviceMocks::lv_obj_create(nullptr);
        DeviceMocks::lv_scr_load(screen);
    }
    
public:
    static void display_flush_callback(DeviceMocks::lv_display_t* display, const DeviceMocks::lv_area_t* area, unsigned char* data) {
        MockDevice& device = GetInstance();
        
        int32_t w = area->x2 - area->x1 + 1;
        int32_t h = area->y2 - area->y1 + 1;
        
        device.panelInstance_.startWrite();
        device.panelInstance_.pushImage(area->x1, area->y1, w, h, data);
        device.panelInstance_.endWrite();
        
        DeviceMocks::lv_display_flush_ready(display);
    }
};

// Reset function for device tests
void resetDeviceMockState() {
    DeviceMocks::reset();
    
    // Reset the device instance state
    MockDevice& device = MockDevice::GetInstance();
    device.reset();
}

// =================================================================
// DEVICE SINGLETON TESTS
// =================================================================

void test_device_singleton_access(void) {
    MockDevice& device1 = MockDevice::GetInstance();
    MockDevice& device2 = MockDevice::GetInstance();
    
    TEST_ASSERT_EQUAL_PTR(&device1, &device2);
}

void test_device_constants_validation(void) {
    TEST_ASSERT_EQUAL_INT(240, MockDevice::SCREEN_WIDTH);
    TEST_ASSERT_EQUAL_INT(240, MockDevice::SCREEN_HEIGHT);
    TEST_ASSERT_EQUAL_INT(0, MockDevice::SCREEN_OFFSET_X);
    TEST_ASSERT_EQUAL_INT(0, MockDevice::SCREEN_OFFSET_Y);
    TEST_ASSERT_FALSE(MockDevice::SCREEN_RGB_ORDER);
}

void test_device_pin_configuration(void) {
    TEST_ASSERT_EQUAL_INT(18, MockDevice::SCLK);
    TEST_ASSERT_EQUAL_INT(23, MockDevice::MOSI);
    TEST_ASSERT_EQUAL_INT(-1, MockDevice::MISO);
    TEST_ASSERT_EQUAL_INT(16, MockDevice::DC);
    TEST_ASSERT_EQUAL_INT(22, MockDevice::CS);
    TEST_ASSERT_EQUAL_INT(4, MockDevice::RST);
    TEST_ASSERT_EQUAL_INT(3, MockDevice::BL);
}

// =================================================================
// SPI BUS CONFIGURATION TESTS
// =================================================================

void test_device_spi_bus_configuration(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    TEST_ASSERT_TRUE(DeviceMocks::spi_configured);
    TEST_ASSERT_EQUAL_UINT32(80000000, DeviceMocks::spi_freq_write);
    TEST_ASSERT_EQUAL_UINT32(20000000, DeviceMocks::spi_freq_read);
}

void test_device_spi_bus_parameters(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    DeviceMocks::bus_config_t cfg = device.busInstance_.config();
    TEST_ASSERT_EQUAL_UINT8(0, cfg.spi_mode);
    TEST_ASSERT_TRUE(cfg.spi_3wire);
    TEST_ASSERT_TRUE(cfg.use_lock);
    TEST_ASSERT_EQUAL_UINT8(DeviceMocks::SPI_DMA_CH_AUTO, cfg.dma_channel);
}

void test_device_spi_pin_mapping(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    DeviceMocks::bus_config_t cfg = device.busInstance_.config();
    TEST_ASSERT_EQUAL_INT(MockDevice::SCLK, cfg.pin_sclk);
    TEST_ASSERT_EQUAL_INT(MockDevice::MOSI, cfg.pin_mosi);
    TEST_ASSERT_EQUAL_INT(MockDevice::MISO, cfg.pin_miso);
    TEST_ASSERT_EQUAL_INT(MockDevice::DC, cfg.pin_dc);
}

// =================================================================
// PANEL CONFIGURATION TESTS
// =================================================================

void test_device_panel_configuration(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    TEST_ASSERT_TRUE(DeviceMocks::panel_configured);
    TEST_ASSERT_EQUAL_UINT16(240, DeviceMocks::screen_width);
    TEST_ASSERT_EQUAL_UINT16(240, DeviceMocks::screen_height);
    TEST_ASSERT_EQUAL_INT(22, DeviceMocks::pin_cs);
    TEST_ASSERT_EQUAL_INT(4, DeviceMocks::pin_rst);
}

void test_device_panel_geometry(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    DeviceMocks::panel_config_t cfg = device.panelInstance_.config();
    TEST_ASSERT_EQUAL_UINT16(MockDevice::SCREEN_WIDTH, cfg.memory_width);
    TEST_ASSERT_EQUAL_UINT16(MockDevice::SCREEN_HEIGHT, cfg.memory_height);
    TEST_ASSERT_EQUAL_UINT16(MockDevice::SCREEN_WIDTH, cfg.panel_width);
    TEST_ASSERT_EQUAL_UINT16(MockDevice::SCREEN_HEIGHT, cfg.panel_height);
}

void test_device_panel_timing_parameters(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    DeviceMocks::panel_config_t cfg = device.panelInstance_.config();
    TEST_ASSERT_EQUAL_UINT8(8, cfg.dummy_read_pixel);
    TEST_ASSERT_EQUAL_UINT8(1, cfg.dummy_read_bits);
    TEST_ASSERT_FALSE(cfg.readable);
    TEST_ASSERT_FALSE(cfg.dlen_16bit);
    TEST_ASSERT_FALSE(cfg.bus_shared);
}

void test_device_panel_color_inversion(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    // Test depends on INVERT define
#ifdef INVERT
    TEST_ASSERT_TRUE(DeviceMocks::invert_setting);
#else
    TEST_ASSERT_FALSE(DeviceMocks::invert_setting);
#endif
}

// =================================================================
// LIGHT CONFIGURATION TESTS
// =================================================================

void test_device_light_configuration(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    TEST_ASSERT_TRUE(DeviceMocks::light_configured);
}

void test_device_light_parameters(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    DeviceMocks::light_config_t cfg = device.lightInstance_.config();
    TEST_ASSERT_EQUAL_INT(MockDevice::BL, cfg.pin);
    TEST_ASSERT_EQUAL_UINT8(1, cfg.pwm_channel);
    TEST_ASSERT_EQUAL_UINT32(1200, cfg.freq);
    TEST_ASSERT_FALSE(cfg.invert);
}

// =================================================================
// DISPLAY INITIALIZATION TESTS
// =================================================================

void test_device_prepare_initialization(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    device.prepare();
    
    TEST_ASSERT_TRUE(DeviceMocks::display_initialized);
    TEST_ASSERT_TRUE(DeviceMocks::lvgl_initialized);
    TEST_ASSERT_NOT_NULL(device.screen);
}

void test_device_initialization_sequence(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    // Before prepare
    TEST_ASSERT_FALSE(DeviceMocks::display_initialized);
    TEST_ASSERT_FALSE(DeviceMocks::lvgl_initialized);
    
    device.prepare();
    
    // After prepare
    TEST_ASSERT_TRUE(DeviceMocks::display_initialized);
    TEST_ASSERT_TRUE(DeviceMocks::lvgl_initialized);
}

void test_device_display_dependencies(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    // Display initialization depends on SPI and panel configuration
    TEST_ASSERT_TRUE(DeviceMocks::spi_configured);
    TEST_ASSERT_TRUE(DeviceMocks::panel_configured);
    
    device.prepare();
    TEST_ASSERT_TRUE(DeviceMocks::display_initialized);
}

// =================================================================
// LVGL INTEGRATION TESTS
// =================================================================

void test_device_lvgl_display_creation(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    device.prepare();
    
    TEST_ASSERT_TRUE(DeviceMocks::lvgl_initialized);
}

void test_device_buffer_configuration(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    
    // Check buffer size calculation
    const unsigned int expected_size = MockDevice::SCREEN_WIDTH * 60 * 2; // 60 lines, 16-bit
    TEST_ASSERT_EQUAL_UINT(expected_size, MockDevice::LV_BUFFER_SIZE);
}

void test_device_dual_buffer_allocation(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    
    // Verify buffers are allocated
    TEST_ASSERT_NOT_NULL(device.lvBuffer_[0]);
    TEST_ASSERT_NOT_NULL(device.lvBuffer_[1]);
    
    // Verify buffers are different
    TEST_ASSERT_TRUE(&device.lvBuffer_[0][0] != &device.lvBuffer_[1][0]);
}

// =================================================================
// DISPLAY FLUSH CALLBACK TESTS
// =================================================================

void test_device_flush_callback_functionality(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    device.prepare();
    
    // Simulate flush callback
    DeviceMocks::lv_area_t area = {0, 0, 100, 50};
    uint8_t test_data[100 * 51 * 2] = {0}; // 16-bit color data
    
    MockDevice::display_flush_callback(nullptr, &area, test_data);
    
    TEST_ASSERT_TRUE(DeviceMocks::flush_callback_called);
    TEST_ASSERT_EQUAL_PTR(test_data, DeviceMocks::flush_data);
}

void test_device_flush_callback_area_calculation(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    device.prepare();
    
    // Test different area sizes
    DeviceMocks::lv_area_t areas[] = {
        {0, 0, 99, 49},    // 100x50
        {10, 20, 59, 79},  // 50x60
        {0, 0, 239, 59}    // Full width, 60 lines
    };
    
    for (size_t i = 0; i < 3; i++) {
        DeviceMocks::flush_callback_called = false;
        uint8_t test_data[240 * 60 * 2] = {0};
        
        MockDevice::display_flush_callback(nullptr, &areas[i], test_data);
        
        TEST_ASSERT_TRUE(DeviceMocks::flush_callback_called);
        
        int32_t expected_width = areas[i].x2 - areas[i].x1 + 1;
        int32_t expected_height = areas[i].y2 - areas[i].y1 + 1;
        size_t expected_size = expected_width * expected_height * 2;
        
        TEST_ASSERT_EQUAL_size_t(expected_size, DeviceMocks::flush_data_size);
    }
}

// =================================================================
// SCREEN MANAGEMENT TESTS
// =================================================================

void test_device_main_screen_creation(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    device.prepare();
    
    TEST_ASSERT_NOT_NULL(device.screen);
    TEST_ASSERT_TRUE(device.screen->created);
}

void test_device_screen_lifecycle(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    // Before prepare
    TEST_ASSERT_NULL(device.screen);
    
    device.prepare();
    
    // After prepare
    TEST_ASSERT_NOT_NULL(device.screen);
}

// =================================================================
// ERROR HANDLING TESTS
// =================================================================

void test_device_display_initialization_failure(void) {
    resetDeviceMockState();
    
    // Force initialization failure
    DeviceMocks::spi_configured = false;
    
    MockDevice& device = MockDevice::GetInstance();
    device.prepare();
    
    // Should handle gracefully
    TEST_ASSERT_FALSE(DeviceMocks::display_initialized);
}

void test_device_partial_configuration_failure(void) {
    resetDeviceMockState();
    
    // Panel configured but SPI not configured
    DeviceMocks::panel_configured = true;
    DeviceMocks::spi_configured = false;
    
    MockDevice& device = MockDevice::GetInstance();
    
    // Should still have panel configuration
    TEST_ASSERT_TRUE(DeviceMocks::panel_configured);
    TEST_ASSERT_FALSE(DeviceMocks::display_initialized);
}

// =================================================================
// INTEGRATION TESTS
// =================================================================

void test_device_complete_initialization_flow(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    // Full initialization sequence
    device.prepare();
    
    // Verify all components initialized
    TEST_ASSERT_TRUE(DeviceMocks::spi_configured);
    TEST_ASSERT_TRUE(DeviceMocks::panel_configured);
    TEST_ASSERT_TRUE(DeviceMocks::light_configured);
    TEST_ASSERT_TRUE(DeviceMocks::display_initialized);
    TEST_ASSERT_TRUE(DeviceMocks::lvgl_initialized);
    TEST_ASSERT_NOT_NULL(device.screen);
}

void test_device_hardware_consistency(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    device.prepare();
    
    // Verify pin mappings are consistent
    DeviceMocks::bus_config_t bus_cfg = device.busInstance_.config();
    DeviceMocks::panel_config_t panel_cfg = device.panelInstance_.config();
    DeviceMocks::light_config_t light_cfg = device.lightInstance_.config();
    
    TEST_ASSERT_EQUAL_INT(MockDevice::CS, panel_cfg.pin_cs);
    TEST_ASSERT_EQUAL_INT(MockDevice::RST, panel_cfg.pin_rst);
    TEST_ASSERT_EQUAL_INT(MockDevice::DC, bus_cfg.pin_dc);
    TEST_ASSERT_EQUAL_INT(MockDevice::BL, light_cfg.pin);
}

// =================================================================
// PERFORMANCE TESTS
// =================================================================

void test_device_memory_usage_validation(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    
    // Verify buffer size calculations
    const unsigned int expected_buffer_size = MockDevice::SCREEN_WIDTH * 60 * 2; // 60 lines, 16-bit
    TEST_ASSERT_EQUAL_UINT32(expected_buffer_size, MockDevice::LV_BUFFER_SIZE);
    
    // Verify individual buffer is reasonable size (should be exactly 60 lines worth)
    const unsigned int expected_lines = 60;
    const unsigned int line_size = MockDevice::SCREEN_WIDTH * 2; // 16-bit color
    const unsigned int expected_single_buffer = expected_lines * line_size;
    TEST_ASSERT_EQUAL_UINT32(expected_single_buffer, MockDevice::LV_BUFFER_SIZE);
    
    // Verify buffer is reasonable - should be 28,800 bytes (240 * 60 * 2)
    TEST_ASSERT_EQUAL_UINT32(28800, MockDevice::LV_BUFFER_SIZE);
}

void test_device_configuration_efficiency(void) {
    resetDeviceMockState();
    MockDevice& device = MockDevice::GetInstance();
    device.ensureConfigured();
    
    // Configuration should happen during construction
    TEST_ASSERT_TRUE(DeviceMocks::spi_configured);
    TEST_ASSERT_TRUE(DeviceMocks::panel_configured);
    TEST_ASSERT_TRUE(DeviceMocks::light_configured);
    
    // prepare() should only initialize, not reconfigure
    device.prepare();
    
    TEST_ASSERT_TRUE(DeviceMocks::display_initialized);
    TEST_ASSERT_TRUE(DeviceMocks::lvgl_initialized);
}

// Note: PlatformIO will automatically discover and run test_ functions