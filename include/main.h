#pragma once

/**
 * @file main.h
 * @brief Main application entry point and core system includes
 *
 * @details This header defines the main application version and includes all core
 * system dependencies. It serves as the central coordination point for the
 * Clarity ESP32 digital gauge system.
 * @architecture MVP (Model-View-Presenter) with layered design
 * @target_hardware ESP32-WROOM-32 with 1.28" round GC9A01 display
 *
 * @system_flow:
 * 1. Device initialization and display setup
 * 2. Preference loading and theme configuration
 * 3. Panel manager initialization and default panel loading
 * 4. Main event loop with ticker-based updates
 *
 * @context This is the main entry point for the application. The system
 * follows a strict initialization order and uses dependency injection for
 * coordinating system services.
 */

// ========== Macros/Defines ==========
#define UI_VERSION "4.1.0"

// ========== System/Library Includes ==========
#include <LovyanGFX.hpp>
#include <lvgl.h>

// ========== Project Includes ==========
#include "utilities/ticker.h"
#include "definitions/configs.h"
#include "definitions/constants.h"
#include <memory>

// ========== Forward Declarations ==========
class IProviderFactory;
class ManagerFactory;
class DeviceProvider;
class IGpioProvider;
class IDisplayProvider;
class StyleManager;
class IPreferenceService;
class PanelManager;
class InterruptManager;
class ErrorManager;

// ========== Global Variables ==========
// Global factories - dual factory pattern implementation
extern std::unique_ptr<IProviderFactory> providerFactory;
extern std::unique_ptr<ManagerFactory> managerFactory;

// Global providers - created by ProviderFactory
extern std::unique_ptr<DeviceProvider> deviceProvider;
extern std::unique_ptr<IGpioProvider> gpioProvider;
extern std::unique_ptr<IDisplayProvider> displayProvider;

// Global managers - created by ManagerFactory
extern std::unique_ptr<StyleManager> styleManager;
extern std::unique_ptr<IPreferenceService> preferenceManager;
extern std::unique_ptr<PanelManager> panelManager;
extern InterruptManager *interruptManager;
extern ErrorManager *errorManager;

// ========== System Configuration Items ==========
// Inline definitions (C++17) - global scope configuration
inline Config::ConfigItem defaultPanelConfig(ConfigConstants::Items::DEFAULT_PANEL, UIStrings::ConfigLabels::DEFAULT_PANEL,
                                             std::string(ConfigConstants::Panels::OEM_OIL_PANEL),
                                             Config::ConfigMetadata("OemOilPanel,ConfigPanel,DiagnosticPanel", Config::ConfigItemType::Selection));

inline Config::ConfigItem updateRateConfig(ConfigConstants::Items::UPDATE_RATE, UIStrings::ConfigLabels::UPDATE_RATE,
                                           500, Config::ConfigMetadata("100,250,500,750,1000,1500,2000", ConfigConstants::Units::MILLISECONDS, Config::ConfigItemType::Selection));

inline Config::ConfigItem showSplashConfig(ConfigConstants::Items::SHOW_SPLASH, UIStrings::ConfigLabels::SHOW_SPLASH,
                                           true, Config::ConfigMetadata());

// ========== Function Declarations ==========
void registerSystemConfiguration();