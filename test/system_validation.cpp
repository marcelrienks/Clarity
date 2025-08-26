/**
 * @file system_validation.cpp
 * @brief Comprehensive system validation for Clarity automotive gauge system
 * 
 * This file provides hardware-in-the-loop testing capabilities for validating
 * the complete system implementation including:
 * - Panel switching and restoration
 * - Error handling workflow
 * - Universal button system
 * - Memory and performance characteristics
 * - Integration between all major components
 * 
 * Can be compiled as the main program for system validation testing.
 */

#include <Arduino.h>
#include <esp32-hal-log.h>
#include "main_system.h" // Include main system initialization
#include "managers/panel_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/error_manager.h"
#include "utilities/constants.h"

// External references to global managers (from main.cpp)
extern std::unique_ptr<PanelManager> panelManager;
extern std::unique_ptr<InterruptManager> interruptManager;

// Test state tracking
struct ValidationState {
    unsigned long testStartTime = 0;
    int currentTestPhase = 0;
    bool testCompleted = false;
    unsigned long lastPhaseTime = 0;
    
    // Test results
    bool panelSwitchingPassed = false;
    bool errorHandlingPassed = false;
    bool buttonSystemPassed = false;
    bool memoryTestPassed = false;
    bool performanceTestPassed = false;
} validationState;

/**
 * @brief Phase 1: Basic Panel Switching Validation
 * Test fundamental panel loading and switching functionality
 */
void validatePanelSwitching() {
    log_i("=== PHASE 1: Panel Switching Validation ===");
    
    if (!panelManager) {
        log_e("PanelManager not initialized - FAILED");
        return;
    }
    
    // Test 1: Load default Oil Panel
    bool result = panelManager->CreateAndLoadPanel(PanelNames::OIL, false);
    if (!result) {
        log_e("Failed to load Oil Panel - FAILED");
        return;
    }
    log_i("✓ Oil Panel loaded successfully");
    
    // Test 2: Load Config Panel  
    result = panelManager->CreateAndLoadPanel(PanelNames::CONFIG, false);
    if (!result) {
        log_e("Failed to load Config Panel - FAILED");
        return;
    }
    log_i("✓ Config Panel loaded successfully");
    
    // Test 3: Verify restoration panel tracking
    const char* restorationPanel = panelManager->GetRestorationPanel();
    if (strcmp(restorationPanel, PanelNames::OIL) != 0) {
        log_e("Restoration panel incorrect: %s (expected: %s) - FAILED", 
              restorationPanel, PanelNames::OIL);
        return;
    }
    log_i("✓ Restoration panel tracking working correctly");
    
    // Test 4: Load Splash Panel
    result = panelManager->CreateAndLoadPanel(PanelNames::SPLASH, false);
    if (!result) {
        log_e("Failed to load Splash Panel - FAILED");
        return;
    }
    log_i("✓ Splash Panel loaded successfully");
    
    validationState.panelSwitchingPassed = true;
    log_i("✓ Panel Switching Validation: PASSED");
}

/**
 * @brief Phase 2: Error Handling Workflow Validation
 * Test complete error detection, display, and restoration cycle
 */
void validateErrorHandling() {
    log_i("=== PHASE 2: Error Handling Validation ===");
    
    ErrorManager& errorManager = ErrorManager::Instance();
    
    // Test 1: Clear any existing errors
    errorManager.ClearAllErrors();
    if (errorManager.HasPendingErrors()) {
        log_e("Failed to clear existing errors - FAILED");
        return;
    }
    log_i("✓ Error queue cleared successfully");
    
    // Test 2: Generate test errors
    errorManager.ReportError(ErrorLevel::WARNING, "SystemTest", "Test warning message");
    errorManager.ReportError(ErrorLevel::ERROR, "SystemTest", "Test error message");
    errorManager.ReportError(ErrorLevel::CRITICAL, "SystemTest", "Test critical message");
    
    if (!errorManager.HasPendingErrors()) {
        log_e("Errors not reported correctly - FAILED");
        return;
    }
    log_i("✓ Test errors reported successfully");
    
    // Test 3: Verify error panel trigger
    if (!errorManager.ShouldTriggerErrorPanel()) {
        log_e("Error panel trigger not activated - FAILED");
        return;
    }
    log_i("✓ Error panel trigger activated");
    
    // Test 4: Load error panel (simulating automatic trigger)
    if (panelManager) {
        errorManager.SetErrorPanelActive(true);
        bool result = panelManager->CreateAndLoadPanel(PanelNames::ERROR, true);
        if (!result) {
            log_e("Failed to load Error Panel - FAILED");
            return;
        }
        log_i("✓ Error Panel loaded successfully");
    }
    
    // Test 5: Clear errors and verify restoration
    errorManager.ClearAllErrors();
    errorManager.SetErrorPanelActive(false);
    
    if (errorManager.HasPendingErrors() || errorManager.ShouldTriggerErrorPanel()) {
        log_e("Errors not cleared properly - FAILED");
        return;
    }
    log_i("✓ Errors cleared and trigger deactivated");
    
    validationState.errorHandlingPassed = true;
    log_i("✓ Error Handling Validation: PASSED");
}

/**
 * @brief Phase 3: Universal Button System Validation  
 * Test button function injection and execution
 */
void validateButtonSystem() {
    log_i("=== PHASE 3: Button System Validation ===");
    
    if (!panelManager || !interruptManager) {
        log_e("Required managers not initialized - FAILED");
        return;
    }
    
    // Test 1: Load Config Panel for button testing
    bool result = panelManager->CreateAndLoadPanel(PanelNames::CONFIG, false);
    if (!result) {
        log_e("Failed to load Config Panel for button test - FAILED");
        return;
    }
    log_i("✓ Config Panel loaded for button testing");
    
    // Test 2: Verify current panel implements IActionService
    auto* currentPanel = panelManager->GetCurrentPanel();
    if (!currentPanel) {
        log_e("Current panel is null - FAILED");
        return;
    }
    
    auto* actionService = dynamic_cast<IActionService*>(currentPanel);
    if (!actionService) {
        log_e("Current panel doesn't implement IActionService - FAILED");
        return;
    }
    log_i("✓ Current panel implements IActionService");
    
    // Test 3: Extract button functions
    auto shortPressFunc = actionService->GetShortPressFunction();
    auto longPressFunc = actionService->GetLongPressFunction();
    auto panelContext = actionService->GetPanelContext();
    
    if (!shortPressFunc || !longPressFunc || !panelContext) {
        log_e("Button functions not properly extracted - FAILED");
        return;
    }
    log_i("✓ Button functions extracted successfully");
    
    // Test 4: Inject functions into interrupt manager
    interruptManager->UpdateButtonInterrupts(shortPressFunc, longPressFunc, panelContext);
    log_i("✓ Button functions injected into interrupt manager");
    
    // Test 5: Test different panel types
    const char* testPanels[] = {PanelNames::OIL, PanelNames::ERROR, PanelNames::SPLASH};
    for (const char* panelName : testPanels) {
        result = panelManager->CreateAndLoadPanel(panelName, false);
        if (result) {
            auto* testPanel = panelManager->GetCurrentPanel();
            auto* testActionService = dynamic_cast<IActionService*>(testPanel);
            if (testActionService) {
                log_i("✓ Panel %s implements IActionService correctly", panelName);
            }
        }
    }
    
    validationState.buttonSystemPassed = true;
    log_i("✓ Button System Validation: PASSED");
}

/**
 * @brief Phase 4: Memory Usage Validation
 * Test memory efficiency and stability
 */
void validateMemoryUsage() {
    log_i("=== PHASE 4: Memory Usage Validation ===");
    
    // Test 1: Check ESP32 memory constraints
    size_t totalHeap = ESP.getHeapSize();
    size_t freeHeap = ESP.getFreeHeap();
    size_t usedHeap = totalHeap - freeHeap;
    
    log_i("Memory Status:");
    log_i("  Total Heap: %d bytes", totalHeap);
    log_i("  Used Heap:  %d bytes", usedHeap);
    log_i("  Free Heap:  %d bytes", freeHeap);
    
    // ESP32-WROOM-32 should have reasonable memory usage
    if (usedHeap > 200000) { // 200KB limit
        log_w("Memory usage high: %d bytes (limit: 200KB)", usedHeap);
    } else {
        log_i("✓ Memory usage within acceptable limits");
    }
    
    if (freeHeap < 50000) { // 50KB minimum
        log_e("Insufficient free memory: %d bytes (minimum: 50KB) - FAILED", freeHeap);
        return;
    }
    log_i("✓ Sufficient free memory available");
    
    // Test 2: Memory stability under load
    size_t initialFreeHeap = freeHeap;
    
    // Perform memory-intensive operations
    for (int i = 0; i < 20; i++) {
        if (panelManager) {
            panelManager->CreateAndLoadPanel(PanelNames::OIL, false);
            panelManager->CreateAndLoadPanel(PanelNames::CONFIG, false);
        }
        
        ErrorManager::Instance().ReportError(ErrorLevel::WARNING, "MemTest", "Memory test warning");
        ErrorManager::Instance().ClearAllErrors();
        
        if (i % 5 == 0) {
            delay(10); // Allow some processing time
        }
    }
    
    size_t finalFreeHeap = ESP.getFreeHeap();
    long memoryDifference = static_cast<long>(finalFreeHeap) - static_cast<long>(initialFreeHeap);
    
    log_i("Memory stability test:");
    log_i("  Initial free: %d bytes", initialFreeHeap);
    log_i("  Final free:   %d bytes", finalFreeHeap);
    log_i("  Difference:   %ld bytes", memoryDifference);
    
    // Allow for some memory fragmentation but not major leaks
    if (memoryDifference < -5000) { // More than 5KB lost
        log_e("Significant memory loss detected: %ld bytes - FAILED", memoryDifference);
        return;
    }
    log_i("✓ Memory stability acceptable");
    
    validationState.memoryTestPassed = true;
    log_i("✓ Memory Usage Validation: PASSED");
}

/**
 * @brief Phase 5: Performance Validation
 * Test system responsiveness and timing requirements
 */
void validatePerformance() {
    log_i("=== PHASE 5: Performance Validation ===");
    
    if (!interruptManager) {
        log_e("InterruptManager not initialized - FAILED");
        return;
    }
    
    // Test 1: Interrupt processing time
    unsigned long startTime = millis();
    for (int i = 0; i < 100; i++) {
        interruptManager->Process();
    }
    unsigned long processingTime = millis() - startTime;
    
    log_i("Interrupt processing test:");
    log_i("  100 cycles took: %lu ms", processingTime);
    log_i("  Average per cycle: %lu ms", processingTime / 100);
    
    if ((processingTime / 100) > 50) { // More than 50ms average
        log_e("Interrupt processing too slow: %lu ms average - FAILED", processingTime / 100);
        return;
    }
    log_i("✓ Interrupt processing performance acceptable");
    
    // Test 2: Panel switching performance
    startTime = millis();
    for (int i = 0; i < 10; i++) {
        if (panelManager) {
            panelManager->CreateAndLoadPanel(PanelNames::OIL, false);
            panelManager->CreateAndLoadPanel(PanelNames::CONFIG, false);
        }
    }
    unsigned long panelSwitchTime = millis() - startTime;
    
    log_i("Panel switching test:");
    log_i("  20 switches took: %lu ms", panelSwitchTime);
    log_i("  Average per switch: %lu ms", panelSwitchTime / 20);
    
    if ((panelSwitchTime / 20) > 100) { // More than 100ms per switch
        log_e("Panel switching too slow: %lu ms average - FAILED", panelSwitchTime / 20);
        return;
    }
    log_i("✓ Panel switching performance acceptable");
    
    validationState.performanceTestPassed = true;
    log_i("✓ Performance Validation: PASSED");
}

/**
 * @brief Print final validation results
 */
void printValidationResults() {
    log_i("========================================");
    log_i("      SYSTEM VALIDATION RESULTS");
    log_i("========================================");
    
    log_i("Phase 1 - Panel Switching:    %s", validationState.panelSwitchingPassed ? "PASSED" : "FAILED");
    log_i("Phase 2 - Error Handling:     %s", validationState.errorHandlingPassed ? "PASSED" : "FAILED");
    log_i("Phase 3 - Button System:      %s", validationState.buttonSystemPassed ? "PASSED" : "FAILED");
    log_i("Phase 4 - Memory Usage:       %s", validationState.memoryTestPassed ? "PASSED" : "FAILED");
    log_i("Phase 5 - Performance:        %s", validationState.performanceTestPassed ? "PASSED" : "FAILED");
    
    int passedTests = 0;
    if (validationState.panelSwitchingPassed) passedTests++;
    if (validationState.errorHandlingPassed) passedTests++;
    if (validationState.buttonSystemPassed) passedTests++;
    if (validationState.memoryTestPassed) passedTests++;
    if (validationState.performanceTestPassed) passedTests++;
    
    log_i("========================================");
    log_i("OVERALL RESULT: %d/5 tests PASSED", passedTests);
    
    if (passedTests == 5) {
        log_i("🎉 SYSTEM VALIDATION: COMPLETE SUCCESS");
        log_i("All architectural requirements validated!");
    } else {
        log_e("❌ SYSTEM VALIDATION: INCOMPLETE");
        log_e("Some tests failed - review implementation");
    }
    
    log_i("Validation completed in %lu seconds", (millis() - validationState.testStartTime) / 1000);
    log_i("========================================");
}

/**
 * @brief Main validation loop - runs system validation phases
 */
void runSystemValidation() {
    unsigned long currentTime = millis();
    
    // Initialize validation on first run
    if (validationState.testStartTime == 0) {
        validationState.testStartTime = currentTime;
        log_i("🚀 Starting Clarity System Validation");
        log_i("Testing coordinated interrupt architecture implementation");
        return;
    }
    
    // Run validation phases with timing
    if (!validationState.testCompleted) {
        unsigned long phaseDelay = currentTime - validationState.lastPhaseTime;
        
        // Allow 3 seconds between phases for system stability
        if (phaseDelay < 3000) return;
        
        switch (validationState.currentTestPhase) {
            case 0:
                validatePanelSwitching();
                break;
            case 1:
                validateErrorHandling();
                break;
            case 2:
                validateButtonSystem();
                break;
            case 3:
                validateMemoryUsage();
                break;
            case 4:
                validatePerformance();
                validationState.testCompleted = true;
                break;
            default:
                printValidationResults();
                return;
        }
        
        validationState.currentTestPhase++;
        validationState.lastPhaseTime = currentTime;
    } else {
        // Print results once
        static bool resultsPrinted = false;
        if (!resultsPrinted) {
            printValidationResults();
            resultsPrinted = true;
        }
    }
}

// Conditional compilation for validation mode
#ifdef SYSTEM_VALIDATION_MODE

void setup() {
    // Initialize the main system first
    setupMainSystem(); // This would call the main system setup
    
    delay(5000); // Allow system to fully initialize
    log_i("System validation mode enabled");
}

void loop() {
    // Run normal system operations
    loopMainSystem(); // This would call the main system loop
    
    // Run validation tests
    runSystemValidation();
    
    delay(100); // Prevent overwhelming the system
}

#endif // SYSTEM_VALIDATION_MODE