#include "actions/input_actions.h"
#include <cstring>
#include <cstdio>

#ifdef CLARITY_DEBUG
#include "esp32-hal-log.h"
#define LOG_TAG "InputActions"
#else
#define log_d(...)
#define log_i(...)
#endif

// PanelSwitchAction Implementation

PanelSwitchAction::PanelSwitchAction(IPanelService* panelService, const char* targetPanel)
    : panelService_(panelService), targetPanel_(targetPanel)
{
}

void PanelSwitchAction::Execute()
{
    if (panelService_ && targetPanel_) {
        log_i("Executing panel switch to: %s", targetPanel_);
        panelService_->CreateAndLoadPanel(targetPanel_);
    }
}

const char* PanelSwitchAction::GetDescription() const
{
    snprintf(description_, sizeof(description_), "Switch to panel: %s", 
             targetPanel_ ? targetPanel_ : "unknown");
    return description_;
}

// SimplePanelSwitchAction Implementation

SimplePanelSwitchAction::SimplePanelSwitchAction(const char* targetPanel, std::function<void(const char*)> onExecute)
    : targetPanel_(targetPanel), onExecute_(onExecute)
{
}

void SimplePanelSwitchAction::Execute()
{
    if (onExecute_) {
        log_i("Executing simple panel switch callback for: %s", targetPanel_);
        onExecute_(targetPanel_);
    } else {
        log_i("Simple panel switch requested (no callback): %s", targetPanel_);
        // The action execution will be handled by the InputManager
    }
}

const char* SimplePanelSwitchAction::GetDescription() const
{
    snprintf(description_, sizeof(description_), "Switch to panel: %s", 
             targetPanel_ ? targetPanel_ : "unknown");
    return description_;
}

// SkipAnimationAction Implementation

SkipAnimationAction::SkipAnimationAction(std::function<void()> skipCallback)
    : skipCallback_(skipCallback)
{
}

void SkipAnimationAction::Execute()
{
    if (skipCallback_) {
        log_i("Executing skip animation action");
        skipCallback_();
    }
}

const char* SkipAnimationAction::GetDescription() const
{
    return "Skip current animation";
}

// MenuNavigationAction Implementation

MenuNavigationAction::MenuNavigationAction(Direction direction, std::function<void(Direction)> navigationCallback)
    : direction_(direction), navigationCallback_(navigationCallback)
{
}

void MenuNavigationAction::Execute()
{
    if (navigationCallback_) {
        log_i("Executing menu navigation action: %s", GetDescription());
        navigationCallback_(direction_);
    }
}

const char* MenuNavigationAction::GetDescription() const
{
    const char* directionStr = "";
    switch (direction_) {
        case NEXT: directionStr = "Next"; break;
        case PREVIOUS: directionStr = "Previous"; break;
        case SELECT: directionStr = "Select"; break;
    }
    
    snprintf(description_, sizeof(description_), "Menu navigation: %s", directionStr);
    return description_;
}