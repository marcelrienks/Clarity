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

    /**
     * @brief Initialize the panel service and register available panels
     */
    virtual void Init() = 0;

    /**
     * @brief Create and load a panel by name with optional completion callback
     * @param panelName Name of the panel to create and load
     * @param completionCallback Optional callback function to execute when loading is complete
     * @param isTriggerDriven Whether this panel change is triggered by an interrupt trigger
     */
    virtual void CreateAndLoadPanel(const char *panelName, std::function<void()> completionCallback = nullptr,
                                    bool isTriggerDriven = false) = 0;

    /**
     * @brief Update the currently active panel (called from main loop)
     */
    virtual void UpdatePanel() = 0;

    // State Management Methods

    /**
     * @brief Set current UI state for synchronization
     * @param state Current UI processing state
     */
    virtual void SetUiState(UIState state) = 0;

    /**
     * @brief Get the current panel name
     * @return Current panel identifier string
     */
    virtual const char *GetCurrentPanel() const = 0;

    /**
     * @brief Get the restoration panel name (panel to restore when triggers are inactive)
     * @return Restoration panel identifier string
     */
    virtual const char *GetRestorationPanel() const = 0;

    /**
     * @brief Check if the current panel is trigger-driven
     * @return True if current panel was loaded by a trigger, false for user-driven panels
     */
    virtual bool IsCurrentPanelTriggerDriven() const = 0;

    // Trigger Integration Methods

    /**
     * @brief Callback executed when trigger-driven panel loading is complete
     * @param triggerId ID of the trigger that initiated the panel switch
     */
    virtual void TriggerPanelSwitchCallback(const char *triggerId) = 0;
};