#include "panels/error_panel.h"
#include "factories/ui_factory.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include <Arduino.h>

// Constructors and Destructors
ErrorPanel::ErrorPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService) 
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService),
      panelLoaded_(false), previousTheme_(nullptr)
{
    log_d("Creating ErrorPanel");
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
    
    // Store current theme before switching to ERROR theme
    if (styleService_) {
        previousTheme_ = styleService_->GetCurrentTheme();
        log_d("Storing previous theme: %s", previousTheme_);
        styleService_->SetTheme(Themes::ERROR);
        styleService_->ApplyThemeToScreen(screen_);
    }
    
    centerLocation_ = ComponentLocation(LV_ALIGN_CENTER, 0, 0);
    
    // Set error panel as active in ErrorManager
    ErrorManager::Instance().SetErrorPanelActive(true);
}

/// @brief Load the error panel UI components
void ErrorPanel::Load(std::function<void()> callbackFunction)
{
    log_d("Loading error panel with current errors");

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
    lv_screen_load(screen_);
    
    // Ensure ERROR theme is applied when panel is loaded
    if (styleService_) {
        styleService_->SetTheme(Themes::ERROR);
        styleService_->ApplyThemeToScreen(screen_);
    }
    
    panelLoaded_ = true;
    callbackFunction();
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
        log_d("Error list changed - updating display");
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
    log_d("Error panel loaded successfully");
    // Panel load completion handled automatically
}

// IInputService Interface Implementation

Action ErrorPanel::GetShortPressAction()
{
    // Short press cycles through each error
    return Action([this]() {
        if (currentErrors_.empty()) {
            log_d("ErrorPanel: No errors to cycle through");
            return;
        }
        
        // Move to next error (wrap around)
        currentErrorIndex_ = (currentErrorIndex_ + 1) % currentErrors_.size();
        
        log_i("ErrorPanel: Short press - showing error %d of %d", 
              currentErrorIndex_ + 1, currentErrors_.size());
        
        // Update the error list component to highlight current error
        if (errorListComponent_) {
            log_d("Currently showing error %d: %s", currentErrorIndex_ + 1, 
                  currentErrors_[currentErrorIndex_].message.c_str());
        }
    }, "ErrorPanel cycle through errors");
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
        currentErrorIndex_ = 0;
        
        // Update the display
        if (errorListComponent_) {
            log_d("Error list cleared, component will update on next cycle");
        }
    }, "ErrorPanel clear all errors");
}

bool ErrorPanel::CanProcessInput() const
{
    // ErrorPanel can always process input (no animations that block input)
    return true;
}