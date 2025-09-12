#include "panels/error_panel.h"
#include "components/error_component.h"
#include "factories/component_factory.h"
#include "interfaces/i_component_factory.h"
#include "managers/style_manager.h"
#include "managers/panel_manager.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <algorithm>

// Constructors and Destructors
ErrorPanel::ErrorPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService,
                       IComponentFactory* componentFactory)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService), panelService_(nullptr),
      componentFactory_(componentFactory ? componentFactory : &ComponentFactory::Instance()),
      panelLoaded_(false), previousTheme_(), currentErrorIndex_(0)
{
    log_v("ErrorPanel constructor called");
}

ErrorPanel::~ErrorPanel()
{
    log_v("~ErrorPanel() destructor called");

    if (screen_)
    {
        lv_obj_delete(screen_);
    }

    if (errorComponent_)
    {
        errorComponent_.reset();
    }

    // Reset error panel active flag when panel is destroyed
    ErrorManager::Instance().SetErrorPanelActive(false);

    // Don't restore theme here - let the trigger system manage themes
    // The CheckRestoration() method will apply the correct theme based on active triggers
}

// Core Functionality Methods
/// @brief Initialize the error panel and its UI structure
void ErrorPanel::Init()
{
    log_v("Init() called");

    if (!displayProvider_ || !gpioProvider_)
    {
        log_e("ErrorPanel requires display and gpio providers");
        ErrorManager::Instance().ReportCriticalError("ErrorPanel",
                                                     "Missing required providers - display or gpio provider is null");
        return;
    }

    screen_ = displayProvider_->CreateScreen();

    // Store current theme before switching to ERROR theme (but don't apply it yet)
    if (styleService_)
    {
        previousTheme_ = styleService_->GetCurrentTheme();
    }

    centerLocation_ = ComponentLocation(LV_ALIGN_CENTER, 0, 0);

    // Set error panel as active in ErrorManager
    ErrorManager::Instance().SetErrorPanelActive(true);
    log_i("Error panel activated - switching to error display mode");
    
    log_i("ErrorPanel initialization completed");
}

/// @brief Load the error panel UI components
void ErrorPanel::Load()
{
    log_v("Load() called");


    if (!componentFactory_)
    {
        log_e("ErrorPanel requires component factory");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ErrorPanel", "ComponentFactory is null");
        return;
    }

    // Create component using injected factory
    errorComponent_ = componentFactory_->CreateErrorComponent(styleService_);
    if (!errorComponent_)
    {
        log_e("Failed to create error component");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ErrorPanel", "Component creation failed");
        return;
    }

    if (!displayProvider_)
    {
        log_e("ErrorPanel load requires display provider");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ErrorPanel",
                                             "Cannot render component - display provider is null");
        return;
    }

    // Render the component
    errorComponent_->Render(screen_, centerLocation_, displayProvider_);

    // Get current errors and refresh component
    std::vector<ErrorInfo> currentErrors = ErrorManager::Instance().GetErrorQueue();
    errorComponent_->Refresh(Reading{}); // Component will fetch errors internally

    lv_obj_add_event_cb(screen_, ErrorPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);

    // Add safety check before screen load
    if (!screen_)
    {
        log_e("Screen is null, cannot load error panel");
        ErrorManager::Instance().ReportCriticalError("ErrorPanel", "Cannot load panel - screen creation failed");
        // Error condition - no callback needed
        return;
    }

    lv_screen_load(screen_);

    // Ensure ERROR theme is applied when panel is loaded
    if (styleService_)
    {
        styleService_->SetTheme(Themes::ERROR);
        styleService_->ApplyThemeToScreen(screen_);
    }

    panelLoaded_ = true;
    
    log_t("ErrorPanel loaded successfully");
}

/// @brief Update the error panel with current error data
void ErrorPanel::Update()
{
    log_v("Update() called");


    // Get current errors from ErrorManager
    std::vector<ErrorInfo> newErrors = ErrorManager::Instance().GetErrorQueue();

    // Check if error list has changed
    bool errorsChanged = false;
    if (newErrors.size() != currentErrors_.size())
    {
        errorsChanged = true;
    }
    else
    {
        // Compare timestamps to detect changes
        for (size_t i = 0; i < newErrors.size() && !errorsChanged; i++)
        {
            if (newErrors[i].timestamp != currentErrors_[i].timestamp ||
                newErrors[i].acknowledged != currentErrors_[i].acknowledged)
            {
                errorsChanged = true;
            }
        }
    }

    // Update display if errors have changed
    if (errorsChanged)
    {
        currentErrors_ = newErrors;
        
        // Sort errors by severity (CRITICAL first, WARNING last)
        SortErrorsBySeverity();
        
        // Start from first error (highest severity)
        currentErrorIndex_ = 0;
        
        // Tell component to display all errors with current index
        if (errorComponent_)
        {
            auto errorComp = std::static_pointer_cast<ErrorComponent>(errorComponent_);
            if (!currentErrors_.empty())
            {
                errorComp->UpdateErrorDisplay(currentErrors_, currentErrorIndex_);
            }
        }
    }

    // If no errors remain, trigger auto-restoration to previous panel
    if (currentErrors_.empty() && panelLoaded_)
    {
        log_i("ErrorPanel: No errors remaining - triggering auto-restoration");
        ErrorManager::Instance().SetErrorPanelActive(false);
        
        // Auto-restore using CheckRestoration to handle active triggers
        if (panelService_)
        {
            log_i("ErrorPanel: No errors remaining - checking for active triggers before restoration");
            
            #ifdef CLARITY_DEBUG
            // In debug builds, manually trigger restoration since debug error sensor
            // state is GPIO-controlled, not error-state-controlled
            PanelManager::TriggerService().CheckRestoration();
            log_i("ErrorPanel: Auto-restoration triggered for debug build");
            #endif
            
            // In production, error triggers would naturally deactivate when errors are resolved
            return; // Exit early to prevent callback execution on replaced panel
        }
    }


    // Error panel updates are handled internally - no notification needed
}

// Static Event Callbacks
void ErrorPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    log_i("ErrorPanel::ShowPanelCompletionCallback() called - CRITICAL for UI state");

    if (!event)
    {
        log_e("ShowPanelCompletionCallback: event is null!");
        return;
    }

    auto thisInstance = static_cast<ErrorPanel *>(lv_event_get_user_data(event));
    if (thisInstance)
    {
        // Set UI state to IDLE after static panel loads so triggers can be evaluated again
        if (thisInstance->panelService_)
        {
            log_i("ErrorPanel: Setting UI state to IDLE - this unblocks main loop processing");
            thisInstance->panelService_->SetUiState(UIState::IDLE);
            log_i("ErrorPanel: UI state set to IDLE successfully");
        }
        else
        {
            log_e("ErrorPanel::ShowPanelCompletionCallback: panelService_ is null!");
        }
    }
    else
    {
        log_e("ErrorPanel::ShowPanelCompletionCallback: thisInstance is null!");
    }
}

// Legacy Action interface methods (retained for reference)
/*
Action ErrorPanel::GetShortPressAction()
{
    log_v("GetShortPressAction() called");

    // Short press cycles through each error
    return Action(
        [this]()
        {
            log_i("ErrorPanel: Short press - cycling to next error");
            if (currentErrors_.empty())
            {
                return;
            }

            // Use the ErrorComponent's built-in cycling method
            if (errorComponent_)
            {
                auto errorList = std::static_pointer_cast<ErrorComponent>(errorComponent_);
                errorList->CycleToNextError();
            }
            else
            {
                log_e("ErrorPanel: errorComponent_ is null!");
                ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ErrorPanel",
                                                     "Cannot cycle errors - errorListComponent is null");
            }
        });
}

Action ErrorPanel::GetLongPressAction()
{
    log_v("GetLongPressAction() called");

    // Long press clears all errors
    return Action(
        [this]()
        {
            log_i("ErrorPanel: Long press - clearing all errors");
            // Clear all errors from the error manager
            ErrorManager::Instance().ClearAllErrors();

            // Clear local cache
            currentErrors_.clear();
        });
}
*/


// Manager injection method
void ErrorPanel::SetManagers(IPanelService *panelService, IStyleService *styleService)
{
    log_v("SetManagers() called");

    panelService_ = panelService;
    // styleService_ is already set in constructor, but update if different instance provided
    if (styleService != styleService_)
    {
        styleService_ = styleService;
    }
}

// IActionService Interface Implementation

static void ErrorPanelShortPress(void* panelContext)
{
    log_i("ErrorPanelShortPress() called with context=%p", panelContext);
    auto* panel = static_cast<ErrorPanel*>(panelContext);
    
    if (panel)
    {
        log_t("ErrorPanel: Calling HandleShortPress() on valid panel instance");
        panel->HandleShortPress();
    }
    else
    {
        log_e("ErrorPanel: Cannot cycle errors - invalid context (null panel)");
    }
}

static void ErrorPanelLongPress(void* panelContext)
{
    log_i("ErrorPanelLongPress() called with context=%p", panelContext);
    auto* panel = static_cast<ErrorPanel*>(panelContext);
    
    if (panel)
    {
        log_t("ErrorPanel: Calling HandleLongPress() on valid panel instance");
        panel->HandleLongPress();
    }
    else
    {
        log_e("ErrorPanel: Cannot execute long press - invalid context (null panel)");
    }
}

void (*ErrorPanel::GetShortPressFunction())(void* panelContext)
{
    return ErrorPanelShortPress;
}

void (*ErrorPanel::GetLongPressFunction())(void* panelContext)
{
    return ErrorPanelLongPress;
}

void* ErrorPanel::GetPanelContext()
{
    return this;
}

void ErrorPanel::HandleShortPress()
{
    log_t("ErrorPanel::HandleShortPress() - Manual cycling to next error");
    
    if (currentErrors_.empty())
    {
        log_w("ErrorPanel::HandleShortPress() - No errors to cycle through");
        return;
    }
    
    log_t("ErrorPanel::HandleShortPress() - Current error index: %zu/%zu", 
          currentErrorIndex_, currentErrors_.size());
    
    // Always advance to next error (cycles back to 0 after last error)
    log_t("ErrorPanel::HandleShortPress() - Advancing to next error");
    AdvanceToNextError();
    
    // Short press only cycles through errors, never exits
    // User must use long press to clear and exit
}

void ErrorPanel::HandleLongPress()
{
    log_t("ErrorPanel: Long press - clearing all errors and exiting panel");
    
    // Clear all errors from the error manager
    ErrorManager::Instance().ClearAllErrors();
    
    // Set error panel as inactive
    ErrorManager::Instance().SetErrorPanelActive(false);
    
    // Use standard restoration logic for all builds
    // This will check for other active triggers and restore appropriately
    PanelManager::TriggerService().CheckRestoration();
    log_i("ErrorPanel: Triggered restoration check after clearing errors");
}

// Auto-cycling implementation
void ErrorPanel::SortErrorsBySeverity()
{
    log_v("SortErrorsBySeverity() called");
    
    // Sort errors: CRITICAL (2) first, ERROR (1) middle, WARNING (0) last
    std::sort(currentErrors_.begin(), currentErrors_.end(), 
        [](const ErrorInfo& a, const ErrorInfo& b) {
            return static_cast<int>(a.level) > static_cast<int>(b.level);
        });
    
}


void ErrorPanel::AdvanceToNextError()
{
    if (currentErrors_.empty())
    {
        log_w("AdvanceToNextError: No errors to advance through");
        return;
    }
    
    size_t oldIndex = currentErrorIndex_;
    
    // Move to next error
    currentErrorIndex_ = (currentErrorIndex_ + 1) % currentErrors_.size();
    
    log_i("AdvanceToNextError: Moving from error %zu to %zu (total: %zu)", 
          oldIndex, currentErrorIndex_, currentErrors_.size());
    
    // Update the component to display the new current error
    if (errorComponent_)
    {
        auto errorComp = std::static_pointer_cast<ErrorComponent>(errorComponent_);
        log_i("AdvanceToNextError: Updating error component display");
        errorComp->UpdateErrorDisplay(currentErrors_, currentErrorIndex_);
        log_i("AdvanceToNextError: Error component updated successfully");
    }
    else
    {
        log_e("AdvanceToNextError: Error component is null - cannot update display");
    }
    
    log_i("AdvanceToNextError: Successfully advanced to error %zu/%zu", currentErrorIndex_ + 1, currentErrors_.size());
}