#pragma once

/**
 * @file system_config.h
 * @brief System-wide configuration constants for main.cpp
 */

namespace SystemConfig {
    // Configuration section and keys
    static constexpr const char* CONFIG_SECTION = "system";
    static constexpr const char* CONFIG_DEFAULT_PANEL = "system.default_panel";
    static constexpr const char* CONFIG_UPDATE_RATE = "system.update_rate";
    static constexpr const char* CONFIG_SHOW_SPLASH = "system.show_splash";
}