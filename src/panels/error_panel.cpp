#include "panels/error_panel.h"
#include "components/error_component.h"
#include "managers/style_manager.h"
#include "managers/panel_manager.h"
#include "utilities/logging.h"
#include "definitions/constants.h"
#include <Arduino.h>
#include <algorithm>

// ========== Constructors and Destructor ==========

/**
 * @brief Constructs error panel with required service dependencies
 * @param gpio GPIO provider for hardware interaction
 * @param display Display provider for screen management
 * @param styleService Style service for theme management
 *
 * Creates error panel with stack-allocated error component for memory efficiency.
 * Initializes component state and sets error panel as active in ErrorManager
 * for proper coordination with the trigger system.
 */
ErrorPanel::ErrorPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleManager *styleService)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService), panelService_(nullptr),
      errorComponent_(styleService), componentInitialized_(false), panelLoaded_(false), previousTheme_(), currentErrorIndex_(0)
{
    log_v("ErrorPanel constructor called");
}

/**
 * @brief Destructor cleans up error panel resources and resets error state
 *
 * Safely deletes LVGL screen objects and resets ErrorManager error panel
 * active flag. Allows trigger system to resume normal operation after
 * error panel cleanup. Stack-allocated component is automatically destroyed.
 */
ErrorPanel::~ErrorPanel()
{
    log_v("~ErrorPanel() destructor called");

    if (screen_)
    {
        lv_obj_delete(screen_);
    }

    // Component is now stack-allocated and will be automatically destroyed

    // Reset error panel active flag when panel is destroyed
    ErrorManager::Instance().SetErrorPanelActive(false);

    // Don't restore theme here - let the trigger system manage themes
    // The CheckRestoration() method will apply the correct theme based on active triggers
}

// ========== Public Interface Methods ==========

/**
 * @brief Initializes error panel UI structure and loads current errors
 *
 * Creates LVGL screen, applies dark theme for error visibility, initializes
 * stack-allocated error component, and loads current error queue. Sets up
 * button action handlers for error navigation and exit functionality.
 */
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

/**
 * @brief Loads error panel UI components and applies error theme
 *
 * Renders error component on screen, fetches current error queue, and applies
 * ERROR theme for optimal visibility. Sets up screen loading callback and
 * loads the LVGL screen. Critical for displaying system errors to the user
 * in automotive applications where error visibility is essential.
 */
void ErrorPanel::Load()
{
    log_v("Load() called");


    // Component is now stack-allocated and initialized in constructor
    componentInitialized_ = true;

    if (!displayProvider_)
    {
        log_e("ErrorPanel load requires display provider");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ErrorPanel",
                                             "Cannot render component - display provider is null");
        return;
    }

    // Render the component
    errorComponent_.Render(screen_, centerLocation_, displayProvider_);

    // Get current errors and refresh component
    std::vector<ErrorInfo> currentErrors = ErrorManager::Instance().GetErrorQueue();
    errorComponent_.Refresh(Reading{}); // Component will fetch errors internally

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

/**
 * @brief Updates error panel with current error data and manages auto-restoration
 *
 * Monitors ErrorManager for error queue changes, updates display when errors
 * change, and triggers auto-restoration when no errors remain. Handles error
 * sorting by severity and maintains current error index for navigation.
 * Essential for real-time error monitoring in automotive systems.
 */
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
        if (componentInitialized_)
        {
            // Direct access to concrete component (no casting needed)
            if (!currentErrors_.empty())
            {
                errorComponent_.UpdateErrorDisplay(currentErrors_, currentErrorIndex_);
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
            PanelManager::Instance().CheckRestoration();
            log_i("ErrorPanel: Auto-restoration triggered for debug build");
            #endif
            
            // In production, error triggers would naturally deactivate when errors are resolved
            return; // Exit early to prevent callback execution on replaced panel
        }
    }


    // Error panel updates are handled internally - no notification needed
}

/**
 * @brief Static callback function for LVGL screen load completion
 * @param event LVGL event containing panel instance data
 *
 * Critical callback that registers button action handlers when error panel
 * screen loading is complete. Ensures UI is ready before enabling user
 * interactions. Essential for proper error panel navigation functionality.
 */
void ErrorPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    log_i("ErrorPanel::ShowPanelCompletionCallback() called - CRITICAL for UI state");

    if (!event)
    {
        log_e("ShowPanelCompletionCallback: event is null!");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ErrorPanel",
                                            "ShowPanelCompletionCallback received null event");
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
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ErrorPanel",
                                                "PanelService is null in completion callback");
        }
    }
    else
    {
        log_e("ErrorPanel::ShowPanelCompletionCallback: thisInstance is null!");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ErrorPanel",
                                            "Instance is null in completion callback");
    }
}

/**
 * @brief Injects manager dependencies for panel operation
 * @param panelService Panel service for navigation control
 * @param styleService Style service for theme management
 *
 * Dependency injection method that provides panel with required manager
 * interfaces. Called during panel creation to establish service dependencies
 * needed for proper panel operation and lifecycle management.
 */
void ErrorPanel::SetManagers(IPanelManager *panelService, IStyleManager *styleService)
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
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ErrorPanel",
                                            "Cannot cycle errors - invalid context");
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
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ErrorPanel",
                                            "Cannot execute long press - invalid context");
    }
}

/**
 * @brief Gets the short press callback function for this panel
 * @return Function pointer to the short press handler
 *
 * Returns the static callback function that will be invoked when a short
 * button press is detected while this panel is active. The returned function
 * takes a panel context pointer and handles error cycling through the list.
 */
// REMOVED: 
/**
 * @brief Handles short button press events for error cycling
 *
 * Processes short button press events by advancing to the next error in the
 * current error list. Cycles through all errors sequentially, wrapping back
 * to the first error after reaching the last one. Does not clear errors or
 * exit the panel - only provides navigation through the error list.
 */
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

/**
 * @brief Handles long button press events for error clearing and panel exit
 *
 * Processes long button press events by clearing all errors from the error
 * manager and exiting the error panel. Marks the error panel as inactive and
 * triggers the restoration logic to return to the appropriate previous panel
 * or default state. This provides the primary mechanism for dismissing errors.
 */
void ErrorPanel::HandleLongPress()
{
    log_t("ErrorPanel: Long press - clearing all errors and exiting panel");
    
    // Clear all errors from the error manager
    ErrorManager::Instance().ClearAllErrors();
    
    // Set error panel as inactive
    ErrorManager::Instance().SetErrorPanelActive(false);
    
    // Use standard restoration logic for all builds
    // This will check for other active triggers and restore appropriately
    PanelManager::Instance().CheckRestoration();
    log_i("ErrorPanel: Triggered restoration check after clearing errors");
}

// Auto-cycling implementation
/**
 * @brief Sorts the current error list by severity level
 *
 * Arranges errors in priority order with CRITICAL errors first, ERROR level
 * errors in the middle, and WARNING level errors last. This ensures that the
 * most important errors are displayed first when cycling through the error list.
 * Uses a lambda comparator to sort by the numeric severity level values.
 */
void ErrorPanel::SortErrorsBySeverity()
{
    log_v("SortErrorsBySeverity() called");
    
    // Sort errors: CRITICAL (2) first, ERROR (1) middle, WARNING (0) last
    std::sort(currentErrors_.begin(), currentErrors_.end(), 
        [](const ErrorInfo& a, const ErrorInfo& b) {
            return static_cast<int>(a.level) > static_cast<int>(b.level);
        });
    
}


/**
 * @brief Advances to the next error in the current error list
 *
 * Increments the current error index to move to the next error, wrapping back
 * to index 0 when reaching the end of the list. Updates the error component
 * display to show the new current error. Handles empty error lists gracefully
 * and ensures the component is properly initialized before updating the display.
 */
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
    if (componentInitialized_)
    {
        // Direct access to concrete component
        log_i("AdvanceToNextError: Updating error component display");
        errorComponent_.UpdateErrorDisplay(currentErrors_, currentErrorIndex_);
        log_i("AdvanceToNextError: Error component updated successfully");
    }
    else
    {
        log_e("AdvanceToNextError: Error component is null - cannot update display");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ErrorPanel",
                                            "Error component is null - cannot update display");
    }
    
    log_i("AdvanceToNextError: Successfully advanced to error %zu/%zu", currentErrorIndex_ + 1, currentErrors_.size());
}