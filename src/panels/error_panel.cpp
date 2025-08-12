#include "panels/error_panel.h"
#include "factories/ui_factory.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "components/error_list_component.h"
#include <Arduino.h>

// Constructors and Destructors
ErrorPanel::ErrorPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService) 
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService), panelService_(nullptr),
      panelLoaded_(false), previousTheme_(nullptr)
{
    // Creating ErrorPanel
    // Component will be created during load() method
}

ErrorPanel::~ErrorPanel()
{
    if (screen_) {
        lv_obj_delete(screen_);
    }

    if (errorListComponent_)
    {
        errorListComponent_.reset();
    }
    
    // Reset error panel active flag when panel is destroyed
    ErrorManager::Instance().SetErrorPanelActive(false);
    
    // Restore previous theme if one was stored
    if (styleService_ && previousTheme_) {
        log_d("Restoring previous theme: %s", previousTheme_);
        styleService_->SetTheme(previousTheme_);
    }
}

// Core Functionality Methods
/// @brief Initialize the error panel and its UI structure
void ErrorPanel::Init()
{
    log_d("Initializing error panel");

    if (!displayProvider_ || !gpioProvider_) {
        log_e("ErrorPanel requires display and gpio providers");
        return;
    }

    screen_ = displayProvider_->CreateScreen();
    
    // Store current theme before switching to ERROR theme (but don't apply it yet)
    if (styleService_) {
        previousTheme_ = styleService_->GetCurrentTheme();
        // Storing previous theme
        // Don't set theme yet - do it after screen is loaded
    }
    
    centerLocation_ = ComponentLocation(LV_ALIGN_CENTER, 0, 0);
    
    // Set error panel as active in ErrorManager
    ErrorManager::Instance().SetErrorPanelActive(true);
}

/// @brief Load the error panel UI components
void ErrorPanel::Load(std::function<void()> callbackFunction)
{
    // Loading error panel with current errors

    // Store callback for later use
    callbackFunction_ = callbackFunction;

    // Create component directly using UIFactory
    errorListComponent_ = UIFactory::createErrorListComponent(styleService_);

    // Render the component
    if (!displayProvider_) {
        log_e("ErrorPanel load requires display provider");
        return;
    }
    errorListComponent_->Render(screen_, centerLocation_, displayProvider_);
    
    // Get current errors and refresh component
    std::vector<ErrorInfo> currentErrors = ErrorManager::Instance().GetErrorQueue();
    errorListComponent_->Refresh(Reading{}); // Component will fetch errors internally
    
    lv_obj_add_event_cb(screen_, ErrorPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);

    log_v("loading error panel...");
    
    // Add safety check before screen load
    if (!screen_) {
        log_e("Screen is null, cannot load error panel");
        if (callbackFunction_) {
            callbackFunction_();
        }
        return;
    }
    
    // About to call lv_screen_load
    
    // Add more safety checks
    if (!screen_) {
        log_e("screen_ is null before lv_screen_load");
        if (callbackFunction_) callbackFunction_();
        return;
    }
    
    // Check if screen is a valid LVGL object
    if (!lv_obj_is_valid(screen_)) {
        log_e("screen_ is not a valid LVGL object");
        if (callbackFunction_) callbackFunction_();
        return;
    }
    
    // Screen is valid, calling lv_screen_load
    
    // Add a small delay to ensure any previous screen operations are complete
    delay(10);
    
    lv_screen_load(screen_);
    // lv_screen_load completed successfully
    
    // Ensure ERROR theme is applied when panel is loaded
    if (styleService_) {
        // Applying ERROR theme to screen
        styleService_->SetTheme(Themes::ERROR);
        styleService_->ApplyThemeToScreen(screen_);
        // ERROR theme applied successfully
    }
    
    // Setting panelLoaded flag to true
    panelLoaded_ = true;
    // ErrorPanel Load() method completed successfully
}

/// @brief Update the error panel with current error data
void ErrorPanel::Update(std::function<void()> callbackFunction)
{
    // Get current errors from ErrorManager
    std::vector<ErrorInfo> newErrors = ErrorManager::Instance().GetErrorQueue();
    
    // Check if error list has changed
    bool errorsChanged = false;
    if (newErrors.size() != currentErrors_.size()) {
        errorsChanged = true;
    } else {
        // Compare timestamps to detect changes
        for (size_t i = 0; i < newErrors.size() && !errorsChanged; i++) {
            if (newErrors[i].timestamp != currentErrors_[i].timestamp ||
                newErrors[i].acknowledged != currentErrors_[i].acknowledged) {
                errorsChanged = true;
            }
        }
    }
    
    // Update display if errors have changed
    if (errorsChanged) {
        // Error list changed - updating display
        currentErrors_ = newErrors;
        if (errorListComponent_) {
            errorListComponent_->Refresh(Reading{}); // Component will fetch errors internally
        }
    }
    
    // If no errors remain, trigger restoration to previous panel
    if (currentErrors_.empty() && panelLoaded_) {
        log_d("All errors cleared - marking error panel for restoration");
        ErrorManager::Instance().SetErrorPanelActive(false);
    }
    
    callbackFunction();
}

// Static Event Callbacks
void ErrorPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    // Error panel loaded successfully
    
    if (!event) {
        return;
    }

    auto thisInstance = static_cast<ErrorPanel *>(lv_event_get_user_data(event));
    if (thisInstance && thisInstance->callbackFunction_) {
        thisInstance->callbackFunction_();
    }
}

// IInputService Interface Implementation

Action ErrorPanel::GetShortPressAction()
{
    log_i("ErrorPanel: GetShortPressAction() called - creating short press action");
    
    // Short press cycles through each error
    return Action([this]() {
        log_i("ErrorPanel: SHORT PRESS ACTION EXECUTED!");
        
        if (currentErrors_.empty()) {
            log_w("ErrorPanel: No errors to cycle through");
            return;
        }
        
        log_i("ErrorPanel: Short press - cycling to next error (current errors: %zu)", currentErrors_.size());
        
        // Use the ErrorListComponent's built-in cycling method
        if (errorListComponent_) {
            // Static cast to ErrorListComponent (safe since we created it)
            auto errorList = std::static_pointer_cast<ErrorListComponent>(errorListComponent_);
            log_i("ErrorPanel: About to call CycleToNextError()");
            errorList->CycleToNextError();
            log_i("ErrorPanel: CycleToNextError() completed");
        } else {
            log_e("ErrorPanel: errorListComponent_ is null!");
        }
    });
}

Action ErrorPanel::GetLongPressAction()
{
    // Long press clears all errors
    return Action([this]() {
        log_i("ErrorPanel: Long press - clearing all errors");
        
        // Clear all errors from the error manager
        ErrorManager::Instance().ClearAllErrors();
        
        // Clear local cache
        currentErrors_.clear();
        
        // Update the display
        if (errorListComponent_) {
            // Error list cleared, component will update on next cycle
        }
    });
}

bool ErrorPanel::CanProcessInput() const
{
    // ErrorPanel can always process input (no animations that block input)
    return true;
}

// Manager injection method
void ErrorPanel::SetManagers(IPanelService* panelService, IStyleService* styleService)
{
    panelService_ = panelService;
    // styleService_ is already set in constructor, but update if different instance provided
    if (styleService != styleService_) {
        styleService_ = styleService;
    }
    // Managers injected successfully
}