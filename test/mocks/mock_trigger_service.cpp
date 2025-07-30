#include "mock_trigger_service.h"
#include "mock_types.h"

MockTriggerService::MockTriggerService()
    : initCalled_(false)
    , processTriggerEventsCalled_(false)
    , executeTriggerActionCalled_(false)
    , processEventsCallCount_(0)
    , executeActionCallCount_(0)
{
}

void MockTriggerService::init()
{
    initCalled_ = true;
    
    // Initialize all triggers to INIT state
    for (auto& trigger : mockTriggers_) {
        triggerStates_[trigger.triggerId] = TriggerExecutionState::INIT;
    }
}

void MockTriggerService::processTriggerEvents()
{
    processTriggerEventsCalled_ = true;
    processEventsCallCount_++;
    
    if (processEventsCallback_) {
        processEventsCallback_();
    }
    
    // Process each mock trigger
    for (auto& trigger : mockTriggers_) {
        processIndividualTrigger(trigger);
    }
}

void MockTriggerService::executeTriggerAction(Trigger* mapping, TriggerExecutionState state)
{
    executeTriggerActionCalled_ = true;
    executeActionCallCount_++;
    
    if (mapping) {
        executedActions_.emplace_back(mapping, state);
        triggerStates_[mapping->triggerId] = state;
    }
    
    if (executeActionCallback_) {
        executeActionCallback_(mapping, state);
    }
}

const char* MockTriggerService::getStartupPanelOverride() const
{
    return startupPanelOverride_.empty() ? nullptr : startupPanelOverride_.c_str();
}

void MockTriggerService::reset()
{
    initCalled_ = false;
    processTriggerEventsCalled_ = false;
    executeTriggerActionCalled_ = false;
    processEventsCallCount_ = 0;
    executeActionCallCount_ = 0;
    
    mockTriggers_.clear();
    pinStates_.clear();
    executedActions_.clear();
    triggerStates_.clear();
    startupPanelOverride_.clear();
    
    processEventsCallback_ = nullptr;
    executeActionCallback_ = nullptr;
}

void MockTriggerService::setStartupPanelOverride(const char* panelName)
{
    startupPanelOverride_ = panelName ? panelName : "";
}

void MockTriggerService::simulatePinState(int pin, bool state)
{
    pinStates_[pin] = state;
}

void MockTriggerService::addMockTrigger(const Trigger& trigger)
{
    mockTriggers_.push_back(trigger);
    triggerStates_[trigger.triggerId] = TriggerExecutionState::INIT;
    
    // Initialize pin state if not already set
    if (pinStates_.find(trigger.pin) == pinStates_.end()) {
        pinStates_[trigger.pin] = false;
    }
}

void MockTriggerService::clearMockTriggers()
{
    mockTriggers_.clear();
    triggerStates_.clear();
}

void MockTriggerService::setProcessEventsCallback(std::function<void()> callback)
{
    processEventsCallback_ = callback;
}

void MockTriggerService::setExecuteActionCallback(std::function<void(const Trigger*, TriggerExecutionState)> callback)
{
    executeActionCallback_ = callback;
}

bool MockTriggerService::isPinActive(int pin) const
{
    auto it = pinStates_.find(pin);
    return it != pinStates_.end() ? it->second : false;
}

void MockTriggerService::setTriggerState(const char* triggerId, TriggerExecutionState state)
{
    if (triggerId) {
        triggerStates_[triggerId] = state;
    }
}

TriggerExecutionState MockTriggerService::getTriggerState(const char* triggerId) const
{
    if (!triggerId) {
        return TriggerExecutionState::INIT;
    }
    
    auto it = triggerStates_.find(triggerId);
    return it != triggerStates_.end() ? it->second : TriggerExecutionState::INIT;
}

void MockTriggerService::processIndividualTrigger(Trigger& trigger)
{
    bool currentPinState = isPinActive(trigger.pin);
    TriggerExecutionState currentState = getTriggerState(trigger.triggerId);
    
    // Determine if trigger should activate based on pin state
    bool shouldActivate = shouldTriggerActivate(trigger, currentPinState);
    
    TriggerExecutionState newState = currentState;
    
    if (shouldActivate && currentState != TriggerExecutionState::ACTIVE) {
        newState = TriggerExecutionState::ACTIVE;
    } else if (!shouldActivate && currentState == TriggerExecutionState::ACTIVE) {
        newState = TriggerExecutionState::INACTIVE;
    }
    
    // Execute action if state changed
    if (newState != currentState) {
        executeTriggerAction(&trigger, newState);
    }
}

bool MockTriggerService::shouldTriggerActivate(const Trigger& trigger, bool currentPinState) const
{
    // Simple logic: trigger activates when pin is high
    // More complex logic can be added for specific trigger types
    return currentPinState;
}