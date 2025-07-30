#pragma once

#include <stdint.h>
#include <stdbool.h>

// Mock panel types
#define PANEL_OIL "OemOilPanel"
#define PANEL_KEY "KeyPanel" 
#define PANEL_LOCK "LockPanel"
#define PANEL_SPLASH "SplashPanel"

// Mock trigger types (avoiding conflicts with mock_types.h)
#ifndef TRIGGER_KEY_PRESENT
#define TRIGGER_KEY_PRESENT "KEY_PRESENT"
#endif
#ifndef TRIGGER_KEY_NOT_PRESENT
#define TRIGGER_KEY_NOT_PRESENT "KEY_NOT_PRESENT"
#endif
#ifndef TRIGGER_LOCK
#define TRIGGER_LOCK "LOCK_STATE"
#endif
#ifndef TRIGGER_THEME
#define TRIGGER_THEME "THEME_STATE"
#endif

// Mock function declarations
void InitializeTriggersFromGpio(void);
const char* GetCurrentPanel(void);
void SetTrigger(const char* trigger, bool active);
bool IsTriggerActive(const char* trigger);
void ResetAllTriggers(void);
void SimulateSystemTick(uint32_t ms);
void SetTheme(const char* theme);
const char* GetCurrentTheme(void);

// Additional mock functions for scenario tests
bool IsNightThemeActive(void);
bool IsKeyPresent(void);
bool IsKeyNotPresent(void);
bool IsLockActive(void);

// Oil sensor mock functions
void InitializeOilPressureSensor(void);
void InitializeOilTemperatureSensor(void);

// Mock constants
#define DEFAULT_OIL_PRESSURE 50
#define DEFAULT_OIL_TEMPERATURE 80

// Mock state variables
extern bool mock_key_present_active;
extern bool mock_key_not_present_active;
extern bool mock_lock_active;
extern bool mock_theme_active;
extern const char* mock_current_panel;
extern const char* mock_current_theme;