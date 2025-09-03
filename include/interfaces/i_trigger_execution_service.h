#pragma once

#include "utilities/types.h"

/**
 * @interface ITriggerExecutionService
 * @brief Interface for trigger-based panel switching in callback-free architecture
 *
 * @details This interface enables triggers to request panel changes without
 * std::function callbacks. Triggers call this interface directly when state
 * changes occur, maintaining consistency with the callback elimination approach.
 *
 * @design_pattern Dependency Injection - Used by trigger handlers for testability
 * @memory_safety Zero heap allocation - Interface calls compile to direct calls
 *
 * @implementations:
 * - PanelManager: Main implementation for trigger-driven panel switching
 * - Mock implementations: For unit testing trigger behavior
 */
class ITriggerExecutionService {
public:
    virtual ~ITriggerExecutionService() = default;
    
    /// @brief Load a specific panel by name
    /// @param panelName Name of the panel to load
    virtual void LoadPanel(const char* panelName) = 0;
    
    /// @brief Check restoration conditions and load appropriate panel
    /// Called when triggers deactivate to restore previous state
    virtual void CheckRestoration() = 0;
};