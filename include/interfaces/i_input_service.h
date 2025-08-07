#pragma once

/**
 * @interface IInputService
 * @brief Interface for handling button input events in panels
 * 
 * @details This interface defines the contract for panels to handle button input events.
 * Panels implementing this interface can respond to short press and long press events
 * with custom behavior appropriate to their functionality.
 * 
 * @design_pattern Interface Segregation - Focused on input handling only
 * @separation Isolated from trigger system - dedicated to button input
 * @timing Short press: 50ms-500ms, Long press: >500ms
 */
class IInputService
{
public:
    virtual ~IInputService() = default;

    /**
     * @brief Handle short button press event (50ms - 500ms)
     * @details Called when button is pressed and released within the short press window
     */
    virtual void OnShortPress() = 0;

    /**
     * @brief Handle long button press event (>500ms)
     * @details Called when button is held down for longer than the long press threshold
     */
    virtual void OnLongPress() = 0;
};