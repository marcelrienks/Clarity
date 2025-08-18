# TriggerManager Simplification Options

This document outlines various approaches to simplify the TriggerManager and ExecuteTriggerAction function implementation.

## Current Implementation Overview

The current `ExecuteTriggerAction` method uses a nested if-else structure to handle different trigger states and action types. This approach, while functional, can be simplified for better maintainability and readability.

### Current Structure
- Uses if-else chains for action type dispatching
- Handles both ACTIVE and INACTIVE states
- Manages two action types: LoadPanel and ToggleTheme
- Contains logic for finding fallback panels when triggers deactivate

## Option 1: Command Pattern with Action Map

Replace the if-else chain with a map of action handlers for better extensibility.

### Implementation
```cpp
class TriggerManager {
    using ActionHandler = std::function<void(const char*)>;
    std::unordered_map<TriggerActionType, ActionHandler> actionHandlers_;
    
    void InitializeActionHandlers() {
        actionHandlers_[TriggerActionType::LoadPanel] = [this](const char* target) {
            if (panelService_) panelService_->CreateAndLoadPanel(target, true);
        };
        
        actionHandlers_[TriggerActionType::ToggleTheme] = [this](const char* target) {
            if (styleService_) {
                styleService_->SetTheme(target);
                if (panelService_) panelService_->UpdatePanel();
            }
        };
    }
    
    void ExecuteTriggerAction(Trigger* mapping, TriggerExecutionState state) {
        if (state == TriggerExecutionState::ACTIVE && actionHandlers_.count(mapping->actionType)) {
            actionHandlers_[mapping->actionType](mapping->actionTarget);
        }
    }
};
```

### Pros
- Easy to add new action types
- No if-else chains
- Clear separation of action logic

### Cons
- Uses dynamic memory (std::unordered_map)
- More complex initialization
- Function pointer overhead

## Option 2: State Machine Pattern

Simplify state transitions with a cleaner state machine approach.

### Implementation
```cpp
class TriggerManager {
    void ExecuteTriggerAction(Trigger* mapping, TriggerExecutionState state) {
        if (!mapping) return;
        
        // Single dispatch based on state
        auto handler = (state == TriggerExecutionState::ACTIVE) 
            ? &TriggerManager::HandleActivation 
            : &TriggerManager::HandleDeactivation;
            
        (this->*handler)(mapping);
    }
    
private:
    void HandleActivation(Trigger* mapping) {
        switch (mapping->actionType) {
            case TriggerActionType::LoadPanel:
                if (panelService_) 
                    panelService_->CreateAndLoadPanel(mapping->actionTarget, true);
                break;
            case TriggerActionType::ToggleTheme:
                if (styleService_) {
                    styleService_->SetTheme(mapping->actionTarget);
                    if (panelService_) panelService_->UpdatePanel();
                }
                break;
        }
    }
    
    void HandleDeactivation(Trigger* mapping) {
        if (mapping->actionType == TriggerActionType::LoadPanel) {
            Trigger* activePanel = FindActivePanel();
            if (activePanel && panelService_) {
                panelService_->CreateAndLoadPanel(activePanel->actionTarget, true);
            } else if (panelService_) {
                panelService_->CreateAndLoadPanel(mapping->restoreTarget, true);
            }
        }
    }
};
```

### Pros
- Clear state separation
- Easy to understand flow
- Maintains current architecture

### Cons
- Still requires switch/if statements
- More methods to maintain

## Option 3: Direct Action Execution (Remove Abstraction)

Since there are only 2 action types, remove the abstraction layer entirely.

### Implementation
```cpp
class TriggerManager {
    void CheckTriggerChange(const char* triggerId, bool currentPinState) {
        Trigger* mapping = FindTriggerMapping(triggerId);
        if (!mapping) return;
        
        TriggerExecutionState newState = currentPinState 
            ? TriggerExecutionState::ACTIVE 
            : TriggerExecutionState::INACTIVE;
            
        if (newState == mapping->currentState) return;
        
        mapping->currentState = newState;
        
        // Direct execution based on trigger type
        if (strcmp(triggerId, TriggerIds::LIGHTS_STATE) == 0) {
            ExecuteThemeToggle(newState == TriggerExecutionState::ACTIVE 
                ? mapping->actionTarget 
                : mapping->restoreTarget);
        } else {
            ExecutePanelChange(mapping, newState);
        }
    }
    
private:
    void ExecutePanelChange(Trigger* mapping, TriggerExecutionState state) {
        if (!panelService_) return;
        
        if (state == TriggerExecutionState::ACTIVE) {
            panelService_->CreateAndLoadPanel(mapping->actionTarget, true);
        } else {
            // Handle deactivation with fallback logic
            Trigger* activePanel = FindActivePanel();
            const char* targetPanel = activePanel 
                ? activePanel->actionTarget 
                : mapping->restoreTarget;
            panelService_->CreateAndLoadPanel(targetPanel, true);
        }
    }
    
    void ExecuteThemeToggle(const char* themeName) {
        if (styleService_) {
            styleService_->SetTheme(themeName);
            if (panelService_) panelService_->UpdatePanel();
        }
    }
};
```

### Pros
- Very direct and simple
- No abstraction overhead
- Fastest execution

### Cons
- Less flexible for future action types
- Couples trigger logic with action execution

## Option 4: Action Registry with Dependency Injection

Move action execution out of TriggerManager into separate action executors.

### Implementation
```cpp
// Base action executor interface
class IActionExecutor {
public:
    virtual ~IActionExecutor() = default;
    virtual void Execute(const Trigger& trigger, TriggerExecutionState state) = 0;
    virtual TriggerActionType GetActionType() const = 0;
};

// Panel action executor
class PanelActionExecutor : public IActionExecutor {
    IPanelService* panelService_;
    ITriggerService* triggerService_;
    
public:
    PanelActionExecutor(IPanelService* panel, ITriggerService* trigger) 
        : panelService_(panel), triggerService_(trigger) {}
    
    TriggerActionType GetActionType() const override { 
        return TriggerActionType::LoadPanel; 
    }
    
    void Execute(const Trigger& trigger, TriggerExecutionState state) override {
        if (!panelService_) return;
        
        if (state == TriggerExecutionState::ACTIVE) {
            panelService_->CreateAndLoadPanel(trigger.actionTarget, true);
        } else {
            // Deactivation logic with fallback
            const char* fallbackPanel = DetermineFallbackPanel(trigger);
            panelService_->CreateAndLoadPanel(fallbackPanel, true);
        }
    }
    
private:
    const char* DetermineFallbackPanel(const Trigger& trigger) {
        // Logic to find active panel or use restore target
        return trigger.restoreTarget;
    }
};

// Theme action executor
class ThemeActionExecutor : public IActionExecutor {
    IStyleService* styleService_;
    IPanelService* panelService_;
    
public:
    ThemeActionExecutor(IStyleService* style, IPanelService* panel) 
        : styleService_(style), panelService_(panel) {}
    
    TriggerActionType GetActionType() const override { 
        return TriggerActionType::ToggleTheme; 
    }
    
    void Execute(const Trigger& trigger, TriggerExecutionState state) override {
        if (!styleService_) return;
        
        const char* theme = (state == TriggerExecutionState::ACTIVE) 
            ? trigger.actionTarget 
            : trigger.restoreTarget;
            
        styleService_->SetTheme(theme);
        if (panelService_) panelService_->UpdatePanel();
    }
};

// Updated TriggerManager
class TriggerManager {
    std::vector<std::unique_ptr<IActionExecutor>> actionExecutors_;
    
    void RegisterActionExecutors() {
        actionExecutors_.push_back(
            std::make_unique<PanelActionExecutor>(panelService_, this));
        actionExecutors_.push_back(
            std::make_unique<ThemeActionExecutor>(styleService_, panelService_));
    }
    
    void ExecuteTriggerAction(Trigger* mapping, TriggerExecutionState state) {
        for (auto& executor : actionExecutors_) {
            if (executor->GetActionType() == mapping->actionType) {
                executor->Execute(*mapping, state);
                break;
            }
        }
    }
};
```

### Pros
- Complete separation of concerns
- Easy to unit test each executor
- Follows SOLID principles
- New action types don't modify TriggerManager

### Cons
- Most complex solution
- Uses dynamic memory allocation
- Over-engineered for only 2 action types

## Option 5: Simplify with Early Returns (Recommended)

Keep the current structure but make it cleaner with early returns and extracted methods.

### Implementation
```cpp
class TriggerManager {
    void ExecuteTriggerAction(Trigger* mapping, TriggerExecutionState state) {
        if (!mapping) return;
        
        if (state == TriggerExecutionState::ACTIVE) {
            ExecuteActivation(mapping);
            return;
        }
        
        ExecuteDeactivation(mapping);
    }

private:
    void ExecuteActivation(Trigger* mapping) {
        switch (mapping->actionType) {
            case TriggerActionType::LoadPanel:
                LoadPanel(mapping->actionTarget);
                break;
                
            case TriggerActionType::ToggleTheme:
                SetTheme(mapping->actionTarget);
                break;
        }
    }
    
    void ExecuteDeactivation(Trigger* mapping) {
        // Only panel loads need deactivation handling
        if (mapping->actionType != TriggerActionType::LoadPanel) {
            return;
        }
        
        // Find another active panel or use restore target
        Trigger* activePanel = FindActivePanel();
        const char* targetPanel = activePanel 
            ? activePanel->actionTarget 
            : mapping->restoreTarget;
            
        LoadPanel(targetPanel);
    }
    
    void LoadPanel(const char* panelName) {
        if (!panelService_ || !panelName) return;
        
        log_i("Loading panel: %s", panelName);
        panelService_->CreateAndLoadPanel(panelName, true);
    }
    
    void SetTheme(const char* themeName) {
        if (!styleService_ || !themeName) return;
        
        log_i("Setting theme: %s", themeName);
        styleService_->SetTheme(themeName);
        
        if (panelService_) {
            panelService_->UpdatePanel();
        }
    }
};
```

### Pros
- Minimal changes to existing code
- Clear and readable
- No dynamic memory allocation
- Easy to debug
- Maintains current architecture

### Cons
- Still uses switch statement
- Less flexible than command pattern

## Recommendation

**Option 5 (Simplify with Early Returns)** is recommended for the following reasons:

1. **ESP32 Constraints**: No dynamic memory allocation, suitable for embedded systems
2. **Minimal Risk**: Small incremental changes to existing code
3. **Clarity**: Each method has a single responsibility
4. **Maintainability**: Easy to understand and modify
5. **Performance**: No function pointer overhead or map lookups
6. **Pragmatic**: Appropriate complexity for only 2 action types

## Implementation Steps

1. Extract `ExecuteActivation` and `ExecuteDeactivation` methods
2. Create helper methods `LoadPanel` and `SetTheme`
3. Simplify the main `ExecuteTriggerAction` method
4. Add appropriate logging
5. Update unit tests

## Future Considerations

If more than 5-6 action types are added in the future, consider migrating to Option 1 (Command Pattern) or Option 4 (Action Registry) for better scalability.