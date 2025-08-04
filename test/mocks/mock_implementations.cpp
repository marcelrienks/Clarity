#ifdef UNIT_TESTING

#include "Arduino.h"
#include "nvs_flash.h"
#include "factories/ui_factory.h"
#include "interfaces/i_component.h"
#include "interfaces/i_panel.h"
#include "utilities/types.h"
#include <functional>

// Mock Arduino global instances
MockSerial Serial;
MockSPI SPI;

// Mock component for testing
class MockComponent : public IComponent {
public:
    void render(lv_obj_t *screen, const ComponentLocation& location, IDisplayProvider* display) override {}
    void refresh(const Reading& reading) override {}
    void setValue(int32_t value) override {}
};

// Mock panel for testing
class MockPanel : public IPanel {
private:
    std::string panelName;
    
public:
    MockPanel(const std::string& name = "") : panelName(name) {}
    
    void init(IGpioProvider *gpio, IDisplayProvider *display) override {}
    void load(std::function<void()> callbackFunction, IGpioProvider *gpio, IDisplayProvider *display) override {
        if (callbackFunction) callbackFunction();
    }
    void update(std::function<void()> callbackFunction, IGpioProvider *gpio, IDisplayProvider *display) override {
        if (callbackFunction) callbackFunction();
    }
    
    const std::string& getName() const { return panelName; }
};

// Mock UIFactory implementation
std::unique_ptr<IComponent> UIFactory::createKeyComponent(IStyleService* styleService) {
    return std::make_unique<MockComponent>();
}

std::unique_ptr<IComponent> UIFactory::createLockComponent(IStyleService* styleService) {
    return std::make_unique<MockComponent>();
}

std::unique_ptr<IComponent> UIFactory::createClarityComponent(IStyleService* styleService) {
    return std::make_unique<MockComponent>();
}

std::unique_ptr<IComponent> UIFactory::createOemOilPressureComponent(IStyleService* styleService) {
    return std::make_unique<MockComponent>();
}

std::unique_ptr<IComponent> UIFactory::createOemOilTemperatureComponent(IStyleService* styleService) {
    return std::make_unique<MockComponent>();
}

std::unique_ptr<IPanel> UIFactory::createKeyPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService) {
    return std::make_unique<MockPanel>("KEY");
}

std::unique_ptr<IPanel> UIFactory::createLockPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService) {
    return std::make_unique<MockPanel>("LOCK");
}

std::unique_ptr<IPanel> UIFactory::createSplashPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService) {
    return std::make_unique<MockPanel>("SPLASH");
}

std::unique_ptr<IPanel> UIFactory::createOemOilPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService) {
    return std::make_unique<MockPanel>("OIL");
}

// Mock ESP32 functions implementation
extern "C" {
    esp_err_t nvs_flash_erase() { 
        return ESP_OK; 
    }
    
    esp_err_t nvs_flash_init() {
        return ESP_OK;
    }
}

// Mock LVGL font definitions
#include "lvgl.h"
const lv_font_t lv_font_montserrat_20 = {0};
const lv_font_t lv_font_montserrat_16 = {0};
const lv_font_t lv_font_montserrat_14 = {0};

// Global mock service instances to prevent redefinition conflicts
#include "mock_globals.h"
#include "mock_services.h"
#include "mock_gpio_provider.h"

MockDisplayProvider* g_mockDisplay = nullptr;
MockGpioProvider* g_mockGpio = nullptr; 
MockStyleService* g_mockStyle = nullptr;

void initGlobalMocks() {
    if (!g_mockDisplay) g_mockDisplay = new MockDisplayProvider();
    if (!g_mockGpio) g_mockGpio = new MockGpioProvider();
    if (!g_mockStyle) g_mockStyle = new MockStyleService();
    
    if (g_mockDisplay) g_mockDisplay->initialize();
    if (g_mockStyle) g_mockStyle->initializeStyles();
}

void cleanupGlobalMocks() {
    delete g_mockDisplay;
    delete g_mockGpio;
    delete g_mockStyle;
    g_mockDisplay = nullptr;
    g_mockGpio = nullptr;
    g_mockStyle = nullptr;
}

#endif // UNIT_TESTING