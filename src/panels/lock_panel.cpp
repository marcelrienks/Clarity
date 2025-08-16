#include "panels/lock_panel.h"
#include "factories/component_factory.h"
#include "interfaces/i_component_factory.h"
#include "managers/style_manager.h"
#include "managers/error_manager.h"
#include <esp32-hal-log.h>
#include <variant>

// Constructors and Destructors
LockPanel::LockPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService,
                     IComponentFactory* componentFactory)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService),
      componentFactory_(componentFactory ? componentFactory : &ComponentFactory::Instance()),
      lockSensor_(std::make_shared<LockSensor>(gpio))
{
    log_v("LockPanel constructor called");
    // Component will be created during load() method
}

LockPanel::~LockPanel()
{
    log_v("~LockPanel() destructor called");
    if (screen_)
    {
        lv_obj_delete(screen_);
    }

    if (lockComponent_)
    {
        lockComponent_.reset();
    }

    if (lockSensor_)
    {
        lockSensor_.reset();
    }
}

// Core Functionality Methods

/// @brief Initialize the lock panel and its components
void LockPanel::Init()
{
    log_v("Init() called");

    if (!displayProvider_ || !gpioProvider_)
    {
        log_e("LockPanel requires display and gpio providers");
        ErrorManager::Instance().ReportCriticalError("LockPanel",
                                                     "Missing required providers - display or gpio provider is null");
        return;
    }

    screen_ = displayProvider_->CreateScreen();

    // Apply current theme immediately after screen creation
    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
    }
    centerLocation_ = ComponentLocation(LV_ALIGN_CENTER, 0, 0);

    lockSensor_->Init();
    isLockEngaged_ = false;
    
    log_i("LockPanel initialization completed");
}

/// @brief Load the lock panel UI components
void LockPanel::Load(std::function<void()> callbackFunction)
{
    log_v("Load() called");
    
    callbackFunction_ = callbackFunction;

    // Set BUSY at start of load
    if (panelService_)
    {
        panelService_->SetUiState(UIState::BUSY);
    }

    if (!componentFactory_)
    {
        log_e("LockPanel requires component factory");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "LockPanel", "ComponentFactory is null");
        return;
    }

    // Create component using injected factory
    lockComponent_ = componentFactory_->CreateLockComponent(styleService_);
    if (!lockComponent_)
    {
        log_e("Failed to create lock component");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "LockPanel", "Component creation failed");
        return;
    }

    // Create the lock component centered on screen, and immediately refresh it with the current lock status
    lockComponent_->Render(screen_, centerLocation_, displayProvider_);
    lockComponent_->Refresh(Reading{isLockEngaged_});
    lv_obj_add_event_cb(screen_, LockPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);

    lv_screen_load(screen_);

    // Always apply current theme to the screen when loading (ensures theme is current)
    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
    }
    
    log_i("LockPanel loaded successfully");
}

/// @brief Update the lock panel with current sensor data
void LockPanel::Update(std::function<void()> callbackFunction)
{
    log_v("Update() called");
    
    // Set BUSY at start of update
    if (panelService_)
    {
        panelService_->SetUiState(UIState::BUSY);
    }

    // Get current lock state from sensor
    if (lockSensor_)
    {
        auto sensorReading = lockSensor_->GetReading();
        bool currentLockState = std::get<bool>(sensorReading);
        
        // Update display if lock state has changed
        if (currentLockState != isLockEngaged_)
        {
            log_d("Lock state changed from %s to %s", 
                  isLockEngaged_ ? "engaged" : "disengaged",
                  currentLockState ? "engaged" : "disengaged");
            isLockEngaged_ = currentLockState;
            if (lockComponent_)
            {
                lockComponent_->Refresh(Reading{isLockEngaged_});
            }
        }
    }

    // Set IDLE when update completes
    if (panelService_)
    {
        panelService_->SetUiState(UIState::IDLE);
    }
    
    callbackFunction();
}

// Static Methods

/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void LockPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    log_v("ShowPanelCompletionCallback() called");
    
    if (!event)
        return;

    auto thisInstance = static_cast<LockPanel *>(lv_event_get_user_data(event));
    if (!thisInstance)
        return;
    
    // Set IDLE when load completes
    if (thisInstance->panelService_)
    {
        thisInstance->panelService_->SetUiState(UIState::IDLE);
    }
    
    thisInstance->callbackFunction_();
}

// Manager injection method
void LockPanel::SetManagers(IPanelService *panelService, IStyleService *styleService)
{
    log_v("SetManagers() called");
    panelService_ = panelService;

    // Update styleService if different instance provided
    if (styleService != styleService_)
    {
        styleService_ = styleService;
    }
}