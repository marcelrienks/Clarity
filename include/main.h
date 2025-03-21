// include/main.h
#pragma once

#define UI_VERSION "4.1.0"

#include "utilities/serial_logger.h"
#include "utilities/ticker.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "device.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <memory>

// Global device and preferences
Device _device;
PreferenceManager _preferences("clarity");

// Panel manager
std::shared_ptr<PanelManager> _panel_manager;