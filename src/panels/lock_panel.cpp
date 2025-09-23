#include "panels/lock_panel.h"
#include "components/lock_component.h"
#include "managers/error_manager.h"
#include "utilities/logging.h"
#include <esp32-hal-log.h>
#include <variant>

// ========== Constructors and Destructor ==========

/**
 * @brief Constructs lock panel for vehicle security status display
 * @param gpio GPIO provider for hardware interaction
 * @param display Display provider for screen management
 * @param styleManager Style service for theme-based styling
 *
 * Creates lock panel that displays vehicle security lock status. Assumes
 * lock engaged state since panel is only loaded when lock state changes.
 * Uses stack-allocated component for efficient memory management.
 */
LockPanel::LockPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleManager *styleManager)
    : BasePanel(gpio, display, styleManager),
      lockComponent_(styleManager), componentInitialized_(false)
{
    // Note: LockPanel is display-only, state comes from interrupt system
    // Since this panel is only loaded when lock state changes, assume engaged
    isLockEngaged_ = true;
}

// ========== Protected Methods ==========

/**
 * @brief Creates lock panel UI content with security status display
 *
 * Renders lock component on screen showing current lock engagement state.
 * Always displays red (engaged) state as panel is triggered by lock events.
 * Provides immediate visual confirmation of vehicle security activation.
 */
void LockPanel::CreateContent()
{
    // Component is now stack-allocated and initialized in constructor
    componentInitialized_ = true;

    // Create the lock component centered on screen - always shows red (engaged) state
    lockComponent_.Render(screen_, centerLocation_, displayProvider_);
}

/**
 * @brief Updates lock panel content (static panel - no updates needed)
 *
 * Lock panel displays static state determined by trigger system.
 * State changes result in panel transitions rather than content updates
 * within the same panel instance. Interrupt system manages state changes.
 */
void LockPanel::UpdateContent()
{
    // Lock panel is static - no updates needed
    // State changes are handled by the interrupt system loading different panels
}

/**
 * @brief Handles long press button action to access configuration menu
 *
 * Responds to long press by loading configuration panel, providing access
 * to system settings from the lock display. Essential for automotive user
 * interface navigation from security status screen.
 */
void LockPanel::HandleLongPress()
{
    if (panelManager_)
    {
        log_i("LockPanel long press - loading config panel");
        panelManager_->CreateAndLoadPanel(PanelNames::CONFIG, true);
    }
    else
    {
        log_w("LockPanel: Cannot load config panel - panelManager not available");
    }
}