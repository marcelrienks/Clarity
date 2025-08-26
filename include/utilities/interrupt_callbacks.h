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
 * @memory_optimization Single processing function per interrupt (4 bytes saved per interrupt)
 * @callback_pattern Each interrupt has single process function (evaluate + signal execution)
 *
 * @context_parameter All functions receive void* context pointing to sensor instance
 * The context parameter allows static functions to access sensor state and return
 * whether execution should proceed via InterruptResult.
 */
struct InterruptCallbacks {
    
    // Memory-optimized single-function callbacks (evaluate + signal execution)
    static InterruptResult KeyPresentProcess(void* context);
    static InterruptResult KeyNotPresentProcess(void* context);
    static InterruptResult LockStateProcess(void* context);
    static InterruptResult LightsStateProcess(void* context);
    static InterruptResult ShortPressProcess(void* context);
    static InterruptResult LongPressProcess(void* context);
    static InterruptResult ErrorOccurredProcess(void* context);
    
private:
    // Helper functions for panel management
    static void* GetPanelManager();
    static void* GetStyleManager();
    static void* GetCurrentPanel();
    
    // No instantiation - all static methods
    InterruptCallbacks() = delete;
    ~InterruptCallbacks() = delete;
};