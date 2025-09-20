#pragma once

// System/Library Includes
#include <functional>
#include <memory>

// Project Includes
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_panel.h"
#include "utilities/types.h"

/**
 * @interface IPanelService
 * @brief Interface for panel lifecycle management and transitions
 *
 * @details This interface abstracts panel management functionality,
 * providing access to panel creation, loading, updating, and transitions.
 * Implementations should handle panel factory registration, dynamic panel
 * creation, lifecycle management, and state synchronization.
 *
 * @design_pattern Interface Segregation - Focused on panel operations only
 * @testability Enables mocking for unit tests with mock panels
 * @dependency_injection Replaces direct PanelManager singleton access
 * @hardware_dependencies Requires IGpioProvider and IDisplayProvider
 */
class IPanelService
{
  public:
    virtual ~IPanelService() = default;

    // Core Functionality Methods
    virtual void Init() = 0;
    virtual void CreateAndLoadPanel(const char *panelName, bool isTriggerDriven = false) = 0;
    virtual void UpdatePanel() = 0;

    // State Management Methods
    virtual void SetUiState(UIState state) = 0;
    virtual UIState GetUiState() const = 0;
    virtual const char *GetCurrentPanel() const = 0;
    virtual const char *GetRestorationPanel() const = 0;
    virtual bool IsCurrentPanelTriggerDriven() const = 0;

    // Trigger Integration Methods
    virtual void TriggerPanelSwitchCallback(const char *triggerId) = 0;
};