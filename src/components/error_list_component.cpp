#include "components/error_list_component.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors
ErrorListComponent::ErrorListComponent(IStyleService* styleService) 
    : styleService_(styleService), errorContainer_(nullptr), errorContentArea_(nullptr), 
      errorCountLabel_(nullptr), errorLevelLabel_(nullptr), errorSourceLabel_(nullptr),
      errorMessageLabel_(nullptr), navigationIndicator_(nullptr), currentErrorIndex_(0),
      buttonPressCount_(0)
{
    log_d("Creating ErrorListComponent for single error display");
}

ErrorListComponent::~ErrorListComponent()
{
    // LVGL objects are managed by the parent screen, no manual deletion needed
    log_d("Destroying ErrorListComponent single error display");
}

// Core Functionality Methods
void ErrorListComponent::Render(lv_obj_t *screen, const ComponentLocation &location, IDisplayProvider* display)
{
    log_d("Rendering error list component at specified location");

    if (!display) {
        log_e("ErrorListComponent requires display provider");
        return;
    }
    
    // Create main container using full 240x240 screen size
    errorContainer_ = lv_obj_create(screen);
    lv_obj_set_size(errorContainer_, 240, 240); // Full screen size
    
    // Apply location settings to the container
    lv_obj_align(errorContainer_, location.align, location.x_offset, location.y_offset);
    
    // Apply circular styling with 2px colored border from screen edge
    lv_obj_set_style_radius(errorContainer_, 120, 0); // Make it circular (half of 240)
    lv_obj_set_style_border_width(errorContainer_, 2, 0); // 2px colored border from edge
    
    // Ensure container has transparent background for error panel's dark theme
    lv_obj_set_style_bg_opa(errorContainer_, LV_OPA_TRANSP, 0);
    
    // Create the internal UI structure for single error display
    CreateSingleErrorUI(errorContainer_);
    
    // Initial update with current errors
    UpdateErrorDisplay();
}

void ErrorListComponent::Refresh(const Reading& reading)
{
    log_d("Refreshing single error display with new error data");
    
    // For single error display, we'll update directly from ErrorManager rather than using Reading
    // This allows us to get the complete error queue information and maintain current position
    UpdateErrorDisplay();
}

// Error-specific methods
void ErrorListComponent::UpdateErrorDisplay()
{
    // Get current errors from ErrorManager
    std::vector<ErrorInfo> newErrors = ErrorManager::Instance().GetErrorQueue();
    UpdateErrorDisplay(newErrors);
}

void ErrorListComponent::UpdateErrorDisplay(const std::vector<ErrorInfo>& errors)
{
    log_d("Updating single error display with %d errors", errors.size());
    
    // Store current error state
    currentErrors_ = errors;
    
    // Reset button press counter when new errors arrive
    buttonPressCount_ = 0;
    
    // Ensure current index is valid
    if (currentErrorIndex_ >= currentErrors_.size()) {
        currentErrorIndex_ = 0;
    }
    
    // Display the current error
    DisplayCurrentError();
    
    // Update container border color based on current error level
    if (!currentErrors_.empty() && currentErrorIndex_ < currentErrors_.size()) {
        ErrorLevel currentLevel = currentErrors_[currentErrorIndex_].level;
        lv_color_t borderColor = GetErrorColor(currentLevel);
        lv_obj_set_style_border_color(errorContainer_, borderColor, 0);
    }
}

// Internal Methods
void ErrorListComponent::CreateSingleErrorUI(lv_obj_t* parent)
{
    log_d("Creating single error UI structure optimized for full screen display");
    
    // Create error position indicator at top
    errorCountLabel_ = lv_label_create(parent);
    lv_obj_align(errorCountLabel_, LV_ALIGN_TOP_MID, 0, 8); // Near top with minimal margin
    lv_obj_set_style_text_font(errorCountLabel_, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(errorCountLabel_, lv_color_white(), 0); // Always white for dark background
    lv_label_set_text(errorCountLabel_, "1/1");
    
    // Create main content area using most of the 220x220 available space
    errorContentArea_ = lv_obj_create(parent);
    lv_obj_set_size(errorContentArea_, 200, 180); // Large content area
    lv_obj_align(errorContentArea_, LV_ALIGN_CENTER, 0, 10); // Slightly down from center
    lv_obj_set_style_bg_opa(errorContentArea_, LV_OPA_0, 0); // Transparent background
    lv_obj_set_style_border_width(errorContentArea_, 0, 0); // No border
    lv_obj_set_style_pad_all(errorContentArea_, 5, 0); // Minimal padding
    
    // Create large error level indicator
    errorLevelLabel_ = lv_label_create(errorContentArea_);
    lv_obj_align(errorLevelLabel_, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_text_font(errorLevelLabel_, &lv_font_montserrat_24, 0); // Large font
    lv_obj_set_style_text_color(errorLevelLabel_, lv_color_white(), 0);
    lv_label_set_text(errorLevelLabel_, "ERROR");
    
    // Create error source label
    errorSourceLabel_ = lv_label_create(errorContentArea_);
    lv_obj_align(errorSourceLabel_, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_set_style_text_font(errorSourceLabel_, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(errorSourceLabel_, lv_color_white(), 0);
    lv_label_set_text(errorSourceLabel_, "System");
    
    // Create error message display with maximum available space
    errorMessageLabel_ = lv_label_create(errorContentArea_);
    lv_obj_set_size(errorMessageLabel_, 180, 100); // Large message area
    lv_obj_align(errorMessageLabel_, LV_ALIGN_CENTER, 0, 25);
    lv_obj_set_style_text_font(errorMessageLabel_, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(errorMessageLabel_, lv_color_white(), 0);
    lv_obj_set_style_text_align(errorMessageLabel_, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(errorMessageLabel_, LV_LABEL_LONG_WRAP); // Multi-line text
    lv_label_set_text(errorMessageLabel_, "No errors");
    
    // Create navigation indicator at bottom
    navigationIndicator_ = lv_label_create(parent);
    lv_obj_align(navigationIndicator_, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_text_font(navigationIndicator_, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(navigationIndicator_, lv_color_white(), 0);
    lv_label_set_text(navigationIndicator_, "Loading...");
}

void ErrorListComponent::DisplayCurrentError()
{
    if (currentErrors_.empty()) {
        // No errors to display
        lv_label_set_text(errorCountLabel_, "0/0");
        lv_label_set_text(errorLevelLabel_, "NO ERRORS");
        lv_label_set_text(errorSourceLabel_, "");
        lv_label_set_text(errorMessageLabel_, "All systems operational");
        lv_label_set_text(navigationIndicator_, "");
        return;
    }
    
    if (currentErrorIndex_ >= currentErrors_.size()) {
        currentErrorIndex_ = 0; // Reset if out of bounds
    }
    
    const ErrorInfo& currentError = currentErrors_[currentErrorIndex_];
    
    // Update position indicator
    char positionText[16];
    snprintf(positionText, sizeof(positionText), "%zu/%zu", 
             currentErrorIndex_ + 1, currentErrors_.size());
    lv_label_set_text(errorCountLabel_, positionText);
    
    // Update error level with color
    const char* levelText = GetErrorLevelText(currentError.level);
    lv_label_set_text(errorLevelLabel_, levelText);
    lv_obj_set_style_text_color(errorLevelLabel_, GetErrorColor(currentError.level), 0);
    
    // Update source
    lv_label_set_text(errorSourceLabel_, currentError.source);
    
    // Update message - can display full message now with wrapping
    lv_label_set_text(errorMessageLabel_, currentError.message.c_str());
    
    // Update navigation indicator based on button press count
    if (buttonPressCount_ >= currentErrors_.size()) {
        lv_label_set_text(navigationIndicator_, "Press to exit");
    } else if (currentErrors_.size() > 1) {
        lv_label_set_text(navigationIndicator_, "Press for next");
    } else {
        lv_label_set_text(navigationIndicator_, "Press to exit");
    }
    
    log_d("Displaying error %zu/%zu: [%s] %s", 
          currentErrorIndex_ + 1, currentErrors_.size(), 
          levelText, currentError.source);
}

void ErrorListComponent::CycleToNextError()
{
    if (currentErrors_.empty()) return;
    
    // Increment button press count
    buttonPressCount_++;
    
    // If we've shown all errors, clear and trigger restore
    if (buttonPressCount_ > currentErrors_.size()) {
        log_d("All errors shown - clearing errors and triggering restore");
        ErrorManager::Instance().ClearAllErrors();
        ErrorManager::Instance().SetErrorPanelActive(false);
        // The trigger system will handle restoring to the appropriate panel
        return;
    }
    
    // Move to next error (only if we haven't shown all yet)
    if (buttonPressCount_ <= currentErrors_.size()) {
        currentErrorIndex_ = (currentErrorIndex_ + 1) % currentErrors_.size();
        DisplayCurrentError();
        
        // Update border color for new current error
        lv_color_t borderColor = GetErrorColor(currentErrors_[currentErrorIndex_].level);
        lv_obj_set_style_border_color(errorContainer_, borderColor, 0);
    }
    
    log_d("Button press %zu/%zu, showing error %zu/%zu", 
          buttonPressCount_, currentErrors_.size() + 1,
          currentErrorIndex_ + 1, currentErrors_.size());
}

void ErrorListComponent::HandleCycleButtonPress()
{
    // Public interface method to be called from GPIO button handling
    log_d("GPIO button press detected - cycling to next error");
    CycleToNextError();
}


// Helper Methods
lv_color_t ErrorListComponent::GetErrorColor(ErrorLevel level)
{
    switch (level) {
        case ErrorLevel::CRITICAL:
            return lv_color_hex(0xFF0000); // Red
        case ErrorLevel::ERROR:
            return lv_color_hex(0xFFFF00); // Yellow
        case ErrorLevel::WARNING:
            return lv_color_hex(0xFFFFFF); // White
        default:
            return lv_color_hex(0xFFFFFF); // Default to white
    }
}

const char* ErrorListComponent::GetErrorLevelText(ErrorLevel level)
{
    switch (level) {
        case ErrorLevel::CRITICAL:
            return "CRIT";
        case ErrorLevel::ERROR:
            return "ERR";
        case ErrorLevel::WARNING:
            return "WARN";
        default:
            return "UNKN";
    }
}

// Static Event Callbacks
void ErrorListComponent::ErrorAcknowledgeCallback(lv_event_t *event)
{
    size_t errorIndex = reinterpret_cast<size_t>(lv_event_get_user_data(event));
    log_d("Acknowledging error at index %d", errorIndex);
    
    ErrorManager::Instance().AcknowledgeError(errorIndex);
}

void ErrorListComponent::ClearAllErrorsCallback(lv_event_t *event)
{
    ErrorListComponent* component = static_cast<ErrorListComponent*>(lv_event_get_user_data(event));
    log_d("Clearing all errors");
    
    ErrorManager::Instance().ClearAllErrors();
}