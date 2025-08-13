#pragma once

#include <Arduino.h>
#include <memory>

// Forward declarations
class DeviceProvider;
class GpioProvider;
class LvglDisplayProvider;

/**
 * @class ProviderFactory
 * @brief Factory for creating provider instances with error handling and logging
 *
 * @details This factory provides static methods for creating all provider types
 * used in the Clarity system. Each factory method includes proper error handling,
 * null checking, and debug logging for initialization tracking.
 *
 * @design_pattern Factory Pattern
 * @error_handling All methods return nullptr on failure with error logging
 * @logging Debug level logging for successful creations, error level for failures
 */
class ProviderFactory
{
  public:
    /**
     * @brief Create DeviceProvider instance
     * @return std::unique_ptr<DeviceProvider> or nullptr on failure
     */
    static std::unique_ptr<DeviceProvider> createDeviceProvider();

    /**
     * @brief Create GpioProvider instance
     * @return std::unique_ptr<GpioProvider> or nullptr on failure
     */
    static std::unique_ptr<GpioProvider> createGpioProvider();

    /**
     * @brief Create LvglDisplayProvider instance
     * @param deviceProvider DeviceProvider instance for display integration
     * @return std::unique_ptr<LvglDisplayProvider> or nullptr on failure
     */
    static std::unique_ptr<LvglDisplayProvider> createLvglDisplayProvider(DeviceProvider *deviceProvider);

  private:
    ProviderFactory() = delete; // Static factory only
    ~ProviderFactory() = delete;
};