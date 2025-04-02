// include/main.h
#pragma once

#define UI_VERSION "4.1.0"

#include "utilities/serial_logger.h"
#include "utilities/ticker.h"
#include "handlers/panel_manager.h"
#include "handlers/preference_manager.h"
#include "device.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <memory>

Device _device;
PreferenceManager _preferences;
PanelManager _panel_manager;