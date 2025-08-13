#include "panels/error_panel.h"
#include "components/error_component.h"
#include "factories/ui_factory.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include <Arduino.h>

// Constructors and Destructors
ErrorPanel::ErrorPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService), panelService_(nullptr),
      panelLoaded_(false), previousTheme_(nullptr)
{
}

ErrorPanel::~ErrorPanel()
{
    log_d("Destroying ErrorPanel...");
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

    // Restore previous theme if one was stored
    if (styleService_ && previousTheme_)
    {
        styleService_->SetTheme(previousTheme_);
    }
}

// Core Functionality Methods
/// @brief Initialize the error panel and its UI structure
void ErrorPanel::Init()
{
    log_d("Initializing ErrorPanel");
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
}

/// @brief Load the error panel UI components
void ErrorPanel::Load(std::function<void()> callbackFunction)
{
    log_i("Loading ErrorPanel with current error queue");

    // Store callback for later use
    callbackFunction_ = callbackFunction;

    // Create component directly using UIFactory
    errorComponent_ = UIFactory::createErrorComponent(styleService_);

    // Render the component
    if (!displayProvider_)
    {
        log_e("ErrorPanel load requires display provider");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ErrorPanel",
                                             "Cannot render component - display provider is null");
        return;
    }
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
        if (callbackFunction_)
        {
            callbackFunction_();
        }
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
}

/// @brief Update the error panel with current error data
void ErrorPanel::Update(std::function<void()> callbackFunction)
{
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
        if (errorComponent_)
        {
            errorComponent_->Refresh(Reading{}); // Component will fetch errors internally
        }
    }

    // If no errors remain, trigger restoration to previous panel
    if (currentErrors_.empty() && panelLoaded_)
    {
        ErrorManager::Instance().SetErrorPanelActive(false);
    }

    callbackFunction();
}

// Static Event Callbacks
void ErrorPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    if (!event)
    {
        return;
    }

    auto thisInstance = static_cast<ErrorPanel *>(lv_event_get_user_data(event));
    if (thisInstance && thisInstance->callbackFunction_)
    {
        thisInstance->callbackFunction_();
    }
}

// IInputService Interface Implementation

Action ErrorPanel::GetShortPressAction()
{
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

bool ErrorPanel::CanProcessInput() const
{
    // ErrorPanel can always process input (no animations that block input)
    return true;
}

// Manager injection method
void ErrorPanel::SetManagers(IPanelService *panelService, IStyleService *styleService)
{
    panelService_ = panelService;
    // styleService_ is already set in constructor, but update if different instance provided
    if (styleService != styleService_)
    {
        styleService_ = styleService;
    }
}