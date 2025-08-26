#include "factories/provider_factory.h"
#include "providers/gpio_provider.h"
#include "providers/lvgl_display_provider.h"
#include "providers/device_provider.h"
#include "interfaces/i_device_provider.h"
#include "managers/error_manager.h"
#include <Arduino.h>

#include "esp32-hal-log.h"

// Store device provider for display creation
static std::unique_ptr<DeviceProvider> s_deviceProvider;

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

std::unique_ptr<IDisplayProvider> ProviderFactory::CreateDisplayProvider()
{
    log_v("CreateDisplayProvider() called");
    
    // First ensure device provider exists
    if (!s_deviceProvider)
    {
        log_d("Creating DeviceProvider for display initialization");
        s_deviceProvider = std::make_unique<DeviceProvider>();
        if (!s_deviceProvider)
        {
            log_e("Failed to create DeviceProvider for display");
            ErrorManager::Instance().ReportCriticalError("ProviderFactory", "DeviceProvider allocation failed");
            return nullptr;
        }
        
        // Prepare the device (initializes screen)
        s_deviceProvider->prepare();
        if (!s_deviceProvider->screen)
        {
            log_e("DeviceProvider screen initialization failed");
            ErrorManager::Instance().ReportCriticalError("ProviderFactory", "DeviceProvider screen is null after prepare");
            return nullptr;
        }
    }
    
    try
    {
        auto provider = std::make_unique<LvglDisplayProvider>(s_deviceProvider->screen);
        if (provider)
        {
            log_d("Successfully created LvglDisplayProvider");
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
    
    // If device provider already exists (created for display), return it
    if (s_deviceProvider)
    {
        log_d("Returning existing DeviceProvider instance");
        std::unique_ptr<IDeviceProvider> provider = std::move(s_deviceProvider);
        return provider;
    }
    
    try
    {
        auto provider = std::make_unique<DeviceProvider>();
        if (provider)
        {
            // Prepare the device
            provider->prepare();
            if (!provider->screen)
            {
                log_e("DeviceProvider screen initialization failed");
                ErrorManager::Instance().ReportCriticalError("ProviderFactory", "DeviceProvider screen is null");
                return nullptr;
            }
            
            log_d("Successfully created and prepared DeviceProvider");
            std::unique_ptr<IDeviceProvider> deviceProvider = std::move(provider);
            return deviceProvider;
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