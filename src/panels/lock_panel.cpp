#include "panels/lock_panel.h"
#include "components/lock_component.h"
#include "managers/error_manager.h"
#include "utilities/logging.h"
#include <esp32-hal-log.h>
#include <variant>

// Constructors and Destructors
LockPanel::LockPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService,
                     IComponentFactory* componentFactory)
    : BasePanel(gpio, display, styleService, componentFactory),
      lockComponent_(styleService), componentInitialized_(false)
{
    // Note: LockPanel is display-only, state comes from interrupt system
    // Since this panel is only loaded when lock state changes, assume engaged
    isLockEngaged_ = true;
}

// BasePanel template method implementations
void LockPanel::CreateContent()
{
    // Component is now stack-allocated and initialized in constructor
    componentInitialized_ = true;

    // Create the lock component centered on screen - always shows red (engaged) state
    lockComponent_.Render(screen_, centerLocation_, displayProvider_);
}

void LockPanel::UpdateContent()
{
    // Lock panel is static - no updates needed
    // State changes are handled by the interrupt system loading different panels
}

// Panel-specific button handling
void LockPanel::HandleLongPress()
{
    if (panelService_)
    {
        log_i("LockPanel long press - loading config panel");
        panelService_->CreateAndLoadPanel(PanelNames::CONFIG, true);
    }
    else
    {
        log_w("LockPanel: Cannot load config panel - panelService not available");
    }
}