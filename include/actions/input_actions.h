#pragma once

#include "interfaces/i_input_action.h"
#include "interfaces/i_panel_service.h"
#include <memory>
#include <functional>

/**
 * @class PanelSwitchAction
 * @brief Action to switch to a different panel
 */
class PanelSwitchAction : public IInputAction
{
public:
    PanelSwitchAction(IPanelService* panelService, const char* targetPanel);
    void Execute() override;
    const char* GetDescription() const override;
    const char* GetActionType() const override { return "PanelSwitchAction"; }

private:
    IPanelService* panelService_;
    const char* targetPanel_;
    mutable char description_[64];
};

/**
 * @class SimplePanelSwitchAction
 * @brief Action to request a panel switch without needing IPanelService at creation
 */
class SimplePanelSwitchAction : public IInputAction
{
public:
    SimplePanelSwitchAction(const char* targetPanel, std::function<void(const char*)> onExecute = nullptr);
    void Execute() override;
    const char* GetDescription() const override;
    const char* GetActionType() const override { return "SimplePanelSwitchAction"; }
    const char* GetTargetPanel() const { return targetPanel_; }

private:
    const char* targetPanel_;
    std::function<void(const char*)> onExecute_;
    mutable char description_[64];
};

/**
 * @class SkipAnimationAction  
 * @brief Action to skip current animation and proceed immediately
 */
class SkipAnimationAction : public IInputAction
{
public:
    SkipAnimationAction(std::function<void()> skipCallback);
    void Execute() override;
    const char* GetDescription() const override;
    const char* GetActionType() const override { return "SkipAnimationAction"; }

private:
    std::function<void()> skipCallback_;
};

/**
 * @class MenuNavigationAction
 * @brief Action to navigate through menu options
 */
class MenuNavigationAction : public IInputAction
{
public:
    enum Direction { NEXT, PREVIOUS, SELECT };
    
    MenuNavigationAction(Direction direction, std::function<void(Direction)> navigationCallback);
    void Execute() override;
    const char* GetDescription() const override;
    const char* GetActionType() const override { return "MenuNavigationAction"; }

private:
    Direction direction_;
    std::function<void(Direction)> navigationCallback_;
    mutable char description_[64];
};

/**
 * @class NoAction
 * @brief Null object pattern - represents no action to take
 */
class NoAction : public IInputAction
{
public:
    void Execute() override {} // Do nothing
    const char* GetDescription() const override { return "No action"; }
    const char* GetActionType() const override { return "NoAction"; }
};