#pragma once

/**
 * @interface IActionService
 * @brief Interface for panels to provide static button function pointers for universal button system
 *
 * @details This interface defines the contract for panels to provide static function pointers
 * for button input handling. The universal button system injects these functions into
 * QUEUED interrupts, enabling all panels to respond to button input consistently.
 *
 * @design_pattern Function injection pattern with static callbacks for memory safety
 * @execution_flow Panel provides functions → QueuedHandler injects into interrupts → 
 *                 InterruptManager coordinates execution with panel context
 * @timing Short press: 50ms-2000ms, Long press: 2000ms-5000ms
 *
 * @memory_safety All function pointers must be static to prevent heap fragmentation
 * on ESP32. Function execution receives panel context via void* parameter for
 * access to panel state without heap allocation.
 *
 * @universal_button_system Integration:
 * 1. Panel implements IActionService with static callback functions
 * 2. PanelManager extracts functions when panel loads
 * 3. Functions injected into universal button interrupts (short_press/long_press)
 * 4. Button events execute current panel's functions with panel context
 * 5. Panel switching updates injected functions automatically
 *
 * @context_pattern All function pointers receive void* panelContext parameter
 * that contains the current panel instance for state access during execution.
 */
class IActionService
{
public:
    virtual ~IActionService() = default;

    /**
     * @brief Get static function pointer for short button press (50ms - 2000ms)
     * @details Returns static callback function that will be injected into
     * universal short press interrupt. Function receives panel context.
     * @return Static function pointer with signature: void(*)(void* panelContext)
     */
    virtual void (*GetShortPressFunction())(void* panelContext) = 0;

    /**
     * @brief Get static function pointer for long button press (2000ms - 5000ms)
     * @details Returns static callback function that will be injected into
     * universal long press interrupt. Function receives panel context.
     * @return Static function pointer with signature: void(*)(void* panelContext)
     */
    virtual void (*GetLongPressFunction())(void* panelContext) = 0;

    /**
     * @brief Get panel context for function execution
     * @details Returns pointer to panel instance that will be passed to
     * button functions as context parameter for state access.
     * @return void* pointer to panel instance (typically 'this')
     */
    virtual void* GetPanelContext() = 0;
};