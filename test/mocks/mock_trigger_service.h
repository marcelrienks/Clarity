#pragma once

// System/Library Includes
#include <functional>
#include <vector>
#include <string>
#include <map>

// Project Includes
#include "interfaces/i_trigger_service.h"
#include "utilities/types.h"

/**
 * @class MockTriggerService
 * @brief Mock implementation of ITriggerService for testing
 * 
 * @details Provides a testable implementation of the trigger service interface
 * with controllable behavior for unit tests. Simulates GPIO trigger detection
 * and action execution without requiring actual hardware.
 * 
 * @testability Allows verification of trigger processing and action execution
 * @dependency_free No actual GPIO dependency - uses simulated pin states
 */
class MockTriggerService : public ITriggerService
{
public:
    MockTriggerService();
    virtual ~MockTriggerService() = default;

    // Core Functionality Methods
    void init() override;
    void processTriggerEvents() override;
    void executeTriggerAction(Trigger* mapping, TriggerExecutionState state) override;

    // Startup Configuration Methods
    const char* getStartupPanelOverride() const override;

    // Test Helper Methods
    bool wasInitCalled() const { return initCalled_; }
    bool wasProcessTriggerEventsCalled() const { return processTriggerEventsCalled_; }
    bool wasExecuteTriggerActionCalled() const { return executeTriggerActionCalled_; }
    int getProcessEventsCallCount() const { return processEventsCallCount_; }
    int getExecuteActionCallCount() const { return executeActionCallCount_; }

    // Test Control Methods
    void reset();
    void setStartupPanelOverride(const char* panelName);
    void simulatePinState(int pin, bool state);
    void addMockTrigger(const Trigger& trigger);
    void clearMockTriggers();
    void setProcessEventsCallback(std::function<void()> callback);
    void setExecuteActionCallback(std::function<void(const Trigger*, TriggerExecutionState)> callback);

    // Test Data Access
    const std::vector<Trigger>& getMockTriggers() const { return mockTriggers_; }
    const std::map<int, bool>& getPinStates() const { return pinStates_; }
    const std::vector<std::pair<const Trigger*, TriggerExecutionState>>& getExecutedActions() const { return executedActions_; }
    bool isPinActive(int pin) const;

    // Trigger State Management
    void setTriggerState(const char* triggerId, TriggerExecutionState state);
    TriggerExecutionState getTriggerState(const char* triggerId) const;

private:
    // Mock Data
    std::vector<Trigger> mockTriggers_;
    std::map<int, bool> pinStates_;
    std::string startupPanelOverride_;

    // State Tracking
    bool initCalled_;
    bool processTriggerEventsCalled_;
    bool executeTriggerActionCalled_;
    int processEventsCallCount_;
    int executeActionCallCount_;

    // Test Data
    std::vector<std::pair<const Trigger*, TriggerExecutionState>> executedActions_;
    std::map<std::string, TriggerExecutionState> triggerStates_;

    // Test Callbacks
    std::function<void()> processEventsCallback_;
    std::function<void(const Trigger*, TriggerExecutionState)> executeActionCallback_;

    void processIndividualTrigger(Trigger& trigger);
    bool shouldTriggerActivate(const Trigger& trigger, bool currentPinState) const;
};