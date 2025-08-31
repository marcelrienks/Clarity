#pragma once

#include "utilities/types.h"

/**
 * @file interrupt_callbacks.h
 * @brief Static callback functions for interrupt system
 *
 * @details Static callback functions with separated evaluation and execution.
 * Static function pointers are used to prevent heap fragmentation and ensure
 * memory safety on ESP32.
 */
struct InterruptCallbacks {
    
    // Evaluation functions - check if interrupt condition is met
    static bool KeyPresentEvaluate(void* context);
    static bool KeyNotPresentEvaluate(void* context);
    static bool LockStateEvaluate(void* context);
    static bool LightsStateEvaluate(void* context);
    static bool ShortPressEvaluate(void* context);
    static bool LongPressEvaluate(void* context);
    static bool ErrorOccurredEvaluate(void* context);
    
    // Execution functions - perform the interrupt action
    static void KeyPresentExecute(void* context);
    static void KeyNotPresentExecute(void* context);
    static void LockStateExecute(void* context);
    static void LightsStateExecute(void* context);
    static void ShortPressExecute(void* context);
    static void LongPressExecute(void* context);
    static void ErrorOccurredExecute(void* context);
    
private:
    // Helper functions for panel management
    static void* GetPanelManager();
    static void* GetStyleManager();
    static void* GetCurrentPanel();
    
    // No instantiation - all static methods
    InterruptCallbacks() = delete;
    ~InterruptCallbacks() = delete;
};