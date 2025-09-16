#include "factories/provider_factory.h"
#include "providers/gpio_provider.h"
#include "providers/lvgl_display_provider.h"
#include "providers/device_provider.h"
#include "managers/error_manager.h"
#include <Arduino.h>

#include "esp32-hal-log.h"

// ========== Public Interface Methods ==========

/**
 * @brief Creates a GPIO provider for hardware pin control
 * @return Unique pointer to IGpioProvider interface or nullptr on failure
 *
 * Instantiates GpioProvider with exception handling for allocation failures.
 * Provides hardware abstraction for GPIO operations including digital/analog
 * reads and interrupt management.
 */
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

/**
 * @brief Creates a display provider for LVGL UI operations
 * @param deviceProvider Device provider with initialized screen
 * @return Unique pointer to IDisplayProvider interface or nullptr on failure
 *
 * Creates LvglDisplayProvider using screen from DeviceProvider. Validates
 * device provider and screen availability. Reports critical errors on failure.
 */
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

/**
 * @brief Creates a device provider for display hardware initialization
 * @return Unique pointer to DeviceProvider or nullptr on failure
 *
 * Instantiates DeviceProvider and initializes display hardware including
 * SPI configuration, GC9A01 panel setup, and LVGL initialization.
 * Validates screen initialization before returning.
 */
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