#include "factories/provider_factory.h"
#include "providers/gpio_provider.h"
#include "providers/lvgl_display_provider.h"
#include "providers/device_provider.h"
#include "managers/error_manager.h"
#include <Arduino.h>

#include "esp32-hal-log.h"

std::unique_ptr<IGpioProvider> ProviderFactory::CreateGpioProvider()
{
    try
    {
        auto provider = std::make_unique<GpioProvider>();
        if (provider)
        {
            return provider;
        }
        else
        {
            log_e("Failed to create GpioProvider - allocation failed");
            return nullptr;
        }
    }
    catch (const std::exception& e)
    {
        log_e("Exception creating GpioProvider: %s", e.what());
        return nullptr;
    }
}

std::unique_ptr<IDisplayProvider> ProviderFactory::CreateDisplayProvider(DeviceProvider* deviceProvider)
{
    if (!deviceProvider)
    {
        log_e("DeviceProvider parameter is null");
        ErrorManager::Instance().ReportCriticalError("ProviderFactory", "DeviceProvider parameter is null");
        return nullptr;
    }
    
    // Access the screen from the injected deviceProvider using interface method
    lv_obj_t* screen = deviceProvider->GetScreen();
    if (!screen)
    {
        log_e("DeviceProvider screen is null");
        ErrorManager::Instance().ReportCriticalError("ProviderFactory", "DeviceProvider screen is null");
        return nullptr;
    }

    try
    {
        auto provider = std::make_unique<LvglDisplayProvider>(screen);
        if (provider)
        {
            return provider;
        }
        else
        {
            log_e("Failed to create LvglDisplayProvider - allocation failed");
            return nullptr;
        }
    }
    catch (const std::exception& e)
    {
        log_e("Exception creating LvglDisplayProvider: %s", e.what());
        return nullptr;
    }
}

std::unique_ptr<DeviceProvider> ProviderFactory::CreateDeviceProvider()
{
    try
    {
        auto provider = std::make_unique<DeviceProvider>();
        if (provider)
        {
            // Prepare the device (initializes screen)
            provider->prepare();
            if (!provider->GetScreen())
            {
                log_e("DeviceProvider screen initialization failed");
                ErrorManager::Instance().ReportCriticalError("ProviderFactory", "DeviceProvider screen is null");
                return nullptr;
            }
            
            return provider;
        }
        else
        {
            log_e("Failed to create DeviceProvider - allocation failed");
            return nullptr;
        }
    }
    catch (const std::exception& e)
    {
        log_e("Exception creating DeviceProvider: %s", e.what());
        return nullptr;
    }
}