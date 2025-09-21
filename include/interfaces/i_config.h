#pragma once

#include "interfaces/i_preference_service.h"

/**
 * @interface IConfig
 * @brief Interface for components that register configuration items
 *
 * @details Components that have configurable settings should inherit from this
 * interface and implement the RegisterConfig method to register their
 * configuration items with the preference service.
 */
class IConfig {
public:
    virtual ~IConfig() = default;

    /**
     * @brief Register configuration items with the preference service
     * @param preferenceService The preference service to register configs with
     */
    virtual void RegisterConfig(IPreferenceService* preferenceService) = 0;
};