#include "panels/oem_oil_panel.h"
#include "factories/component_factory.h"
#include "interfaces/i_component_factory.h"
#include "managers/error_manager.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "utilities/constants.h"
#include "utilities/logging.h"

// Constructors and Destructors

OemOilPanel::OemOilPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService,
                         IComponentFactory *componentFactory)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService), panelService_(nullptr),
      componentFactory_(componentFactory ? componentFactory : &ComponentFactory::Instance()),
      oemOilPressureSensor_(std::make_shared<OilPressureSensor>(gpio)),
      oemOilTemperatureSensor_(std::make_shared<OilTemperatureSensor>(gpio)), currentOilPressureValue_(-1),
      currentOilTemperatureValue_(-1), lastTheme_("")
{
    log_v("OemOilPanel constructor called");
    // Initialize LVGL animation structures to prevent undefined behavior
    lv_anim_init(&pressureAnimation_);
    lv_anim_init(&temperatureAnimation_);
}

OemOilPanel::~OemOilPanel()
{
    log_v("~OemOilPanel() destructor called");

    // Stop any running animations before destroying the panel
    if (animationState_ != AnimationState::IDLE)
    {
        lv_anim_delete(&pressureAnimation_, nullptr);
        lv_anim_delete(&temperatureAnimation_, nullptr);
        animationState_ = AnimationState::IDLE;
    }

    // Delete all animations that might reference this instance
    lv_anim_delete(this, nullptr);

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
}

// Core Functionality Methods

/// @brief Initialize the panel for showing Oil related information
/// Creates screen and initializes sensors with sentinel values
void OemOilPanel::Init()
{
    log_v("Init() called");

    if (!displayProvider_)
    {
        log_e("OemOilPanel requires display provider");
        ErrorManager::Instance().ReportCriticalError("OemOilPanel", "Missing required display provider");
        return;
    }

    if (!oemOilPressureSensor_ || !oemOilTemperatureSensor_)
    {
        log_e("OemOilPanel requires pressure and temperature sensors");
        ErrorManager::Instance().ReportCriticalError("OemOilPanel", "Missing required sensors");
        return;
    }

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

    // Cache sensor references to avoid repeated static_pointer_cast operations
    cachedPressureSensor_ = std::static_pointer_cast<OilPressureSensor>(oemOilPressureSensor_);
    cachedTemperatureSensor_ = std::static_pointer_cast<OilTemperatureSensor>(oemOilTemperatureSensor_);
    
    log_i("OemOilPanel initialization completed");
}

/// @brief Load the panel with component rendering and screen display
/// @param callbackFunction to be called when the panel load is completed
void OemOilPanel::Load()
{
    log_v("Load() called");


    // Create components for both pressure and temperature
    if (!styleService_ || !styleService_->IsInitialized())
    {
        log_w("StyleService not properly initialized, skipping component creation");
        return;
    }

    if (!componentFactory_)
    {
        log_e("ComponentFactory is required for component creation");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "OemOilPanel", "ComponentFactory is null");
        return;
    }

    // Creating pressure and temperature components
    oemOilPressureComponent_ = componentFactory_->CreateOilPressureComponent(styleService_);
    oemOilTemperatureComponent_ = componentFactory_->CreateOilTemperatureComponent(styleService_);

    if (!oemOilPressureComponent_ || !oemOilTemperatureComponent_)
    {
        log_e("Failed to create required components");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "OemOilPanel", "Component creation failed");
        return;
    }

    // Create location parameters with rotational start points for scales
    ComponentLocation pressureLocation(210);   // rotation starting at 210 degrees
    ComponentLocation temperatureLocation(30); // rotation starting at 30 degrees

    // Render both components
    oemOilPressureComponent_->Render(screen_, pressureLocation, displayProvider_);
    oemOilTemperatureComponent_->Render(screen_, temperatureLocation, displayProvider_);
    
    // Initialize needle positions to match current values (prevents animation jumps)
    oemOilPressureComponent_->SetValue(currentOilPressureValue_);
    oemOilTemperatureComponent_->SetValue(currentOilTemperatureValue_);
    
    // Initialize needle colors based on initial values
    oemOilPressureComponent_->Refresh(Reading{currentOilPressureValue_});
    oemOilTemperatureComponent_->Refresh(Reading{currentOilTemperatureValue_});
    
    
    // Log heap status before critical operation
    
    // Validate screen object pointer and basic properties
    if (screen_) {
        // Check if LVGL object is valid by accessing safe properties
        lv_obj_t* parent = lv_obj_get_parent(screen_);
        
        // Check object coordinates (safe to read)
        lv_coord_t x = lv_obj_get_x(screen_);
        lv_coord_t y = lv_obj_get_y(screen_);
        
        // Check object size
        lv_coord_t w = lv_obj_get_width(screen_);
        lv_coord_t h = lv_obj_get_height(screen_);
    } else {
        log_e("  screen_ is NULL!");
    }
    
    // Validate component pointers
    
    // Check if 'this' pointer is still valid
    
    lv_obj_add_event_cb(screen_, OemOilPanel::ShowPanelCompletionCallback, LV_EVENT_SCREEN_LOADED, this);
    
    // Final validation before the critical call
    
    // Memory pattern check - write and verify a pattern
    static const uint32_t MEMORY_PATTERN = 0xDEADBEEF;
    uint32_t test_pattern = MEMORY_PATTERN;
    if (test_pattern != MEMORY_PATTERN) {
        log_e("Pattern changed from 0x%08X to 0x%08X!", MEMORY_PATTERN, test_pattern);
    }

    lv_screen_load(screen_);
    
    // Post-load validation

    if (styleService_)
    {
        styleService_->ApplyThemeToScreen(screen_);
        // Update lastTheme_ to current theme to sync with theme detection in update()
        lastTheme_ = String(styleService_->GetCurrentTheme().c_str());
    }
    
    log_t("OemOilPanel loaded successfully");
}

/// @brief Update the reading on the screen
void OemOilPanel::Update()
{
    // Check for theme changes and apply immediately
    const char *currentTheme = styleService_ ? styleService_->GetCurrentTheme().c_str() : "";
    bool themeChanged = lastTheme_.isEmpty() || !lastTheme_.equals(currentTheme);
    if (themeChanged)
    {
        lastTheme_ = String(currentTheme);
        if (styleService_ && screen_)
        {
            styleService_->ApplyThemeToScreen(screen_);
        }
    }

    // Apply sensor settings periodically (every ~100 updates) instead of every cycle
    // This reduces overhead while still catching preference changes reasonably quickly
    static uint16_t settingsUpdateCounter = 0;
    if (++settingsUpdateCounter % 100 == 0)
    {
        ApplyCurrentSensorSettings();
    }

    // Update sensor readings and components (pass theme change flag)
    UpdateOilPressure(themeChanged);
    UpdateOilTemperature(themeChanged);

    // If no animations were started, updates complete immediately
    if (animationState_ == AnimationState::IDLE)
    {
        if (panelService_)
        {
            panelService_->SetUiState(UIState::IDLE);
        }
    }
}

// Private Methods

/// @brief Update the oil pressure reading on the screen
void OemOilPanel::UpdateOilPressure(bool forceRefresh)
{
    // Skip if pressure animation is running unless this is a forced refresh
    if (!forceRefresh && (animationState_ == AnimationState::PRESSURE_RUNNING || animationState_ == AnimationState::BOTH_RUNNING))
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

    // Handle forced refresh (theme changes) or skip if unchanged
    if (value == currentOilPressureValue_)
    {
        if (forceRefresh)
        {
            oemOilPressureComponent_->Refresh(Reading{value});
        }
        return; // No animation needed since value didn't change
    }

    log_i("Updating pressure from %d to %d", currentOilPressureValue_, value);
    oemOilPressureComponent_->Refresh(Reading{value});

    // Determine animation start value
    // If current value is -1 (initial state) or outside scale bounds, start from appropriate boundary
    int32_t animationStartValue = currentOilPressureValue_;
    const int32_t scaleMin = 0;  // Pressure scale minimum
    const int32_t scaleMax = 60; // Pressure scale maximum
    
    if (currentOilPressureValue_ == -1)
    {
        // Initial load - determine start position based on target value
        if (value < scaleMin)
        {
            animationStartValue = scaleMin; // Start from minimum if value is below scale
        }
        else if (value > scaleMax)
        {
            animationStartValue = scaleMax; // Start from maximum if value is above scale
        }
        else
        {
            // Value is within scale bounds
            // For pressure, always start from minimum (0) for initial animation
            // This represents the engine starting up and building pressure
            animationStartValue = scaleMin;
        }
        log_i("Initial pressure animation: starting from %d to %d", animationStartValue, value);
    }
    else if (currentOilPressureValue_ < scaleMin && value >= scaleMin)
    {
        // Coming from below scale minimum into visible range
        animationStartValue = scaleMin;
        log_i("Pressure entering scale from below: animating from %d to %d", animationStartValue, value);
    }
    else if (currentOilPressureValue_ > scaleMax && value <= scaleMax)
    {
        // Coming from above scale maximum into visible range
        animationStartValue = scaleMax;
        log_i("Pressure entering scale from above: animating from %d to %d", animationStartValue, value);
    }

    // Setup animation
    lv_anim_init(&pressureAnimation_);
    lv_anim_set_duration(&pressureAnimation_, _animation_duration);
    lv_anim_set_repeat_count(&pressureAnimation_, 0);
    lv_anim_set_playback_duration(&pressureAnimation_, 0);
    lv_anim_set_values(&pressureAnimation_, animationStartValue, value);
    lv_anim_set_var(&pressureAnimation_, this);
    lv_anim_set_user_data(&pressureAnimation_, (void *)static_cast<uintptr_t>(OilSensorTypes::Pressure));
    lv_anim_set_exec_cb(&pressureAnimation_, OemOilPanel::ExecutePressureAnimationCallback);
    lv_anim_set_completed_cb(&pressureAnimation_, OemOilPanel::UpdatePanelCompletionCallback);

    // Update animation state based on current state
    if (animationState_ == AnimationState::TEMPERATURE_RUNNING) {
        animationState_ = AnimationState::BOTH_RUNNING;
    } else {
        animationState_ = AnimationState::PRESSURE_RUNNING;
    }
    // Set ANIMATING state before starting animation
    if (panelService_)
    {
        panelService_->SetUiState(UIState::BUSY);
    }
    // Start pressure gauge animation
    lv_anim_start(&pressureAnimation_);
}

/// @brief Update the oil temperature reading on the screen
void OemOilPanel::UpdateOilTemperature(bool forceRefresh)
{

    // Skip update if temperature animation is already running
    if (animationState_ == AnimationState::TEMPERATURE_RUNNING || animationState_ == AnimationState::BOTH_RUNNING)
    {
        // Validate that the LVGL animation is actually still running
        // If not, this is a stale flag that needs to be cleared
        if (!lv_anim_get(&temperatureAnimation_, nullptr))
        {
            log_w("UpdateOilTemperature: Found stale animation flag, clearing it");
            if (animationState_ == AnimationState::TEMPERATURE_RUNNING) {
                animationState_ = AnimationState::IDLE;
            } else if (animationState_ == AnimationState::BOTH_RUNNING) {
                animationState_ = AnimationState::PRESSURE_RUNNING;
            }
        }
        else
        {
            return;
        }
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

    // Handle forced refresh (theme changes) or skip if unchanged
    if (value == currentOilTemperatureValue_)
    {
        if (forceRefresh)
        {
            oemOilTemperatureComponent_->Refresh(Reading{value});
        }
        return; // No animation needed since value didn't change
    }

    log_i("Updating temperature from %d to %d", currentOilTemperatureValue_, value);
    oemOilTemperatureComponent_->Refresh(Reading{value});

    // Determine animation start value
    // If current value is -1 (initial state) or outside scale bounds, start from appropriate boundary
    int32_t animationStartValue = currentOilTemperatureValue_;
    const int32_t scaleMin = 0;   // Temperature scale minimum
    const int32_t scaleMax = 120; // Temperature scale maximum
    
    if (currentOilTemperatureValue_ == -1)
    {
        // Initial load - determine start position based on target value
        if (value < scaleMin)
        {
            animationStartValue = scaleMin; // Start from minimum if value is below scale
        }
        else if (value > scaleMax)
        {
            animationStartValue = scaleMax; // Start from maximum if value is above scale
        }
        else
        {
            // Value is within scale bounds
            // For temperature, always start from minimum (cold) for initial animation
            // This represents a cold engine warming up, which is the typical scenario
            animationStartValue = scaleMin;
        }
        log_i("Initial temperature animation: starting from %d to %d", animationStartValue, value);
    }
    else if (currentOilTemperatureValue_ < scaleMin && value >= scaleMin)
    {
        // Coming from below scale minimum into visible range
        animationStartValue = scaleMin;
        log_i("Temperature entering scale from below: animating from %d to %d", animationStartValue, value);
    }
    else if (currentOilTemperatureValue_ > scaleMax && value <= scaleMax)
    {
        // Coming from above scale maximum into visible range
        animationStartValue = scaleMax;
        log_i("Temperature entering scale from above: animating from %d to %d", animationStartValue, value);
    }

    // Setup animation
    lv_anim_init(&temperatureAnimation_);
    lv_anim_set_duration(&temperatureAnimation_, _animation_duration);
    lv_anim_set_repeat_count(&temperatureAnimation_, 0);
    lv_anim_set_playback_duration(&temperatureAnimation_, 0);
    lv_anim_set_values(&temperatureAnimation_, animationStartValue, value);
    lv_anim_set_var(&temperatureAnimation_, this);
    lv_anim_set_user_data(&temperatureAnimation_, (void *)static_cast<uintptr_t>(OilSensorTypes::Temperature));
    lv_anim_set_exec_cb(&temperatureAnimation_, OemOilPanel::ExecuteTemperatureAnimationCallback);
    lv_anim_set_completed_cb(&temperatureAnimation_, OemOilPanel::UpdatePanelCompletionCallback);

    // Update animation state based on current state
    if (animationState_ == AnimationState::PRESSURE_RUNNING) {
        animationState_ = AnimationState::BOTH_RUNNING;
    } else {
        animationState_ = AnimationState::TEMPERATURE_RUNNING;
    }
    // Set ANIMATING state before starting animation
    if (panelService_)
    {
        panelService_->SetUiState(UIState::BUSY);
    }
    // Start temperature gauge animation
    lv_anim_start(&temperatureAnimation_);
}

// Legacy Action interface methods (retained for reference)
/*
/// @brief Get action for short button press
/// @return NoAction - oil panel doesn't respond to short presses
Action OemOilPanel::GetShortPressAction()
{
    log_v("GetShortPressAction() called");

    // Short press: No action for oil panel
    return Action(nullptr);
}

/// @brief Get action for long button press
/// @return Action to switch to CONFIG panel
Action OemOilPanel::GetLongPressAction()
{
    log_v("GetLongPressAction() called");

    // Return an action that directly calls PanelService interface
    if (!panelService_)
    {
        log_w("OemOilPanel: PanelService not available, returning no action");
        return Action(nullptr);
    }

    return Action(
        [this]()
        {
            panelService_->CreateAndLoadPanel(PanelNames::CONFIG, true);
        });
}
*/

/// @brief Manager injection method to prevent circular references
/// @param panelService the panel manager instance
/// @param styleService  the style manager instance
void OemOilPanel::SetManagers(IPanelService *panelService, IStyleService *styleService)
{
    log_v("SetManagers() called");

    panelService_ = panelService;
    // styleService_ is already set in constructor, but update if different instance provided
    if (styleService != styleService_)
    {
        styleService_ = styleService;
    }
}

/// @brief Set preference service and apply sensor update rate from preferences
/// @param preferenceService The preference service to use for configuration
void OemOilPanel::SetPreferenceService(IPreferenceService *preferenceService)
{
    log_v("SetPreferenceService() called");

    preferenceService_ = preferenceService;

    // Apply sensor configuration from preferences when service is first injected
    ApplyCurrentSensorSettings();
}

/// @brief Apply current sensor settings from preferences directly
void OemOilPanel::ApplyCurrentSensorSettings()
{
    // Apply updateRate and units from preferences to sensors if preference service is available
    if (preferenceService_ && cachedPressureSensor_ && cachedTemperatureSensor_)
    {
        auto config = preferenceService_->GetConfig();

        // Only update settings if they have changed
        bool updateRateChanged = (lastUpdateRate_ != config.updateRate);
        bool pressureUnitChanged = !lastPressureUnit_.equals(config.pressureUnit.c_str());
        bool tempUnitChanged = !lastTempUnit_.equals(config.tempUnit.c_str());

        if (updateRateChanged)
        {
            cachedPressureSensor_->SetUpdateRate(config.updateRate);
            cachedTemperatureSensor_->SetUpdateRate(config.updateRate);
            lastUpdateRate_ = config.updateRate;
        }

        if (pressureUnitChanged)
        {
            cachedPressureSensor_->SetTargetUnit(config.pressureUnit);
            lastPressureUnit_ = String(config.pressureUnit.c_str());
        }

        if (tempUnitChanged)
        {
            cachedTemperatureSensor_->SetTargetUnit(config.tempUnit);
            lastTempUnit_ = String(config.tempUnit.c_str());
        }

        // Only log if something actually changed
        if (updateRateChanged || pressureUnitChanged || tempUnitChanged)
        {
            log_t("Sensor settings updated - Rate: %dms, Pressure: %s, Temp: %s",
                  config.updateRate, config.pressureUnit, config.tempUnit);
        }
    }
    else if (!preferenceService_)
    {
        // Only log this warning once during initialization
        static bool logged = false;
        if (!logged)
        {
            log_w("ApplyCurrentSensorSettings: No PreferenceService available - using default sensor settings");
            logged = true;
        }
    }
}

// Static Callback Methods

/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void OemOilPanel::ShowPanelCompletionCallback(lv_event_t *event)
{
    log_v("ShowPanelCompletionCallback() called");

    if (!event)
    {
        return;
    }

    auto thisInstance = static_cast<OemOilPanel *>(lv_event_get_user_data(event));
    if (!thisInstance)
    {
        return;
    }

    // Set UI state to IDLE so main loop can start calling Update()
    if (thisInstance->panelService_)
    {
        thisInstance->panelService_->SetUiState(UIState::IDLE);
    }
    
    // Animation completed - no callback needed for interface-based approach
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
        // Update animation state when pressure animation completes
        if (thisInstance->animationState_ == AnimationState::PRESSURE_RUNNING) {
            thisInstance->animationState_ = AnimationState::IDLE;
        } else if (thisInstance->animationState_ == AnimationState::BOTH_RUNNING) {
            thisInstance->animationState_ = AnimationState::TEMPERATURE_RUNNING;
        }
    }
    else if (sensorType == OilSensorTypes::Temperature)
    {
        thisInstance->currentOilTemperatureValue_ = animation->current_value;
        // Update animation state when temperature animation completes
        if (thisInstance->animationState_ == AnimationState::TEMPERATURE_RUNNING) {
            thisInstance->animationState_ = AnimationState::IDLE;
        } else if (thisInstance->animationState_ == AnimationState::BOTH_RUNNING) {
            thisInstance->animationState_ = AnimationState::PRESSURE_RUNNING;
        }
    }

    // Only set UI state to idle when all animations are complete
    if (thisInstance->animationState_ == AnimationState::IDLE)
    {
        // Set IDLE state when all animations are complete
        if (thisInstance->panelService_)
        {
            thisInstance->panelService_->SetUiState(UIState::IDLE);
        }
    }
}

/// @brief callback used by the animation to set the values smoothly until ultimate value is reached
/// @param target the object being animated
/// @param value the next value in a sequence to create a smooth transition
void OemOilPanel::ExecutePressureAnimationCallback(void *target, int32_t value)
{
    log_v("ExecutePressureAnimationCallback() called with value: %d", value);

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
    log_v("ExecuteTemperatureAnimationCallback() called with value: %d", value);

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

    if (!preferenceService_)
    {
        int32_t result = MapBarPressure(sensorValue);
        log_v("MapPressureValue() returning (fallback): %d", result);
        return result;
    }

    const Configs &config = preferenceService_->GetConfig();
    int32_t mappedValue = MapPressureByUnit(sensorValue, config.pressureUnit);
    
    log_v("MapPressureValue() returning: %d", mappedValue);
    return mappedValue;
}

int32_t OemOilPanel::MapPressureByUnit(int32_t sensorValue, const std::string& unit)
{
    if (unit == "PSI") return MapPSIPressure(sensorValue);
    if (unit == "kPa") return MapkPaPressure(sensorValue);
    
    return MapBarPressure(sensorValue);
}

int32_t OemOilPanel::MapPSIPressure(int32_t sensorValue)
{
    // Sensor returns 0-145 PSI, map to 0-60 display
    // Clamp to valid range (14.5-145 PSI equivalent to 1-10 Bar)
    int32_t clampedValue = ClampValue(sensorValue, 15, 145);
    
    // Map useful PSI range (15-145) to display scale (0-60)
    return ((clampedValue - 15) * 60) / 130;
}

int32_t OemOilPanel::MapkPaPressure(int32_t sensorValue)
{
    // Sensor returns 0-1000 kPa, map to 0-60 display
    // Clamp to valid range (100-1000 kPa equivalent to 1-10 Bar)
    int32_t clampedValue = ClampValue(sensorValue, 100, 1000);
    
    // Map useful kPa range (100-1000) to display scale (0-60)
    return ((clampedValue - 100) * 60) / 900;
}

int32_t OemOilPanel::MapBarPressure(int32_t sensorValue)
{
    // Bar: sensor returns 0-10 Bar, map to 0-60 display
    // Clamp to valid range (1-10 Bar)
    int32_t clampedValue = ClampValue(sensorValue, 1, 10);
    
    // Map Bar range (1-10) to display scale (0-60)
    return ((clampedValue - 1) * 60) / 9;
}

int32_t OemOilPanel::ClampValue(int32_t value, int32_t minVal, int32_t maxVal)
{
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
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

    log_v("MapTemperatureValue() returning: %d", mappedValue);
    return mappedValue;
}

// IActionService Interface Implementation

static void OemOilPanelShortPress(void* panelContext)
{
    log_v("OemOilPanelShortPress() called");
    // No action for short press on oil panel
}

static void OemOilPanelLongPress(void* panelContext)
{
    log_v("OemOilPanelLongPress() called");
    auto* panel = static_cast<OemOilPanel*>(panelContext);
    
    if (panel)
    {
        // Call public method to handle the long press
        panel->HandleLongPress();
    }
}

void (*OemOilPanel::GetShortPressFunction())(void* panelContext)
{
    return OemOilPanelShortPress;
}

void (*OemOilPanel::GetLongPressFunction())(void* panelContext)
{
    return OemOilPanelLongPress;
}

void* OemOilPanel::GetPanelContext()
{
    return this;
}

void OemOilPanel::HandleLongPress()
{
    if (panelService_)
    {
        log_i("OemOilPanel long press - loading config panel");
        panelService_->CreateAndLoadPanel(PanelNames::CONFIG, true);
    }
    else
    {
        log_w("OemOilPanel: Cannot load config panel - panelService not available");
    }
}