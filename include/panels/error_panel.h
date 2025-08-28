#pragma once

#include "components/error_component.h"
#include "interfaces/i_action_service.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_style_service.h"
#include "managers/error_manager.h"
#include "utilities/types.h"

// Forward declarations
class IComponentFactory;

/**
 * @class ErrorPanel
 * @brief Application error display and management panel
 *
 * @details This panel displays pending application errors in a scrollable list
 * format. It provides user interaction for acknowledging errors and shows
 * appropriate visual indicators based on error severity levels.
 *
 * @presenter_role Coordinates error display with ErrorManager data
 * @data_source ErrorManager providing error queue information
 * @update_strategy Real-time error list updates with auto-refresh
 *
 * @ui_layout:
 * - Header: Error count and severity indicator
 * - List: Scrollable list of individual error entries
 * - Footer: Clear all errors button (for non-critical errors)
 *
 * @visual_feedback:
 * - Critical: Red background/border for critical errors
 * - Error: Orange background/border for regular errors
 * - Warning: Yellow background/border for warnings
 * - Auto-dismiss indicators for warnings
 *
 * @context This panel provides comprehensive error management interface.
 * It automatically appears when errors occur and manages user acknowledgment
 * and dismissal workflows.
 */
class ErrorPanel : public IPanel
{
  public:
    // Constructors and Destructors
    ErrorPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService,
               IComponentFactory* componentFactory = nullptr);
    ~ErrorPanel();

    // Core Functionality Methods
    static constexpr const char *NAME = PanelNames::ERROR;
    void Init() override;
    void Load(std::function<void()> callbackFunction) override;
    void Update(std::function<void()> callbackFunction) override;

    // Manager injection method
    void SetManagers(IPanelService *panelService, IStyleService *styleService) override;

    // IActionService Interface Implementation (inherited through IPanel)
    void (*GetShortPressFunction())(void* panelContext) override;
    void (*GetLongPressFunction())(void* panelContext) override;
    void* GetPanelContext() override;
    
    // Public action handlers
    void HandleShortPress();
    void HandleLongPress();

  private:
    // Static Methods
    static void ShowPanelCompletionCallback(lv_event_t *event);

    // Instance Data Members
    IGpioProvider *gpioProvider_;
    IDisplayProvider *displayProvider_;
    IStyleService *styleService_;
    IPanelService *panelService_;
    IComponentFactory *componentFactory_;
    // screen_ is inherited from IPanel base class
    std::shared_ptr<IComponent> errorComponent_; // Error component
    ComponentLocation centerLocation_;               // Component positioning
    bool panelLoaded_;                               // Track panel load state
    std::vector<ErrorInfo> currentErrors_;           // Cache of current error state
    const char *previousTheme_;                      // Store previous theme to restore on exit
    
    // Error cycling functionality
    size_t currentErrorIndex_;                       // Index of currently displayed error
    
    void SortErrorsBySeverity();                    // Sort errors by severity (CRITICAL first, WARNING last)
    void AdvanceToNextError();                      // Move to next error and update component
};