#pragma once

/**
 * @interface IActionExecutionService
 * @brief Interface for button action execution in callback-free architecture
 *
 * @details This interface enables centralized button action handling without
 * std::function callbacks. Actions call this interface directly when button
 * events occur, maintaining system consistency across the callback elimination.
 *
 * @design_pattern Dependency Injection - Used by action handlers for testability
 * @memory_safety Zero heap allocation - Interface calls compile to direct calls
 *
 * @implementations:
 * - PanelManager: Routes button actions to current panel
 * - Mock implementations: For unit testing action behavior
 */
class IActionExecutionService {
public:
    virtual ~IActionExecutionService() = default;
    
    /// @brief Handle short button press events
    /// Routes to current panel's short press handler
    virtual void HandleShortPress() = 0;
    
    /// @brief Handle long button press events  
    /// Routes to current panel's long press handler
    virtual void HandleLongPress() = 0;
};