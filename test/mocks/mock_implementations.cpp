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
public:
    void init(IGpioProvider *gpio, IDisplayProvider *display) override {}
    void load(std::function<void()> callbackFunction, IGpioProvider *gpio, IDisplayProvider *display) override {}
    void update(std::function<void()> callbackFunction, IGpioProvider *gpio, IDisplayProvider *display) override {}
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
    return std::make_unique<MockPanel>();
}

std::unique_ptr<IPanel> UIFactory::createLockPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService) {
    return std::make_unique<MockPanel>();
}

std::unique_ptr<IPanel> UIFactory::createSplashPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService) {
    return std::make_unique<MockPanel>();
}

std::unique_ptr<IPanel> UIFactory::createOemOilPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService) {
    return std::make_unique<MockPanel>();
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

#endif // UNIT_TESTING