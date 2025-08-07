#include "components/error_list_component.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors
ErrorListComponent::ErrorListComponent(IStyleService* styleService) 
    : styleService_(styleService), errorContainer_(nullptr), errorList_(nullptr), 
      errorCountLabel_(nullptr), clearButton_(nullptr)
{
    log_d("Creating ErrorListComponent");
}

ErrorListComponent::~ErrorListComponent()
{
    // LVGL objects are managed by the parent screen, no manual deletion needed
    log_d("Destroying ErrorListComponent");
}

// Core Functionality Methods
void ErrorListComponent::Render(lv_obj_t *screen, const ComponentLocation &location, IDisplayProvider* display)
{
    log_d("Rendering error list component at specified location");

    if (!display) {
        log_e("ErrorListComponent requires display provider");
        return;
    }
    
    // Create main container as the root component
    errorContainer_ = lv_obj_create(screen);
    lv_obj_set_size(errorContainer_, 220, 220); // Fit within 240x240 display
    
    // Apply location settings to the container
    lv_obj_align(errorContainer_, location.align, location.x_offset, location.y_offset);
    
    // Apply styling
    lv_obj_set_style_radius(errorContainer_, 10, 0);
    lv_obj_set_style_border_width(errorContainer_, 2, 0);
    
    // Create the internal UI structure
    CreateErrorListUI(errorContainer_);
    
    // Initial update with current errors
    UpdateErrorDisplay();
}

void ErrorListComponent::Refresh(const Reading& reading)
{
    log_d("Refreshing error list component with new error data");
    
    // For error list, we'll update directly from ErrorManager rather than using Reading
    // This allows us to get the complete error queue information
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
    log_d("Updating error display with %d errors", errors.size());
    
    // Store current error state
    currentErrors_ = errors;
    
    if (!errorCountLabel_ || !errorList_) {
        log_w("Error list UI not initialized yet");
        return;
    }
    
    // Update error count label
    char countText[32];
    snprintf(countText, sizeof(countText), "Errors: %d", currentErrors_.size());
    lv_label_set_text(errorCountLabel_, countText);
    
    // Clear existing list items
    lv_obj_clean(errorList_);
    
    // Add error entries
    for (size_t i = 0; i < currentErrors_.size(); i++) {
        CreateErrorEntry(errorList_, currentErrors_[i], i);
    }
    
    // Update container border color based on highest error level
    if (!currentErrors_.empty()) {
        ErrorLevel highestLevel = ErrorLevel::WARNING;
        for (const auto& error : currentErrors_) {
            if (!error.acknowledged) {
                if (error.level == ErrorLevel::CRITICAL) {
                    highestLevel = ErrorLevel::CRITICAL;
                    break;
                }
                if (error.level == ErrorLevel::ERROR && highestLevel == ErrorLevel::WARNING) {
                    highestLevel = ErrorLevel::ERROR;
                }
            }
        }
        
        lv_color_t borderColor = GetErrorColor(highestLevel);
        lv_obj_set_style_border_color(errorContainer_, borderColor, 0);
    }
}

// Internal Methods
void ErrorListComponent::CreateErrorListUI(lv_obj_t* parent)
{
    log_d("Creating error list UI structure");
    
    // Create error count label at top
    errorCountLabel_ = lv_label_create(parent);
    lv_obj_align(errorCountLabel_, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_text_font(errorCountLabel_, &lv_font_montserrat_14, 0);
    lv_label_set_text(errorCountLabel_, "Errors: 0");
    
    // Create scrollable error list
    errorList_ = lv_list_create(parent);
    lv_obj_set_size(errorList_, 200, 150);
    lv_obj_align(errorList_, LV_ALIGN_TOP_MID, 0, 35);
    lv_obj_set_style_radius(errorList_, 5, 0);
    
    // Create clear all button at bottom
    clearButton_ = lv_button_create(parent);
    lv_obj_set_size(clearButton_, 100, 25);
    lv_obj_align(clearButton_, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_add_event_cb(clearButton_, ErrorListComponent::ClearAllErrorsCallback, LV_EVENT_CLICKED, this);
    
    lv_obj_t* buttonLabel = lv_label_create(clearButton_);
    lv_label_set_text(buttonLabel, "Clear All");
    lv_obj_center(buttonLabel);
    lv_obj_set_style_text_font(buttonLabel, &lv_font_montserrat_10, 0);
}

void ErrorListComponent::CreateErrorEntry(lv_obj_t* parent, const ErrorInfo& error, size_t index)
{
    // Create list item button
    lv_obj_t* errorItem = lv_list_add_button(parent, nullptr, "");
    lv_obj_set_style_bg_color(errorItem, GetErrorColor(error.level), 0);
    lv_obj_set_style_bg_opa(errorItem, LV_OPA_30, 0);
    
    // Add click event for acknowledgment
    lv_obj_add_event_cb(errorItem, ErrorListComponent::ErrorAcknowledgeCallback, LV_EVENT_CLICKED, 
                       reinterpret_cast<void*>(index));
    
    // Create error text content
    char errorText[128];
    const char* levelText = GetErrorLevelText(error.level);
    
    // Format: [LEVEL] Source: Message
    snprintf(errorText, sizeof(errorText), "[%s] %s:\n%s", 
             levelText, error.source, error.message.c_str());
    
    lv_obj_t* textLabel = lv_label_create(errorItem);
    lv_label_set_text(textLabel, errorText);
    lv_obj_set_style_text_font(textLabel, &lv_font_montserrat_10, 0);
    lv_label_set_long_mode(textLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(textLabel, 170);
    
    // Add acknowledged indicator if needed
    if (error.acknowledged) {
        lv_obj_set_style_bg_opa(errorItem, LV_OPA_10, 0);
        lv_obj_set_style_text_opa(textLabel, LV_OPA_50, 0);
    }
}

// Helper Methods
lv_color_t ErrorListComponent::GetErrorColor(ErrorLevel level)
{
    switch (level) {
        case ErrorLevel::CRITICAL:
            return lv_color_hex(0xFF0000); // Red
        case ErrorLevel::ERROR:
            return lv_color_hex(0xFF8C00); // Orange
        case ErrorLevel::WARNING:
            return lv_color_hex(0xFFD700); // Yellow
        default:
            return lv_color_hex(0xFFD700); // Default to yellow
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