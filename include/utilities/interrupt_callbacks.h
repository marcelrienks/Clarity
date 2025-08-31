#pragma once

#include "utilities/types.h"

/**
 * @file interrupt_callbacks.h
 * @brief Static callback functions for simplified interrupt system
 *
 * @details Static callback functions for single-purpose interrupts.
 * Each function performs one specific action when the interrupt is triggered.
 * Static function pointers are used to prevent heap fragmentation and ensure
 * memory safety on ESP32.
 */
struct InterruptCallbacks {
    
    // Panel loading interrupts
    static void KeyPresentActivate(void* context);
    static void KeyNotPresentActivate(void* context);
    
    // Lock state interrupts - activation and deactivation
    static void LockEngagedActivate(void* context);
    static void LockDisengagedActivate(void* context);
    
    // Lights state interrupts - activation and deactivation  
    static void LightsOnActivate(void* context);
    static void LightsOffActivate(void* context);
    
    // Error state interrupts - activation and deactivation
    static void ErrorOccurredActivate(void* context);
    static void ErrorClearedActivate(void* context);
    
    // Button action interrupts
    static void ShortPressActivate(void* context);
    static void LongPressActivate(void* context);
    
private:
    // Helper functions for panel management
    static void* GetPanelManager();
    static void* GetStyleManager();
    static void* GetCurrentPanel();
    
    // No instantiation - all static methods
    InterruptCallbacks() = delete;
    ~InterruptCallbacks() = delete;
};