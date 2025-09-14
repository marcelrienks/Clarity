#include "components/error_component.h"
#include <Arduino.h>
#include <esp32-hal-log.h>
#include <algorithm>

// Constructors and Destructors
ErrorComponent::ErrorComponent(IStyleService *styleService)
    : styleService_(styleService), errorContainer_(nullptr), errorContentArea_(nullptr), errorCountLabel_(nullptr),
      errorLevelLabel_(nullptr), errorSourceLabel_(nullptr), errorMessageLabel_(nullptr), navigationIndicator_(nullptr),
      currentErrorIndex_(0), buttonPressCount_(0)
{
    log_v("ErrorComponent() constructor called");
}

ErrorComponent::~ErrorComponent()
{
    log_v("~ErrorComponent() destructor called");
    // LVGL objects are managed by the parent screen, no manual deletion needed
}

// Core Functionality Methods
void ErrorComponent::Render(lv_obj_t *screen, const ComponentLocation &location, IDisplayProvider *display)
{
    log_v("Render() called");
    if (!screen)
    {
        log_e("ErrorComponent requires valid screen object");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ErrorComponent",
                                             "Cannot render - screen is null");
        return;
    }

    if (!display)
    {
        log_e("ErrorComponent requires display provider");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ErrorComponent",
                                             "Cannot render - display provider is null");
        return;
    }

    // Create main container using full 240x240 screen size
    errorContainer_ = lv_obj_create(screen);
    lv_obj_set_size(errorContainer_, 240, 240); // Full screen size

    // Apply location settings to the container
    lv_obj_align(errorContainer_, location.align, location.x_offset, location.y_offset);

    // Apply circular styling with 2px colored border from screen edge
    lv_obj_set_style_radius(errorContainer_, 120, 0);     // Make it circular (half of 240)
    lv_obj_set_style_border_width(errorContainer_, 2, 0); // 2px colored border from edge

    // Ensure container has transparent background for error panel's dark theme
    lv_obj_set_style_bg_opa(errorContainer_, LV_OPA_TRANSP, 0);

    // Create the internal UI structure for single error display
    CreateSingleErrorUI(errorContainer_);

    // Initial update with current errors - fetch and sort immediately
    UpdateErrorDisplay();
}

void ErrorComponent::Refresh(const Reading &reading)
{
    // For single error display, we'll update directly from ErrorManager rather than using Reading
    // This allows us to get the complete error queue information and maintain current position
    // Note: Don't call UpdateErrorDisplay() here during initialization - let the panel control the display
}

// Error-specific methods
void ErrorComponent::UpdateErrorDisplay()
{
    // Get current errors from ErrorManager
    std::vector<ErrorInfo> newErrors = ErrorManager::Instance().GetErrorQueue();

    // Sort errors by severity (CRITICAL first, WARNING last)
    std::sort(newErrors.begin(), newErrors.end(),
        [](const ErrorInfo& a, const ErrorInfo& b) {
            return static_cast<int>(a.level) > static_cast<int>(b.level);
        });

    UpdateErrorDisplay(newErrors);
}

void ErrorComponent::UpdateErrorDisplay(const std::vector<ErrorInfo> &errors)
{
    // Store current error state
    currentErrors_ = errors;

    // Ensure current index is valid
    if (currentErrorIndex_ >= currentErrors_.size())
    {
        currentErrorIndex_ = 0;
    }

    // Display the current error
    DisplayCurrentError();

    // Update container border color based on current error level
    if (!currentErrors_.empty() && currentErrorIndex_ < currentErrors_.size())
    {
        ErrorLevel currentLevel = currentErrors_[currentErrorIndex_].level;
        lv_color_t borderColor = StyleUtils::GetErrorColor(currentLevel);
        lv_obj_set_style_border_color(errorContainer_, borderColor, 0);
    }
}

void ErrorComponent::UpdateErrorDisplay(const std::vector<ErrorInfo> &errors, size_t currentIndex)
{
    // Store current error state
    currentErrors_ = errors;

    // Set the current index
    if (currentIndex < currentErrors_.size())
    {
        currentErrorIndex_ = currentIndex;
    }
    else
    {
        currentErrorIndex_ = 0;
    }

    // Display the current error
    DisplayCurrentError();

    // Update container border color based on current error level
    if (!currentErrors_.empty() && currentErrorIndex_ < currentErrors_.size())
    {
        ErrorLevel currentLevel = currentErrors_[currentErrorIndex_].level;
        lv_color_t borderColor = StyleUtils::GetErrorColor(currentLevel);
        lv_obj_set_style_border_color(errorContainer_, borderColor, 0);
    }
}

// Internal Methods
void ErrorComponent::CreateSingleErrorUI(lv_obj_t *parent)
{
    log_v("CreateSingleErrorUI() called");

    // Create error position indicator at top
    errorCountLabel_ = lv_label_create(parent);
    lv_obj_align(errorCountLabel_, LV_ALIGN_TOP_MID, 0, 8); // Near top with minimal margin
    lv_obj_set_style_text_font(errorCountLabel_, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(errorCountLabel_, lv_color_white(), 0); // Always white for dark background
    lv_label_set_text(errorCountLabel_, "1/1");

    // Create main content area using most of the 220x220 available space
    errorContentArea_ = lv_obj_create(parent);
    lv_obj_set_size(errorContentArea_, 200, 180);            // Large content area
    lv_obj_align(errorContentArea_, LV_ALIGN_CENTER, 0, 10); // Slightly down from center
    lv_obj_set_style_bg_opa(errorContentArea_, LV_OPA_0, 0); // Transparent background
    lv_obj_set_style_border_width(errorContentArea_, 0, 0);  // No border
    lv_obj_set_style_pad_all(errorContentArea_, 5, 0);       // Minimal padding

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
    lv_label_set_text(errorMessageLabel_, "Loading errors...");

    // Create navigation indicator at bottom
    navigationIndicator_ = lv_label_create(parent);
    lv_obj_align(navigationIndicator_, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_text_font(navigationIndicator_, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(navigationIndicator_, lv_color_white(), 0);
    lv_label_set_text(navigationIndicator_, "Loading...");
}

void ErrorComponent::DisplayCurrentError()
{
    log_v("DisplayCurrentError() called");
    if (currentErrors_.empty())
    {
        // Don't update display when no errors - panel should be closing
        // This prevents the brief "0/0 errors" screen from showing
        return;
    }

    if (currentErrorIndex_ >= currentErrors_.size())
    {
        currentErrorIndex_ = 0; // Reset if out of bounds
    }

    const ErrorInfo &currentError = currentErrors_[currentErrorIndex_];

    // Update position indicator
    char positionText[16];
    snprintf(positionText, sizeof(positionText), "%zu/%zu", currentErrorIndex_ + 1, currentErrors_.size());
    lv_label_set_text(errorCountLabel_, positionText);

    // Update error level with color
    const char *levelText = GetErrorLevelText(currentError.level);
    lv_label_set_text(errorLevelLabel_, levelText);
    lv_obj_set_style_text_color(errorLevelLabel_, StyleUtils::GetErrorColor(currentError.level), 0);

    // Update source
    lv_label_set_text(errorSourceLabel_, currentError.source);

    // Update message - can display full message now with wrapping
    lv_label_set_text(errorMessageLabel_, currentError.message);

    // Update navigation indicator - same for all errors
    // Short press cycles through errors, long press exits from any error
    lv_label_set_text(navigationIndicator_, "short: next, long: exit");
}

// Note: Deprecated methods CycleToNextError() and HandleCycleButtonPress() removed.
// ErrorPanel now handles all cycling logic via HandleShortPress() -> AdvanceToNextError()

// Helper Methods
const char *ErrorComponent::GetErrorLevelText(ErrorLevel level)
{
    log_v("GetErrorLevelText() called");
    switch (level)
    {
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

// Note: LVGL event callbacks removed - these were never connected to UI elements
// Error handling is now done through ErrorPanel button logic