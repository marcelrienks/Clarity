#include "panels/oem_oil_panel.h"
#include "utilities/lv_tools.h"
#include "managers/style_manager.h"

// Constructors and Destructors

OemOilPanel::OemOilPanel(IComponentFactory* componentFactory)
    : componentFactory_(componentFactory),
      oemOilPressureSensor_(std::make_shared<OilPressureSensor>()),
      oemOilTemperatureSensor_(std::make_shared<OilTemperatureSensor>()) 
{
    // Components will be created during load() method using the component factory
}

OemOilPanel::~OemOilPanel()
{
    log_d("Destroying OemOilPanel...");
    
    // Stop any running animations before destroying the panel
    if (isPressureAnimationRunning_) {
        lv_anim_delete(&pressureAnimation_, nullptr);
        isPressureAnimationRunning_ = false;
        log_d("Deleted pressure animation");
    }
    
    if (isTemperatureAnimationRunning_) {
        lv_anim_delete(&temperatureAnimation_, nullptr);
        isTemperatureAnimationRunning_ = false;
        log_d("Deleted temperature animation");
    }

    // Delete all animations that might reference this instance to prevent callback corruption
    lv_anim_delete(this, nullptr);
    log_d("Deleted all animations for this instance");

    if (screen_) {
        lv_obj_delete(screen_);
        screen_ = nullptr;
    }

    if (oemOilPressureComponent_) {
        oemOilPressureComponent_.reset();
    }

    if (oemOilTemperatureComponent_) {
        oemOilTemperatureComponent_.reset();
    }

    if (oemOilPressureSensor_) {
        oemOilPressureSensor_.reset();
    }

    if (oemOilTemperatureSensor_) {
        oemOilTemperatureSensor_.reset();
    }
    
    log_d("OemOilPanel destruction complete");
}

// Core Functionality Methods

/// @brief Initialize the panel for showing Oil related information
/// Creates screen and initializes sensors with sentinel values
void OemOilPanel::init(IGpioProvider* gpio, IDisplayProvider* display)
{
    log_d("Initializing OEM oil panel with sensors and display components");

    screen_ = display->createScreen();

    oemOilPressureSensor_->init();
    currentOilPressureValue_ = -1; // Sentinel value to ensure first update

    oemOilTemperatureSensor_->init();
    currentOilTemperatureValue_ = -1; // Sentinel value to ensure first update
}

/// @brief Load the panel with component rendering and screen display
/// @param callbackFunction to be called when the panel load is completed
void OemOilPanel::load(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display)
{
    log_d("Loading OEM oil panel with pressure and temperature gauges");
    callbackFunction_ = callbackFunction;

    // Create components using the injected component factory
    // The factory now has all required dependencies (style service and display provider) injected
    oemOilPressureComponent_ = componentFactory_->createComponent("oem_oil_pressure");
    oemOilTemperatureComponent_ = componentFactory_->createComponent("oem_oil_temperature");

    // Create location parameters with rotational start points for scales
    ComponentLocation pressureLocation(210); // rotation starting at 210 degrees
    ComponentLocation temperatureLocation(30); // rotation starting at 30 degrees
    
    oemOilPressureComponent_->render(screen_, pressureLocation, display);
    oemOilTemperatureComponent_->render(screen_, temperatureLocation, display);
    lv_obj_add_event_cb(screen_, OemOilPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);

    log_v("loading...");
    
    lv_screen_load(screen_);
}

/// @brief Update the reading on the screen
void OemOilPanel::update(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display)
{
    log_d("Updating OEM oil panel readings and checking for changes");

    callbackFunction_ = callbackFunction;

    // Always force component refresh when theme has changed (like panel restoration)
    // This ensures icons and pivot styling update regardless of needle value changes
    // During transition: access global StyleManager instance
    extern std::unique_ptr<StyleManager> g_styleManager;
    const char* currentTheme = g_styleManager->THEME;
    if (lastTheme_.isEmpty() || !lastTheme_.equals(currentTheme)) {
        forceComponentRefresh_ = true;
        log_d("Theme changed to %s, forcing component refresh", currentTheme);
        lastTheme_ = String(currentTheme);
    } else {
        forceComponentRefresh_ = false;
    }

    log_v("updating...");
    OemOilPanel::UpdateOilPressure();
    OemOilPanel::UpdateOilTemperature();
    
    // Reset the force refresh flag after updates
    forceComponentRefresh_ = false;
    
    // If no animations were started, call completion callback immediately
    if (!isPressureAnimationRunning_ && !isTemperatureAnimationRunning_) {
        log_d("No animations running, calling completion callback immediately");
        callbackFunction_();
    }
}

// Private Methods

/// @brief Update the oil pressure reading on the screen
void OemOilPanel::UpdateOilPressure()
{
    log_d("...");

    // Skip update if pressure animation is already running
    if (isPressureAnimationRunning_) {
        log_d("Pressure animation running, skipping update");
        return;
    }

    // Use delta-based updates for better performance
    auto sensorValue = std::get<int32_t>(oemOilPressureSensor_->GetReading());
    auto value = MapPressureValue(sensorValue);
    
    // Handle forced refresh (theme changes) even when values unchanged
    if (value == currentOilPressureValue_ && forceComponentRefresh_) {
        log_d("Pressure value unchanged (%d) but forced refresh required - updating colors only", value);
        oemOilPressureComponent_->refresh(Reading{value});
        return; // No animation needed since value didn't change
    }
    
    // Skip update only if value is exactly the same as last update AND this is not a forced update
    if (value == currentOilPressureValue_ && !forceComponentRefresh_) {
        return;
    }
    
    log_i("Updating pressure from %d to %d", currentOilPressureValue_, value);
    oemOilPressureComponent_->refresh(Reading{value});

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
    log_d("animating...");
    lv_anim_start(&pressureAnimation_);
}

/// @brief Update the oil temperature reading on the screen
void OemOilPanel::UpdateOilTemperature()
{
    log_d("...");

    // Skip update if temperature animation is already running
    if (isTemperatureAnimationRunning_) {
        log_d("Temperature animation running, skipping update");
        return;
    }

    // Use delta-based updates for better performance
    auto sensorValue = std::get<int32_t>(oemOilTemperatureSensor_->GetReading());
    auto value = MapTemperatureValue(sensorValue);
    
    // Handle forced refresh (theme changes) even when values unchanged
    if (value == currentOilTemperatureValue_ && forceComponentRefresh_) {
        log_d("Temperature value unchanged (%d) but forced refresh required - updating colors only", value);
        oemOilTemperatureComponent_->refresh(Reading{value});
        return; // No animation needed since value didn't change
    }
    
    // Skip update only if value is exactly the same as last update AND this is not a forced update
    if (value == currentOilTemperatureValue_ && !forceComponentRefresh_) {
        return;
    }
    
    log_i("Updating temperature from %d to %d", currentOilTemperatureValue_, value);
    oemOilTemperatureComponent_->refresh(Reading{value});

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
    log_d("animating...");
    lv_anim_start(&temperatureAnimation_);
}

// Static Callback Methods

/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void OemOilPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    log_d("...");

    if (!event) {
        return;
    }

    auto thisInstance = static_cast<OemOilPanel *>(lv_event_get_user_data(event));
    if (!thisInstance) {
        return;
    }
    
    thisInstance->callbackFunction_();
}

/// @brief Callback when animation has completed. aka update complete
/// @param animation the object that was animated
void OemOilPanel::UpdatePanelCompletionCallback(lv_anim_t *animation)
{
    log_d("...");

    if (animation == nullptr || animation->var == nullptr) {
        log_w("Animation or animation->var is null, skipping completion callback");
        return;
    }

    auto thisInstance = static_cast<OemOilPanel *>(animation->var);
    auto sensorType = static_cast<OilSensorTypes>(reinterpret_cast<uintptr_t>(animation->user_data));

    // Determine which animation has completed and update the corresponding value
    if (sensorType == OilSensorTypes::Pressure) {
        thisInstance->currentOilPressureValue_ = animation->current_value;
        thisInstance->isPressureAnimationRunning_ = false;
    } else if (sensorType == OilSensorTypes::Temperature) {
        thisInstance->currentOilTemperatureValue_ = animation->current_value;
        thisInstance->isTemperatureAnimationRunning_ = false;
    }

    // Only call the callback function if both animations are not running
    if (!thisInstance->isPressureAnimationRunning_ && !thisInstance->isTemperatureAnimationRunning_) {
        if (thisInstance->callbackFunction_) {
            thisInstance->callbackFunction_();
        }
    }
}

/// @brief callback used by the animation to set the values smoothly until ultimate value is reached
/// @param target the object being animated
/// @param value the next value in a sequence to create a smooth transition
void OemOilPanel::ExecutePressureAnimationCallback(void *target, int32_t value)
{
    log_d("...");

    lv_anim_t *animation = lv_anim_get(target, ExecutePressureAnimationCallback); // get the animation
    if (animation == nullptr || animation->var == nullptr) {
        log_w("Animation or animation->var is null, skipping callback");
        return;
    }
    
    auto thisInstance = static_cast<OemOilPanel *>(animation->var);                    // use the animation to get the var which is this instance
    if (thisInstance->oemOilPressureComponent_) {
        thisInstance->oemOilPressureComponent_.get()->SetValue(value);
    }
}

/// @brief callback used by the animation to set the values smoothly until ultimate value is reached
/// @param target the object being animated
/// @param value the next value in a sequence to create a smooth transition
void OemOilPanel::ExecuteTemperatureAnimationCallback(void *target, int32_t value)
{
    log_d("...");

    lv_anim_t *animation = lv_anim_get(target, ExecuteTemperatureAnimationCallback); // get the animation
    if (animation == nullptr || animation->var == nullptr) {
        log_w("Animation or animation->var is null, skipping callback");
        return;
    }
    
    auto thisInstance = static_cast<OemOilPanel *>(animation->var);                       // use the animation to get the var which is this instance
    if (thisInstance->oemOilTemperatureComponent_) {
        thisInstance->oemOilTemperatureComponent_.get()->SetValue(value);
    }
}

// Value mapping methods

/// @brief Map oil pressure sensor value to display scale
/// @param sensorValue Raw sensor value (1-10 Bar)
/// @return Mapped value for display (0-60, representing 0.0-6.0 Bar x10)
int32_t OemOilPanel::MapPressureValue(int32_t sensorValue)
{
    // Clamp sensor value to valid range (1-10 Bar)
    if (sensorValue < 1) sensorValue = 1;
    if (sensorValue > 10) sensorValue = 10;
    
    // Map 1-10 Bar to 0-60 display units
    // Formula: (sensorValue - 1) * 60 / 9
    // This maps: 1 Bar -> 0, 10 Bar -> 60
    // Display represents 0.0-6.0 Bar with 0.1 precision (x10 multiplier)
    return ((sensorValue - 1) * 60) / 9;
}

/// @brief Map oil temperature sensor value to display scale
/// @param sensorValue Raw sensor value (0-120°C)
/// @return Mapped value for display
int32_t OemOilPanel::MapTemperatureValue(int32_t sensorValue)
{
    // Temperature mapping is direct 1:1 as the display range matches sensor range
    // Clamp to valid range (0-120°C)
    if (sensorValue < 0) sensorValue = 0;
    if (sensorValue > 120) sensorValue = 120;
    
    return sensorValue;
}