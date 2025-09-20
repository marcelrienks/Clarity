#pragma once

#include "utilities/constants.h"

/**
 * @file system_config.h
 * @brief System-wide configuration constants for main.cpp
 * @deprecated Use constants from include/constants.h instead
 */

namespace SystemConfig {
    // Configuration section and keys - use centralized constants
    static constexpr const char* CONFIG_SECTION = ConfigConstants::Sections::SYSTEM;
    static constexpr const char* CONFIG_DEFAULT_PANEL = ConfigConstants::Keys::SYSTEM_DEFAULT_PANEL;
    static constexpr const char* CONFIG_UPDATE_RATE = ConfigConstants::Keys::SYSTEM_UPDATE_RATE;
    static constexpr const char* CONFIG_SHOW_SPLASH = ConfigConstants::Keys::SYSTEM_SHOW_SPLASH;
}