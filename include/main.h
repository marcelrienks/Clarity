#ifndef MAIN_H
#define MAIN_H

#define UI_VERSION "4.1.0"

#include "utilities/serial_logger.h"
#include "utilities/ticker.h"
#include "panels/demo_panel.h"
#include "panels/splash_panel.h"
#include "device.h"

#include <Arduino.h>
#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <Preferences.h>

Device _device;
Preferences _preferences;

// Components
SplashPanel *_splash_panel;
DemoPanel *_demo_panel;

#endif