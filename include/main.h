// include/main.h
#pragma once

#define UI_VERSION "4.1.0"

#include "utilities/ticker.h"
#include "handlers/panel_manager.h"
#include "handlers/preference_manager.h"
#include "device.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <memory>

Device _device;
PreferenceManager _preference_manager;
PanelManager _panel_manager;