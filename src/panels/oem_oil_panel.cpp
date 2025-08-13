#include "panels/oem_oil_panel.h"
#include "factories/ui_factory.h"
#include "managers/action_manager.h"
#include "managers/error_manager.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"

// Constructors and Destructors

OemOilPanel::OemOilPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService), panelService_(nullptr),
      oemOilPressureSensor_(std::make_shared<OilPressureSensor>(gpio)),
      oemOilTemperatureSensor_(std::make_shared<OilTemperatureSensor>(gpio)), currentOilPressureValue_(-1),
      currentOilTemperatureValue_(-1), lastTheme_("")
{
    // Initialize LVGL animation structures to prevent undefined behavior
    lv_anim_init(&pressureAnimation_);
    lv_anim_init(&temperatureAnimation_);
}

OemOilPanel::~OemOilPanel()
{
    log_d("Destroying OemOilPanel...");

    // Stop any running animations before destroying the panel
    if (isPressureAnimationRunning_)
    {
        lv_anim_delete(&pressureAnimation_, nullptr);
        isPressureAnimationRunning_ = false;
        log_d("Deleted pressure animation");
    }

    if (isTemperatureAnimationRunning_)
    {
        lv_anim_delete(&temperatureAnimation_, nullptr);
        isTemperatureAnimationRunning_ = false;
        log_d("Deleted temperature animation");
    }

    // Delete all animations that might reference this instance to prevent callback corruption
    lv_anim_delete(this, nullptr);
    log_d("Deleted all animations for this instance");

    if (screen_)
    {
        lv_obj_delete(screen_);
        screen_ = nullptr;
    }

    if (oemOilPressureComponent_)
    {
        oemOilPressureComponent_.reset();
    }

    if (oemOilTemperatureComponent_)
    {
        oemOilTemperatureComponent_.reset();
    }

    if (oemOilPressureSensor_)
    {
        oemOilPressureSensor_.reset();
    }

    if (oemOilTemperatureSensor_)
    {
        oemOilTemperatureSensor_.reset();
    }

    log_d("OemOilPanel destruction complete");
}

// Core Functionality Methods

/// @brief Initialize the panel for showing Oil related information
/// Creates screen and initializes sensors with sentinel values
void OemOilPanel::Init()
{
    // Initializing OEM oil panel with sensors and display components

    screen_ = displayProvider_->CreateScreen();

    // Apply current theme immediately after screen creation
    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
    }

    oemOilPressureSensor_->Init();
    currentOilPressureValue_ = -1; // Sentinel value to ensure first update

    oemOilTemperatureSensor_->Init();
    currentOilTemperatureValue_ = -1; // Sentinel value to ensure first update
}

/// @brief Load the panel with component rendering and screen display
/// @param callbackFunction to be called when the panel load is completed
void OemOilPanel::Load(std::function<void()> callbackFunction)
{
    // Loading OEM oil panel with pressure and temperature gauges
    callbackFunction_ = callbackFunction;

    // Create components for both pressure and temperature
    if (styleService_ && styleService_->IsInitialized())
    {
        // Creating pressure and temperature components
        oemOilPressureComponent_ = UIFactory::createOemOilPressureComponent(styleService_);
        oemOilTemperatureComponent_ = UIFactory::createOemOilTemperatureComponent(styleService_);
    }
    else
    {
        log_w("StyleService not properly initialized, skipping component creation");
    }

    // Create location parameters with rotational start points for scales
    ComponentLocation pressureLocation(210);   // rotation starting at 210 degrees
    ComponentLocation temperatureLocation(30); // rotation starting at 30 degrees

    // Render both components
    if (oemOilPressureComponent_)
    {
        oemOilPressureComponent_->Render(screen_, pressureLocation, displayProvider_);
        // Pressure component rendered successfully
    }
    if (oemOilTemperatureComponent_)
    {
        oemOilTemperatureComponent_->Render(screen_, temperatureLocation, displayProvider_);
        // Temperature component rendered successfully
    }
    lv_obj_add_event_cb(screen_, OemOilPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);

    log_v("loading...");

    lv_screen_load(screen_);

    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
        // Update lastTheme_ to current theme to sync with theme detection in update()
        lastTheme_ = String(styleService_->GetCurrentTheme());
    }
}

/// @brief Update the reading on the screen
void OemOilPanel::Update(std::function<void()> callbackFunction)
{

    callbackFunction_ = callbackFunction;

    // Always force component refresh when theme has changed (like panel restoration)
    // This ensures icons and pivot styling update regardless of needle value changes
    auto *styleService = styleService_;
    const char *currentTheme = styleService ? styleService->GetCurrentTheme() : "";
    if (lastTheme_.isEmpty() || !lastTheme_.equals(currentTheme))
    {
        forceComponentRefresh_ = true;
        // Theme changed, forcing component refresh
        lastTheme_ = String(currentTheme);

        // Apply the new theme to the screen background when theme changes
        if (styleService_ && screen_)
        {
            styleService_->ApplyThemeToScreen(screen_);
        }
    }
    else
    {
        forceComponentRefresh_ = false;
    }

    // Update sensor readings and components
    OemOilPanel::UpdateOilPressure();
    OemOilPanel::UpdateOilTemperature();

    // Reset the force refresh flag after updates
    forceComponentRefresh_ = false;

    // If no animations were started, call completion callback immediately
    if (!isPressureAnimationRunning_ && !isTemperatureAnimationRunning_)
    {
        callbackFunction_();
    }
}

// Private Methods

/// @brief Update the oil pressure reading on the screen
void OemOilPanel::UpdateOilPressure()
{

    // Skip update if pressure animation is already running
    if (isPressureAnimationRunning_)
    {
        return;
    }

    // Safety check for sensor availability
    if (!oemOilPressureSensor_)
    {
        log_w("Pressure sensor is null, skipping update");
        return;
    }

    // Use delta-based updates for better performance
    auto sensorValue = std::get<int32_t>(oemOilPressureSensor_->GetReading());
    auto value = MapPressureValue(sensorValue);

    // Handle forced refresh (theme changes) even when values unchanged
    if (value == currentOilPressureValue_ && forceComponentRefresh_)
    {
        // Pressure value unchanged but forced refresh required - updating colors only
        oemOilPressureComponent_->Refresh(Reading{value});
        return; // No animation needed since value didn't change
    }

    // Skip update only if value is exactly the same as last update AND this is not a forced update
    if (value == currentOilPressureValue_ && !forceComponentRefresh_)
    {
        return;
    }

    log_i("Updating pressure from %d to %d", currentOilPressureValue_, value);
    oemOilPressureComponent_->Refresh(Reading{value});

    // Setup animation
    lv_anim_init(&pressureAnimation_);
    lv_anim_set_duration(&pressureAnimation_, _animation_duration);
    lv_anim_set_repeat_count(&pressureAnimation_, 0);
    lv_anim_set_playback_duration(&pressureAnimation_, 0);
    lv_anim_set_values(&pressureAnimation_, currentOilPressureValue_, value);
    lv_anim_set_var(&pressureAnimation_, this);
    lv_anim_set_user_data(&pressureAnimation_, (void *)static_cast<uintptr_t>(OilSensorTypes::Pressure));
    lv_anim_set_exec_cb(&pressureAnimation_, OemOilPanel::ExecutePressureAnimationCallback);
    lv_anim_set_completed_cb(&pressureAnimation_, OemOilPanel::UpdatePanelCompletionCallback);

    isPressureAnimationRunning_ = true;
    // Start pressure gauge animation
    lv_anim_start(&pressureAnimation_);
}

/// @brief Update the oil temperature reading on the screen
void OemOilPanel::UpdateOilTemperature()
{

    // Skip update if temperature animation is already running
    if (isTemperatureAnimationRunning_)
    {
        return;
    }

    // Safety check for sensor availability
    if (!oemOilTemperatureSensor_)
    {
        log_w("Temperature sensor is null, skipping update");
        return;
    }

    // Use delta-based updates for better performance
    auto sensorValue = std::get<int32_t>(oemOilTemperatureSensor_->GetReading());
    auto value = MapTemperatureValue(sensorValue);

    // Handle forced refresh (theme changes) even when values unchanged
    if (value == currentOilTemperatureValue_ && forceComponentRefresh_)
    {
        // Temperature value unchanged but forced refresh required - updating colors only
        oemOilTemperatureComponent_->Refresh(Reading{value});
        return; // No animation needed since value didn't change
    }

    // Skip update only if value is exactly the same as last update AND this is not a forced update
    if (value == currentOilTemperatureValue_ && !forceComponentRefresh_)
    {
        return;
    }

    log_i("Updating temperature from %d to %d", currentOilTemperatureValue_, value);
    oemOilTemperatureComponent_->Refresh(Reading{value});

    // Setup animation
    lv_anim_init(&temperatureAnimation_);
    lv_anim_set_duration(&temperatureAnimation_, _animation_duration);
    lv_anim_set_repeat_count(&temperatureAnimation_, 0);
    lv_anim_set_playback_duration(&temperatureAnimation_, 0);
    lv_anim_set_values(&temperatureAnimation_, currentOilTemperatureValue_, value);
    lv_anim_set_var(&temperatureAnimation_, this);
    lv_anim_set_user_data(&temperatureAnimation_, (void *)static_cast<uintptr_t>(OilSensorTypes::Temperature));
    lv_anim_set_exec_cb(&temperatureAnimation_, OemOilPanel::ExecuteTemperatureAnimationCallback);
    lv_anim_set_completed_cb(&temperatureAnimation_, OemOilPanel::UpdatePanelCompletionCallback);

    isTemperatureAnimationRunning_ = true;
    // Start temperature gauge animation
    lv_anim_start(&temperatureAnimation_);
}

Action OemOilPanel::GetShortPressAction()
{
    // Short press: No action for oil panel
    log_d("OemOilPanel: Short press action requested - returning NoAction");
    return Action(nullptr);
}

Action OemOilPanel::GetLongPressAction()
{
    // Long press: Switch to CONFIG panel
    log_i("OemOilPanel: Long press - switching to CONFIG panel");

    // Return an action that directly calls PanelService interface
    if (panelService_)
    {
        return Action(
            [this]()
            {
                panelService_->CreateAndLoadPanel(
                    PanelNames::CONFIG,
                    []()
                    {
                        // Panel switch callback handled by service
                    },
                    true); // Mark as trigger-driven to avoid overwriting restoration panel
            });
    }

    log_w("OemOilPanel: PanelService not available, returning no action");
    return Action(nullptr);
}

/// @brief Manager injection method to prevent circular references
/// @param panelService the panel manager instance
/// @param styleService  the style manager instance
void OemOilPanel::SetManagers(IPanelService *panelService, IStyleService *styleService)
{
    panelService_ = panelService;
    // styleService_ is already set in constructor, but update if different instance provided
    if (styleService != styleService_)
    {
        styleService_ = styleService;
    }
    // Managers injected successfully
}

/// @brief Set preference service and apply sensor update rate from preferences
/// @param preferenceService The preference service to use for configuration
void OemOilPanel::SetPreferenceService(IPreferenceService *preferenceService)
{
    preferenceService_ = preferenceService;

    // Apply updateRate and units from preferences to sensors if preference service is available
    if (preferenceService_)
    {
        auto config = preferenceService_->GetConfig();
        log_i("Applying sensor configuration from preferences");
        log_i("Update rate: %d ms", config.updateRate);
        log_i("Pressure unit: %s", config.pressureUnit.c_str());
        log_i("Temperature unit: %s", config.tempUnit.c_str());

        // Apply to pressure sensor - we know the concrete types since we created them
        auto pressureSensor = std::static_pointer_cast<OilPressureSensor>(oemOilPressureSensor_);
        pressureSensor->SetUpdateRate(config.updateRate);
        pressureSensor->SetTargetUnit(config.pressureUnit);

        // Apply to temperature sensor - we know the concrete types since we created them
        auto temperatureSensor = std::static_pointer_cast<OilTemperatureSensor>(oemOilTemperatureSensor_);
        temperatureSensor->SetUpdateRate(config.updateRate);
        temperatureSensor->SetTargetUnit(config.tempUnit);
    }
}

bool OemOilPanel::CanProcessInput() const
{
    // OemOilPanel can always process input (no animations that block input)
    return true;
}

// Static Callback Methods

/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void OemOilPanel::ShowPanelCompletionCallback(lv_event_t *event)
{

    if (!event)
    {
        return;
    }

    auto thisInstance = static_cast<OemOilPanel *>(lv_event_get_user_data(event));
    if (!thisInstance)
    {
        return;
    }

    thisInstance->callbackFunction_();
}

/// @brief Callback when animation has completed. aka update complete
/// @param animation the object that was animated
void OemOilPanel::UpdatePanelCompletionCallback(lv_anim_t *animation)
{

    if (animation == nullptr || animation->var == nullptr)
    {
        log_w("Animation or animation->var is null, skipping completion callback");
        return;
    }

    auto thisInstance = static_cast<OemOilPanel *>(animation->var);
    auto sensorType = static_cast<OilSensorTypes>(reinterpret_cast<uintptr_t>(animation->user_data));

    // Determine which animation has completed and update the corresponding value
    if (sensorType == OilSensorTypes::Pressure)
    {
        thisInstance->currentOilPressureValue_ = animation->current_value;
        thisInstance->isPressureAnimationRunning_ = false;
    }
    else if (sensorType == OilSensorTypes::Temperature)
    {
        thisInstance->currentOilTemperatureValue_ = animation->current_value;
        thisInstance->isTemperatureAnimationRunning_ = false;
    }

    // Only call the callback function if both animations are not running
    if (!thisInstance->isPressureAnimationRunning_ && !thisInstance->isTemperatureAnimationRunning_)
    {
        if (thisInstance->callbackFunction_)
        {
            thisInstance->callbackFunction_();
        }
    }
}

/// @brief callback used by the animation to set the values smoothly until ultimate value is reached
/// @param target the object being animated
/// @param value the next value in a sequence to create a smooth transition
void OemOilPanel::ExecutePressureAnimationCallback(void *target, int32_t value)
{

    lv_anim_t *animation = lv_anim_get(target, ExecutePressureAnimationCallback); // get the animation
    if (animation == nullptr || animation->var == nullptr)
    {
        log_w("Animation or animation->var is null, skipping callback");
        return;
    }

    auto thisInstance =
        static_cast<OemOilPanel *>(animation->var); // use the animation to get the var which is this instance
    if (thisInstance->oemOilPressureComponent_)
    {
        thisInstance->oemOilPressureComponent_.get()->SetValue(value);
    }
}

/// @brief callback used by the animation to set the values smoothly until ultimate value is reached
/// @param target the object being animated
/// @param value the next value in a sequence to create a smooth transition
void OemOilPanel::ExecuteTemperatureAnimationCallback(void *target, int32_t value)
{

    lv_anim_t *animation = lv_anim_get(target, ExecuteTemperatureAnimationCallback); // get the animation
    if (animation == nullptr || animation->var == nullptr)
    {
        log_w("Animation or animation->var is null, skipping callback");
        return;
    }

    auto thisInstance =
        static_cast<OemOilPanel *>(animation->var); // use the animation to get the var which is this instance
    if (thisInstance->oemOilTemperatureComponent_)
    {
        thisInstance->oemOilTemperatureComponent_.get()->SetValue(value);
    }
}

// Value mapping methods

/// @brief Map oil pressure sensor value to display scale
/// @param sensorValue Raw sensor value (1-10 Bar)
/// @return Mapped value for display (0-60, representing pressure range)
int32_t OemOilPanel::MapPressureValue(int32_t sensorValue)
{
    // Sensor now returns values in configured units, map to display scale (0-60)
    int32_t mappedValue;

    if (preferenceService_)
    {
        const Configs &config = preferenceService_->GetConfig();
        if (config.pressureUnit == "PSI")
        {
            // Sensor returns 0-145 PSI, map to 0-60 display
            // Clamp to valid range (14.5-145 PSI equivalent to 1-10 Bar)
            if (sensorValue < 15)
                sensorValue = 15;
            if (sensorValue > 145)
                sensorValue = 145;
            // Map useful PSI range (15-145) to display scale (0-60)
            mappedValue = ((sensorValue - 15) * 60) / 130;
        }
        else if (config.pressureUnit == "kPa")
        {
            // Sensor returns 0-1000 kPa, map to 0-60 display
            // Clamp to valid range (100-1000 kPa equivalent to 1-10 Bar)
            if (sensorValue < 100)
                sensorValue = 100;
            if (sensorValue > 1000)
                sensorValue = 1000;
            // Map useful kPa range (100-1000) to display scale (0-60)
            mappedValue = ((sensorValue - 100) * 60) / 900;
        }
        else
        {
            // Bar: sensor returns 0-10 Bar, map to 0-60 display
            // Clamp to valid range (1-10 Bar)
            if (sensorValue < 1)
                sensorValue = 1;
            if (sensorValue > 10)
                sensorValue = 10;
            // Map Bar range (1-10) to display scale (0-60)
            mappedValue = ((sensorValue - 1) * 60) / 9;
        }
    }
    else
    {
        // Fallback to Bar mapping
        if (sensorValue < 1)
            sensorValue = 1;
        if (sensorValue > 10)
            sensorValue = 10;
        mappedValue = ((sensorValue - 1) * 60) / 9;
    }

    return mappedValue;
}

/// @brief Map oil temperature sensor value to display scale
/// @param sensorValue Temperature value in configured units
/// @return Mapped value for display (0-120 scale)
int32_t OemOilPanel::MapTemperatureValue(int32_t sensorValue)
{
    // Sensor now returns values in configured units, map to display scale (0-120)
    int32_t mappedValue;

    if (preferenceService_)
    {
        const Configs &config = preferenceService_->GetConfig();
        if (config.tempUnit == "F")
        {
            // Sensor returns 32-248°F, map to 0-120 display scale
            // Clamp to valid range (32-248°F equivalent to 0-120°C)
            if (sensorValue < 32)
                sensorValue = 32;
            if (sensorValue > 248)
                sensorValue = 248;
            // Map Fahrenheit range (32-248) to display scale (0-120)
            mappedValue = ((sensorValue - 32) * 120) / 216;
        }
        else
        {
            // Celsius: sensor returns 0-120°C, direct mapping to display scale
            // Clamp to valid range (0-120°C)
            if (sensorValue < 0)
                sensorValue = 0;
            if (sensorValue > 120)
                sensorValue = 120;
            mappedValue = sensorValue;
        }
    }
    else
    {
        // Fallback to Celsius mapping
        if (sensorValue < 0)
            sensorValue = 0;
        if (sensorValue > 120)
            sensorValue = 120;
        mappedValue = sensorValue;
    }

    return mappedValue;
}

// IPanel override to provide input service via composition
// IInputService* OemOilPanel::GetInputService()
// {
//     return inputHandler_.get();
// }