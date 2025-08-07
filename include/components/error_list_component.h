#pragma once

#include "interfaces/i_component.h"
#include "interfaces/i_style_service.h"
#include "managers/error_manager.h"
#include "utilities/types.h"

#include <lvgl.h>
#include <vector>

/**
 * @class ErrorListComponent
 * @brief Error display and management UI component
 * 
 * @details This component displays pending application errors in a scrollable list
 * format. It provides visual indicators based on error severity and handles user
 * interaction for acknowledging errors.
 * 
 * @view_role Renders error list with theme-aware styling and severity indicators
 * @ui_elements Scrollable list container with error entries, count label, and clear button
 * @positioning Supports all ComponentLocation alignment options
 * 
 * @data_source Designed to work with ErrorManager for error queue updates
 * @visual_states Color-coded severity levels (red/orange/yellow) with auto-dismiss indicators
 * @interaction_support Click-to-acknowledge individual errors, clear all functionality
 * 
 * @context This component displays system errors and manages user acknowledgment.
 * It's designed to be used by ErrorPanel as part of the error handling workflow.
 */
class ErrorListComponent : public IComponent
{
public:
    // Constructors and Destructors
    explicit ErrorListComponent(IStyleService *styleService);
    virtual ~ErrorListComponent();

    // Core Functionality Methods
    void Render(lv_obj_t *screen, const ComponentLocation& location, IDisplayProvider *display) override;
    void Refresh(const Reading& reading) override;

    // Error-specific methods
    void UpdateErrorDisplay();
    void UpdateErrorDisplay(const std::vector<ErrorInfo>& errors);

protected:
    // UI Event Handlers
    static void ErrorAcknowledgeCallback(lv_event_t *event);
    static void ClearAllErrorsCallback(lv_event_t *event);

private:
    // Internal Methods
    void CreateErrorListUI(lv_obj_t* parent);
    void CreateErrorEntry(lv_obj_t* parent, const ErrorInfo& error, size_t index);
    lv_color_t GetErrorColor(ErrorLevel level);
    const char* GetErrorLevelText(ErrorLevel level);

    // Protected Data Members
    IStyleService *styleService_;
    lv_obj_t *errorContainer_;           // Main container for error list
    lv_obj_t *errorList_;                // Scrollable list of errors
    lv_obj_t *errorCountLabel_;          // Header showing error count
    lv_obj_t *clearButton_;              // Clear all errors button
    std::vector<ErrorInfo> currentErrors_; // Cache of current error state
};