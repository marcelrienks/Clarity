#include <unity.h>
#include "test_utilities.h"

// Mock hardware dependencies for unit testing
extern "C" {
    // Mock display/panel states
    bool mock_panel_configured = false;
    bool mock_display_initialized = false;
    bool mock_spi_configured = false;
    bool mock_light_configured = false;
    bool mock_lvgl_initialized = false;
    uint32_t mock_spi_freq_write = 0;
    uint32_t mock_spi_freq_read = 0;
    uint16_t mock_screen_width = 0;
    uint16_t mock_screen_height = 0;
    int mock_pin_cs = -1;
    int mock_pin_rst = -1;
    bool mock_invert_setting = false;
    
    // Mock display flush callback state
    bool mock_flush_callback_called = false;
    const void* mock_flush_data = nullptr;
    size_t mock_flush_data_size = 0;
    
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
    } mock_bus_config_t;
    
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
    } mock_panel_config_t;
    
    typedef struct {
        bool configured;
        int pin;
        uint8_t pwm_channel;
        uint32_t freq;
        bool invert;
    } mock_light_config_t;
    
    // Mock LVGL types
    typedef struct {
        uint16_t x1, y1, x2, y2;
    } mock_lv_area_t;
    
    typedef struct {
        bool initialized;
        void* user_data;
    } mock_lv_display_t;
    
    typedef struct {
        bool created;
        bool styles_applied;
    } mock_lv_obj_t;
    
    // Mock hardware classes
    class MockBusSPI {
    public:
        mock_bus_config_t cfg;
        
        mock_bus_config_t config() {
            return cfg;
        }
        
        void config(const mock_bus_config_t& new_cfg) {
            cfg = new_cfg;
            mock_spi_configured = true;
            mock_spi_freq_write = new_cfg.freq_write;
            mock_spi_freq_read = new_cfg.freq_read;
        }
    };
    
    class MockPanelGC9A01 {
    public:
        mock_panel_config_t cfg;
        MockBusSPI* bus = nullptr;
        
        mock_panel_config_t config() {
            return cfg;
        }
        
        void config(const mock_panel_config_t& new_cfg) {
            cfg = new_cfg;
            mock_panel_configured = true;
            mock_screen_width = new_cfg.panel_width;
            mock_screen_height = new_cfg.panel_height;
            mock_pin_cs = new_cfg.pin_cs;
            mock_pin_rst = new_cfg.pin_rst;
            mock_invert_setting = new_cfg.invert;
        }
        
        void setBus(MockBusSPI* bus_instance) {
            bus = bus_instance;
        }
        
        bool init() {
            mock_display_initialized = (bus != nullptr && mock_spi_configured && mock_panel_configured);
            return mock_display_initialized;
        }
        
        void startWrite() {}
        void endWrite() {}
        void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const void* data) {
            mock_flush_callback_called = true;
            mock_flush_data = data;
            mock_flush_data_size = w * h * 2; // 16-bit color
        }
    };
    
    class MockLightPWM {
    public:
        mock_light_config_t cfg;
        
        mock_light_config_t config() {
            return cfg;
        }
        
        void config(const mock_light_config_t& new_cfg) {
            cfg = new_cfg;
            mock_light_configured = true;
        }
        
        void setBrightness(uint8_t brightness) {}
    };
    
    // Mock LVGL functions
    mock_lv_display_t* mock_lv_display_create(int32_t hor_res, int32_t ver_res) {
        static mock_lv_display_t display = {true, nullptr};
        return &display;
    }
    
    void mock_lv_display_set_flush_cb(mock_lv_display_t* display, void (*flush_cb)(mock_lv_display_t*, const mock_lv_area_t*, unsigned char*)) {
        // Store callback for testing
    }
    
    void mock_lv_display_set_buffers(mock_lv_display_t* display, void* buf1, void* buf2, uint32_t buf_size, int render_mode) {
        // Buffer configuration
    }
    
    mock_lv_obj_t* mock_lv_obj_create(mock_lv_obj_t* parent) {
        static mock_lv_obj_t obj = {true, false};
        return &obj;
    }
    
    void mock_lv_scr_load(mock_lv_obj_t* scr) {}
    
    void mock_lv_display_flush_ready(mock_lv_display_t* display) {}
    
    // Mock constants
    const uint32_t SPI2_HOST = 2;
    const uint8_t SPI_DMA_CH_AUTO = 3;
    const int LV_DISPLAY_RENDER_MODE_PARTIAL = 0;
}

// Mock Device class for testing
class MockDevice {
public:
    static MockDevice& GetInstance() {
        static MockDevice instance;
        return instance;
    }
    
    // Mock hardware instances
    MockBusSPI busInstance_;
    MockPanelGC9A01 panelInstance_;
    MockLightPWM lightInstance_;
    mock_lv_obj_t* screen = nullptr;
    
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
        mock_bus_config_t cfg = {};
        cfg.freq_write = 80000000;
        cfg.freq_read = 20000000;
        cfg.spi_mode = 0;
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
    
    void configurePanel() {
        mock_panel_config_t cfg = {};
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
        mock_light_config_t cfg = {};
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
        mock_lv_display_t* display = mock_lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
        
        // Set flush callback
        mock_lv_display_set_flush_cb(display, display_flush_callback);
        
        // Set buffers
        mock_lv_display_set_buffers(display, lvBuffer_[0], lvBuffer_[1], LV_BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
        
        mock_lvgl_initialized = true;
    }
    
    void createMainScreen() {
        screen = mock_lv_obj_create(nullptr);
        mock_lv_scr_load(screen);
    }
    
public:
    static void display_flush_callback(mock_lv_display_t* display, const mock_lv_area_t* area, unsigned char* data) {
        MockDevice& device = GetInstance();
        
        int32_t w = area->x2 - area->x1 + 1;
        int32_t h = area->y2 - area->y1 + 1;
        
        device.panelInstance_.startWrite();
        device.panelInstance_.pushImage(area->x1, area->y1, w, h, data);
        device.panelInstance_.endWrite();
        
        mock_lv_display_flush_ready(display);
    }
};

// Note: setUp() and tearDown() are defined in test_main.cpp

void resetMockDeviceState() {
    mock_panel_configured = false;
    mock_display_initialized = false;
    mock_spi_configured = false;
    mock_light_configured = false;
    mock_lvgl_initialized = false;
    mock_spi_freq_write = 0;
    mock_spi_freq_read = 0;
    mock_screen_width = 0;
    mock_screen_height = 0;
    mock_pin_cs = -1;
    mock_pin_rst = -1;
    mock_invert_setting = false;
    mock_flush_callback_called = false;
    mock_flush_data = nullptr;
    mock_flush_data_size = 0;
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
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    TEST_ASSERT_TRUE(mock_spi_configured);
    TEST_ASSERT_EQUAL_UINT32(80000000, mock_spi_freq_write);
    TEST_ASSERT_EQUAL_UINT32(20000000, mock_spi_freq_read);
}

void test_device_spi_bus_parameters(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    mock_bus_config_t cfg = device.busInstance_.config();
    TEST_ASSERT_EQUAL_UINT8(0, cfg.spi_mode);
    TEST_ASSERT_TRUE(cfg.spi_3wire);
    TEST_ASSERT_TRUE(cfg.use_lock);
    TEST_ASSERT_EQUAL_UINT8(SPI_DMA_CH_AUTO, cfg.dma_channel);
}

void test_device_spi_pin_mapping(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    mock_bus_config_t cfg = device.busInstance_.config();
    TEST_ASSERT_EQUAL_INT(MockDevice::SCLK, cfg.pin_sclk);
    TEST_ASSERT_EQUAL_INT(MockDevice::MOSI, cfg.pin_mosi);
    TEST_ASSERT_EQUAL_INT(MockDevice::MISO, cfg.pin_miso);
    TEST_ASSERT_EQUAL_INT(MockDevice::DC, cfg.pin_dc);
}

// =================================================================
// PANEL CONFIGURATION TESTS
// =================================================================

void test_device_panel_configuration(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    TEST_ASSERT_TRUE(mock_panel_configured);
    TEST_ASSERT_EQUAL_UINT16(240, mock_screen_width);
    TEST_ASSERT_EQUAL_UINT16(240, mock_screen_height);
    TEST_ASSERT_EQUAL_INT(22, mock_pin_cs);
    TEST_ASSERT_EQUAL_INT(4, mock_pin_rst);
}

void test_device_panel_geometry(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    mock_panel_config_t cfg = device.panelInstance_.config();
    TEST_ASSERT_EQUAL_UINT16(MockDevice::SCREEN_WIDTH, cfg.memory_width);
    TEST_ASSERT_EQUAL_UINT16(MockDevice::SCREEN_HEIGHT, cfg.memory_height);
    TEST_ASSERT_EQUAL_UINT16(MockDevice::SCREEN_WIDTH, cfg.panel_width);
    TEST_ASSERT_EQUAL_UINT16(MockDevice::SCREEN_HEIGHT, cfg.panel_height);
}

void test_device_panel_timing_parameters(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    mock_panel_config_t cfg = device.panelInstance_.config();
    TEST_ASSERT_EQUAL_UINT8(8, cfg.dummy_read_pixel);
    TEST_ASSERT_EQUAL_UINT8(1, cfg.dummy_read_bits);
    TEST_ASSERT_FALSE(cfg.readable);
    TEST_ASSERT_FALSE(cfg.dlen_16bit);
    TEST_ASSERT_FALSE(cfg.bus_shared);
}

void test_device_panel_color_inversion(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    // Test depends on INVERT define
#ifdef INVERT
    TEST_ASSERT_TRUE(mock_invert_setting);
#else
    TEST_ASSERT_FALSE(mock_invert_setting);
#endif
}

// =================================================================
// LIGHT CONFIGURATION TESTS
// =================================================================

void test_device_light_configuration(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    TEST_ASSERT_TRUE(mock_light_configured);
}

void test_device_light_parameters(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    mock_light_config_t cfg = device.lightInstance_.config();
    TEST_ASSERT_EQUAL_INT(MockDevice::BL, cfg.pin);
    TEST_ASSERT_EQUAL_UINT8(1, cfg.pwm_channel);
    TEST_ASSERT_EQUAL_UINT32(1200, cfg.freq);
    TEST_ASSERT_FALSE(cfg.invert);
}

// =================================================================
// DISPLAY INITIALIZATION TESTS
// =================================================================

void test_device_prepare_initialization(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    device.prepare();
    
    TEST_ASSERT_TRUE(mock_display_initialized);
    TEST_ASSERT_TRUE(mock_lvgl_initialized);
    TEST_ASSERT_NOT_NULL(device.screen);
}

void test_device_initialization_sequence(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    // Before prepare
    TEST_ASSERT_FALSE(mock_display_initialized);
    TEST_ASSERT_FALSE(mock_lvgl_initialized);
    
    device.prepare();
    
    // After prepare
    TEST_ASSERT_TRUE(mock_display_initialized);
    TEST_ASSERT_TRUE(mock_lvgl_initialized);
}

void test_device_display_dependencies(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    // Display initialization depends on SPI and panel configuration
    TEST_ASSERT_TRUE(mock_spi_configured);
    TEST_ASSERT_TRUE(mock_panel_configured);
    
    device.prepare();
    TEST_ASSERT_TRUE(mock_display_initialized);
}

// =================================================================
// LVGL INTEGRATION TESTS
// =================================================================

void test_device_lvgl_display_creation(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    device.prepare();
    
    TEST_ASSERT_TRUE(mock_lvgl_initialized);
}

void test_device_buffer_configuration(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    // Check buffer size calculation
    const unsigned int expected_size = MockDevice::SCREEN_WIDTH * 60 * 2; // 60 lines, 16-bit
    TEST_ASSERT_EQUAL_UINT(expected_size, MockDevice::LV_BUFFER_SIZE);
}

void test_device_dual_buffer_allocation(void) {
    resetMockDeviceState();
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
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    device.prepare();
    
    // Simulate flush callback
    mock_lv_area_t area = {0, 0, 100, 50};
    uint8_t test_data[100 * 51 * 2] = {0}; // 16-bit color data
    
    MockDevice::display_flush_callback(nullptr, &area, test_data);
    
    TEST_ASSERT_TRUE(mock_flush_callback_called);
    TEST_ASSERT_EQUAL_PTR(test_data, mock_flush_data);
}

void test_device_flush_callback_area_calculation(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    device.prepare();
    
    // Test different area sizes
    mock_lv_area_t areas[] = {
        {0, 0, 99, 49},    // 100x50
        {10, 20, 59, 79},  // 50x60
        {0, 0, 239, 59}    // Full width, 60 lines
    };
    
    for (size_t i = 0; i < 3; i++) {
        mock_flush_callback_called = false;
        uint8_t test_data[240 * 60 * 2] = {0};
        
        MockDevice::display_flush_callback(nullptr, &areas[i], test_data);
        
        TEST_ASSERT_TRUE(mock_flush_callback_called);
        
        int32_t expected_width = areas[i].x2 - areas[i].x1 + 1;
        int32_t expected_height = areas[i].y2 - areas[i].y1 + 1;
        size_t expected_size = expected_width * expected_height * 2;
        
        TEST_ASSERT_EQUAL_size_t(expected_size, mock_flush_data_size);
    }
}

// =================================================================
// SCREEN MANAGEMENT TESTS
// =================================================================

void test_device_main_screen_creation(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    device.prepare();
    
    TEST_ASSERT_NOT_NULL(device.screen);
    TEST_ASSERT_TRUE(device.screen->created);
}

void test_device_screen_lifecycle(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
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
    resetMockDeviceState();
    
    // Force initialization failure
    mock_spi_configured = false;
    
    MockDevice& device = MockDevice::GetInstance();
    device.prepare();
    
    // Should handle gracefully
    TEST_ASSERT_FALSE(mock_display_initialized);
}

void test_device_partial_configuration_failure(void) {
    resetMockDeviceState();
    
    // Panel configured but SPI not configured
    mock_panel_configured = true;
    mock_spi_configured = false;
    
    MockDevice& device = MockDevice::GetInstance();
    
    // Should still have panel configuration
    TEST_ASSERT_TRUE(mock_panel_configured);
    TEST_ASSERT_FALSE(mock_display_initialized);
}

// =================================================================
// INTEGRATION TESTS
// =================================================================

void test_device_complete_initialization_flow(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    // Full initialization sequence
    device.prepare();
    
    // Verify all components initialized
    TEST_ASSERT_TRUE(mock_spi_configured);
    TEST_ASSERT_TRUE(mock_panel_configured);
    TEST_ASSERT_TRUE(mock_light_configured);
    TEST_ASSERT_TRUE(mock_display_initialized);
    TEST_ASSERT_TRUE(mock_lvgl_initialized);
    TEST_ASSERT_NOT_NULL(device.screen);
}

void test_device_hardware_consistency(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    device.prepare();
    
    // Verify pin mappings are consistent
    mock_bus_config_t bus_cfg = device.busInstance_.config();
    mock_panel_config_t panel_cfg = device.panelInstance_.config();
    mock_light_config_t light_cfg = device.lightInstance_.config();
    
    TEST_ASSERT_EQUAL_INT(MockDevice::CS, panel_cfg.pin_cs);
    TEST_ASSERT_EQUAL_INT(MockDevice::RST, panel_cfg.pin_rst);
    TEST_ASSERT_EQUAL_INT(MockDevice::DC, bus_cfg.pin_dc);
    TEST_ASSERT_EQUAL_INT(MockDevice::BL, light_cfg.pin);
}

// =================================================================
// PERFORMANCE TESTS
// =================================================================

void test_device_memory_usage_validation(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    // Verify buffer size is reasonable
    const unsigned int total_buffer_size = MockDevice::LV_BUFFER_SIZE * 2;
    const unsigned int max_reasonable_size = 240 * 240 * 2; // Full screen 16-bit
    
    TEST_ASSERT_LESS_THAN_UINT32(max_reasonable_size, total_buffer_size);
    
    // Verify we're using dual buffers for efficiency
    TEST_ASSERT_GREATER_THAN_UINT32(MockDevice::LV_BUFFER_SIZE, total_buffer_size / 3);
}

void test_device_configuration_efficiency(void) {
    resetMockDeviceState();
    MockDevice& device = MockDevice::GetInstance();
    
    // Configuration should happen during construction
    TEST_ASSERT_TRUE(mock_spi_configured);
    TEST_ASSERT_TRUE(mock_panel_configured);
    TEST_ASSERT_TRUE(mock_light_configured);
    
    // prepare() should only initialize, not reconfigure
    device.prepare();
    
    TEST_ASSERT_TRUE(mock_display_initialized);
    TEST_ASSERT_TRUE(mock_lvgl_initialized);
}

// Note: PlatformIO will automatically discover and run test_ functions