#include "factories/provider_factory.h"
#include "providers/device_provider.h"
#include "providers/gpio_provider.h"
#include "providers/lvgl_display_provider.h"

std::unique_ptr<DeviceProvider> ProviderFactory::createDeviceProvider()
{
    log_d("ProviderFactory: Creating DeviceProvider (ESP32 display hardware)...");
    
    auto provider = std::make_unique<DeviceProvider>();
    if (!provider) {
        log_e("ProviderFactory: Failed to create DeviceProvider - allocation failed");
        return nullptr;
    }
    
    log_d("ProviderFactory: Preparing DeviceProvider hardware...");
    provider->prepare();
    
    if (!provider->screen) {
        log_e("ProviderFactory: DeviceProvider screen initialization failed");
        return nullptr;
    }
    
    log_d("ProviderFactory: DeviceProvider created successfully (screen initialized)");
    return provider;
}

std::unique_ptr<GpioProvider> ProviderFactory::createGpioProvider()
{
    log_d("ProviderFactory: Creating GpioProvider (hardware GPIO interface)...");
    
    auto provider = std::make_unique<GpioProvider>();
    if (!provider) {
        log_e("ProviderFactory: Failed to create GpioProvider - allocation failed");
        return nullptr;
    }
    
    log_d("ProviderFactory: GpioProvider created successfully");
    return provider;
}

std::unique_ptr<LvglDisplayProvider> ProviderFactory::createLvglDisplayProvider(DeviceProvider* deviceProvider)
{
    log_d("ProviderFactory: Creating LvglDisplayProvider (LVGL display abstraction)...");
    
    if (!deviceProvider) {
        log_e("ProviderFactory: Cannot create LvglDisplayProvider - DeviceProvider is null");
        return nullptr;
    }
    
    if (!deviceProvider->screen) {
        log_e("ProviderFactory: Cannot create LvglDisplayProvider - DeviceProvider screen is null");
        return nullptr;
    }
    
    log_d("ProviderFactory: Initializing LvglDisplayProvider with screen object");
    auto provider = std::make_unique<LvglDisplayProvider>(deviceProvider->screen);
    if (!provider) {
        log_e("ProviderFactory: Failed to create LvglDisplayProvider - allocation failed");
        return nullptr;
    }
    
    log_d("ProviderFactory: LvglDisplayProvider created successfully");
    return provider;
}