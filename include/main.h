#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#define UI_VERSION "4.1.0"

#include "utilities/serial_logger.h"
#include "utilities/ticker.h"
#include "panels/demo_panel.h"
#include "panels/splash_panel.h"
#include "panel_manager.h"
#include "device.h"

#include <Arduino.h>
#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <Preferences.h>
#include <memory>

// Global device and preferences
Device _device;
Preferences _preferences;

// Panel manager
std::shared_ptr<PanelManager> _panel_manager;

// Panel instances
std::shared_ptr<SplashPanel> _splash_panel;
std::shared_ptr<DemoPanel> _demo_panel;

static bool _is_setup_complete = false;