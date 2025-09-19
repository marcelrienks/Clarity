#pragma once

/**
 * @interface IActionService
 * @brief Interface for panels to handle button actions directly
 *
 * @details Simplified interface where panels handle button actions directly
 * instead of providing function pointers. This eliminates the unnecessary
 * indirection through static trampoline functions.
 *
 * @timing Short press: 50ms-2000ms, Long press: 2000ms-5000ms
 */
class IActionService
{
public:
    virtual ~IActionService() = default;

    /**
     * @brief Handle short button press (50ms - 2000ms)
     * @details Called directly when a short press is detected
     */
    virtual void HandleShortPress() = 0;

    /**
     * @brief Handle long button press (2000ms - 5000ms)
     * @details Called directly when a long press is detected
     */
    virtual void HandleLongPress() = 0;
};