#pragma once

#include "utilities/types.h"

/**
 * @file interrupt_callbacks.h
 * @brief Static callback functions for interrupt system
 *
 * @details This file contains all static callback functions used by the
 * coordinated interrupt system. Static function pointers are used instead
 * of std::function objects to prevent heap fragmentation and ensure memory
 * safety on ESP32.
 *
 * @architecture_requirement Static function pointers for ESP32 memory safety
 * @memory_optimization Single execution function per interrupt (28 bytes saved)
 * @callback_pattern Each sensor type has evaluation and execution functions
 *
 * @context_parameter All functions receive void* context pointing to sensor instance
 * The context parameter allows static functions to access sensor state for
 * evaluation and execute appropriate actions.
 */
struct InterruptCallbacks {
    
    // Key Present Sensor Callbacks
    static bool KeyPresentChanged(void* context);
    static void LoadKeyPanel(void* context);
    
    // Key Not Present Sensor Callbacks  
    static bool KeyNotPresentChanged(void* context);
    static void RestoreFromKeyPanel(void* context);
    
    // Lock Sensor Callbacks
    static bool LockStateChanged(void* context);
    static void LoadLockPanel(void* context);
    
    // Lights Sensor Callbacks (Theme switching)
    static bool LightsStateChanged(void* context);
    static void SetThemeBasedOnLights(void* context);
    
    // Button Input Callbacks (Universal button system)
    static bool HasShortPressEvent(void* context);
    static void ExecutePanelShortPress(void* context);
    
    static bool HasLongPressEvent(void* context);
    static void ExecutePanelLongPress(void* context);
    
    // Error System Callbacks
    static bool ErrorOccurred(void* context);
    static void LoadErrorPanel(void* context);
    
private:
    // Helper functions for panel management
    static void* GetPanelManager();
    static void* GetStyleManager();
    static void* GetCurrentPanel();
    
    // No instantiation - all static methods
    InterruptCallbacks() = delete;
    ~InterruptCallbacks() = delete;
};