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

// Macros/Defines
#define UI_VERSION "4.1.0"

// System/Library Includes
#include <LovyanGFX.hpp>
#include <lvgl.h>

// Project Includes
#include "utilities/ticker.h"

// Function declarations