#include "factories/provider_factory.h"
#include "providers/gpio_provider.h"
#include "providers/lvgl_display_provider.h"
#include "providers/device_provider.h"
#include "interfaces/i_device_provider.h"
#include "managers/error_manager.h"
#include <Arduino.h>

#include "esp32-hal-log.h"

std::unique_ptr<IGpioProvider> ProviderFactory::CreateGpioProvider()
{
    log_v("CreateGpioProvider() called");
    
    try
    {
        auto provider = std::make_unique<GpioProvider>();
        if (provider)
        {
            log_d("Successfully created GpioProvider");
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

std::unique_ptr<IDisplayProvider> ProviderFactory::CreateDisplayProvider(IDeviceProvider* deviceProvider)
{
    log_v("CreateDisplayProvider() called");
    
    if (!deviceProvider)
    {
        log_e("DeviceProvider parameter is null");
        ErrorManager::Instance().ReportCriticalError("ProviderFactory", "DeviceProvider parameter is null");
        return nullptr;
    }
    
    // Access the screen from the injected deviceProvider
    // Note: Using static_cast since we control the concrete type in this codebase
    auto* concreteDeviceProvider = static_cast<DeviceProvider*>(deviceProvider);
    if (!concreteDeviceProvider->screen)
    {
        log_e("DeviceProvider screen is null");
        ErrorManager::Instance().ReportCriticalError("ProviderFactory", "DeviceProvider screen is null");
        return nullptr;
    }
    
    try
    {
        auto provider = std::make_unique<LvglDisplayProvider>(concreteDeviceProvider->screen);
        if (provider)
        {
            log_d("Successfully created LvglDisplayProvider with injected DeviceProvider");
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

std::unique_ptr<IDeviceProvider> ProviderFactory::CreateDeviceProvider()
{
    log_v("CreateDeviceProvider() called");
    
    try
    {
        auto provider = std::make_unique<DeviceProvider>();
        if (provider)
        {
            // Prepare the device (initializes screen)
            provider->prepare();
            if (!provider->screen)
            {
                log_e("DeviceProvider screen initialization failed");
                ErrorManager::Instance().ReportCriticalError("ProviderFactory", "DeviceProvider screen is null");
                return nullptr;
            }
            
            log_d("Successfully created and prepared DeviceProvider");
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