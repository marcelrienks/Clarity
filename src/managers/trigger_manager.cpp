#include "managers/trigger_manager.h"

// Static Methods

/// @brief Get the singleton instance of TriggerManager
/// @return Reference to the single TriggerManager instance
/// @details Thread-safe singleton pattern ensures only one TriggerManager exists
TriggerManager &TriggerManager::GetInstance()
{
    static TriggerManager instance;
    return instance;
}

// Core Functionality Methods

/// @brief Initialize the dual-core trigger system
/// @details Sets up the complete trigger infrastructure:
/// - Creates ISR event queue for Core 1 ↔ ISR communication
/// - Creates mutexes for thread-safe shared state access
/// - Configures GPIO pins and attaches interrupt handlers
/// - Launches Core 1 monitoring task for processing ISR events
/// This method bridges hardware interrupts to application triggers
void TriggerManager::init()
{
    log_d("...");

    // Create ISR event queue for safe interrupt → task communication
    isrEventQueue = xQueueCreate(10, sizeof(ISREvent));
    
    // Create mutexes for thread-safe access to shared state
    stateMutex = xSemaphoreCreateMutex();      // Hardware state protection
    triggerMutex_ = xSemaphoreCreateMutex();   // Active triggers map protection

    // Configure GPIO pins and attach interrupt handlers
    setup_gpio_interrupts();

    // Launch Core 1 task to process ISR events safely
    xTaskCreatePinnedToCore(
        TriggerManager::TriggerMonitoringTask,              // Task function
        TRIGGER_MONITOR_TASK, // Task name
        4096,                                               // Stack size
        nullptr,                                            // Parameters
        configMAX_PRIORITIES - 1,                           // Priority (highest)
        &triggerTaskHandle_,                                // Task handle
        1                                                   // Core 1 (PRO_CPU)
    );
    
    log_d("Trigger system initialized on dual-core architecture");
}

/// @brief Handle key present GPIO state change from Core 1 task context
/// @param keyPresentState true if key present pin is HIGH, false if LOW
/// @details Called by TriggerMonitoringTask when key present ISR event is processed.
/// Thread-safe processing of key present detection with mutex protection.
/// Converts GPIO state to trigger actions based on current panel context.
void TriggerManager::HandleKeyPresentInterrupt(bool keyPresentState)
{
    log_d("...");

    // Acquire mutex for thread-safe hardware state access
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        log_w("Failed to acquire state mutex for key present interrupt");
        return;
    }

    // Convert GPIO state to appropriate trigger action
    ProcessGpioStateChange(keyPresentState, PanelNames::KEY, TRIGGER_KEY_PRESENT, TriggerPriority::CRITICAL);
    
    // Update internal hardware state tracking
    keyPresentState_ = keyPresentState;
    
    xSemaphoreGive(stateMutex);
}

/// @brief Handle key not present GPIO state change from Core 1 task context
/// @param keyNotPresentState true if key not present pin is HIGH, false if LOW
/// @details Called by TriggerMonitoringTask when key not present ISR event is processed.
/// Thread-safe processing of key absence detection with mutex protection.
/// Converts GPIO state to trigger actions based on current panel context.
void TriggerManager::HandleKeyNotPresentInterrupt(bool keyNotPresentState)
{
    log_d("...");

    // Acquire mutex for thread-safe hardware state access
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        log_w("Failed to acquire state mutex for key not present interrupt");
        return;
    }

    // Convert GPIO state to appropriate trigger action
    ProcessGpioStateChange(keyNotPresentState, PanelNames::KEY, TRIGGER_KEY_NOT_PRESENT, TriggerPriority::CRITICAL);
    
    // Update internal hardware state tracking
    keyNotPresentState_ = keyNotPresentState;
    
    xSemaphoreGive(stateMutex);
}

/// @brief Handle lock state GPIO change from Core 1 task context
/// @param lockEngaged true if lock pin is HIGH (engaged), false if LOW (disengaged)
/// @details Called by TriggerMonitoringTask when lock state ISR event is processed.
/// Thread-safe processing of lock engagement detection with mutex protection.
/// Converts GPIO state to trigger actions based on current panel context.
void TriggerManager::HandleLockStateInterrupt(bool lockEngaged)
{
    log_d("...");

    // Acquire mutex for thread-safe hardware state access
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        log_w("Failed to acquire state mutex for lock state interrupt");
        return;
    }

    // Convert GPIO state to appropriate trigger action
    ProcessGpioStateChange(lockEngaged, PanelNames::LOCK, TRIGGER_LOCK_STATE, TriggerPriority::IMPORTANT);
    
    // Update internal hardware state tracking
    lockEngagedState_ = lockEngaged;
    
    xSemaphoreGive(stateMutex);
}

/// @brief Handle theme switch GPIO change from Core 1 task context
/// @param nightMode true if lights pin is HIGH (night mode), false if LOW (day mode)
/// @details Called by TriggerMonitoringTask when theme switch ISR event is processed.
/// Thread-safe processing of theme switching with mutex protection.
/// Only creates trigger if theme actually needs to change to avoid unnecessary work.
void TriggerManager::HandleThemeSwitchInterrupt(bool nightMode)
{
    log_d("...");

    // Acquire mutex for thread-safe theme state access
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(THEME_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        log_w("Failed to acquire state mutex for theme switch interrupt");
        return;
    }

    // Determine target theme based on GPIO state
    const char *targetTheme = nightMode ? THEME_NIGHT : THEME_DAY;
    const char *currentTheme = StyleManager::GetInstance().THEME;

    // Only create trigger if theme actually needs to change
    if (strcmp(currentTheme, targetTheme) != 0)
    {
        log_d("Theme change needed: %s → %s", currentTheme, targetTheme);
        SetTriggerState(TRIGGER_THEME_SWITCH, ACTION_CHANGE_THEME, targetTheme, TriggerPriority::NORMAL);
    }
    else
    {
        log_d("Theme already set to %s, clearing any pending theme triggers", targetTheme);
        ClearTriggerState(TRIGGER_THEME_SWITCH);
    }

    // Update internal hardware state tracking
    nightModeState_ = nightMode;
    
    xSemaphoreGive(stateMutex);
}


// Task Methods

/// @brief Core 1 monitoring task that processes ISR events safely
/// @param pvParameters Unused task parameters (required by FreeRTOS)
/// @details This task runs on Core 1 and serves as the bridge between
/// hardware interrupts (ISR context) and application processing (task context).
/// ISR handlers post events to a queue, this task processes them safely.
/// 
/// **Flow**: GPIO Interrupt → ISR Handler → Queue Event → This Task → Handle*Interrupt()
/// 
/// **Why needed**: ISR context is heavily restricted (no logging, limited mutex use,
/// no blocking operations). This task provides safe context for full processing.
void TriggerManager::TriggerMonitoringTask(void *pvParameters)
{
    log_d("...");

    TriggerManager &manager = TriggerManager::GetInstance();
    ISREvent event;

    // Infinite task loop - process ISR events safely in task context
    while (1)
    {
        // Wait for ISR events from interrupt handlers (100ms timeout)
        if (xQueueReceive(manager.isrEventQueue, &event, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            log_v("Processing ISR event type %d with pin state %d", (int)event.eventType, event.pinState);
            
            // Dispatch event to appropriate handler method
            // These methods can safely use mutexes, logging, and blocking operations
            switch (event.eventType)
            {
            case ISREventType::KEY_PRESENT:
                manager.HandleKeyPresentInterrupt(event.pinState);
                break;

            case ISREventType::KEY_NOT_PRESENT:
                manager.HandleKeyNotPresentInterrupt(event.pinState);
                break;

            case ISREventType::LOCK_STATE_CHANGE:
                manager.HandleLockStateInterrupt(event.pinState);
                break;

            case ISREventType::THEME_SWITCH:
                manager.HandleThemeSwitchInterrupt(event.pinState);
                break;
                
            default:
                log_w("Unknown ISR event type: %d", (int)event.eventType);
                break;
            }
        }
        // Note: 100ms timeout allows periodic task scheduling even without events
    }
}

// Private Methods

/// @brief Create or update a trigger in the active triggers map
/// @param triggerId Unique string identifier for the trigger (e.g., "key_present")
/// @param action Action to perform ("LoadPanel", "RestorePreviousPanel", "ChangeTheme")
/// @param target Target for the action (panel name, theme name, etc.)
/// @param priority Priority level (CRITICAL, IMPORTANT, NORMAL)
/// @details Thread-safe method that adds/updates triggers in the shared state map.
/// This is where GPIO state changes get converted into actionable triggers.
/// Core 0 will later retrieve these triggers for processing.
void TriggerManager::SetTriggerState(const char *triggerId, const char *action, const char *target, TriggerPriority priority)
{
    // Acquire mutex for thread-safe access to triggers map
    if (xSemaphoreTake(triggerMutex_, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        return;
    }

    log_d("...");

    // Create/update trigger state with current timestamp
    activeTriggers_[triggerId] = TriggerState(action, target, priority, esp_timer_get_time());
    
    xSemaphoreGive(triggerMutex_);
}

/// @brief Remove a trigger from the active triggers map
/// @param triggerId Unique string identifier for the trigger to remove
/// @details Thread-safe method that removes completed triggers from the shared state map.
/// Called by Core 0 after trigger action has been executed successfully.
/// Essential for preventing infinite trigger loops.
void TriggerManager::ClearTriggerState(const char *triggerId)
{
    // Acquire mutex for thread-safe access to triggers map
    if (xSemaphoreTake(triggerMutex_, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        return;
    }

    log_d("...");

    // Find and remove trigger from active triggers map
    auto it = activeTriggers_.find(triggerId);
    if (it != activeTriggers_.end())
    {
        log_d("Trigger %s cleared successfully", triggerId);
        activeTriggers_.erase(it);
    }
    else
    {
        log_w("Trigger %s not found for clearing - may have been processed already", triggerId);
    }
    
    xSemaphoreGive(triggerMutex_);
}


/// @brief Get the highest priority trigger ready for processing
/// @return Pair of (trigger ID, trigger state pointer) or (nullptr, nullptr) if none available
/// @details Thread-safe method called by Core 0 to retrieve next trigger to process.
/// Implements priority-based selection with debouncing and duplicate prevention.
/// 
/// **Selection Logic**:
/// 1. Higher priority triggers are selected first (CRITICAL > IMPORTANT > NORMAL)
/// 2. Within same priority, older triggers are selected first (FIFO)
/// 3. Debouncing: Same trigger can't be processed within 500ms
/// 4. Processing flag prevents double-processing of same trigger
/// 
/// **Critical Feature**: Returns trigger ID directly from map key - no guessing needed!
std::pair<const char*, TriggerState*> TriggerManager::GetHighestPriorityTrigger()
{
    // Acquire mutex for thread-safe access to triggers map
    if (xSemaphoreTake(triggerMutex_, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        return std::make_pair(nullptr, nullptr);
    }

    log_d("...");

    TriggerState *highest = nullptr;
    const char* highestId = nullptr;
    TriggerPriority highestPriority = TriggerPriority::NORMAL;
    uint64_t oldestTimestamp = UINT64_MAX;
    uint64_t currentTime = esp_timer_get_time();

    // Scan all active triggers to find the best candidate
    for (auto &pair : activeTriggers_)
    {
        const char* triggerId = pair.first;
        TriggerState &trigger = pair.second;
        
        // Skip inactive or already processing triggers
        if (!trigger.active || trigger.processing)
        {
            log_v("Skipping trigger %s: active=%d, processing=%d", triggerId, trigger.active, trigger.processing);
            continue;
        }
            
        // Skip if trigger was processed too recently (500ms debouncing)
        if (trigger.lastProcessed > 0 && 
            (currentTime - trigger.lastProcessed) < TRIGGER_DEBOUNCE_TIME_US)
        {
            log_v("Skipping trigger %s: debouncing (%llu μs ago)", 
                  triggerId, currentTime - trigger.lastProcessed);
            continue;
        }

        // Check if this trigger should be selected over current best candidate
        if (ShouldUpdateHighestPriority(trigger, highest, highestPriority, oldestTimestamp))
        {
            highest = &trigger;
            highestId = triggerId;
            highestPriority = trigger.priority;
            oldestTimestamp = trigger.timestamp;
            log_v("New highest priority candidate: %s [priority: %d]", triggerId, (int)highestPriority);
        }
    }

    // Mark the selected trigger as processing to prevent reprocessing
    if (highest != nullptr)
    {
        highest->processing = true;
        highest->lastProcessed = currentTime;
        log_d("Selected trigger %s for processing [priority: %d, action: %s]", 
              highestId, (int)highest->priority, highest->action.c_str());
    }
    else
    {
        log_v("No triggers available for processing");
    }

    xSemaphoreGive(triggerMutex_);
    return std::make_pair(highestId, highest);
}

/// @brief Configure GPIO pins and attach interrupt handlers
/// @details Sets up hardware interrupt system for all monitored GPIO inputs.
/// Configures pins as INPUT_PULLDOWN and attaches ISR handlers for state changes.
/// All interrupts trigger on CHANGE (both rising and falling edges).
/// 
/// **Hardware Setup**:
/// - KEY_PRESENT: Pin goes HIGH when key is detected
/// - KEY_NOT_PRESENT: Pin goes HIGH when key absence is detected  
/// - LOCK: Pin goes HIGH when lock is engaged
/// - LIGHTS: Pin goes HIGH when headlights are on (night mode)
void TriggerManager::setup_gpio_interrupts()
{
    log_d("...");

    // Configure GPIO pins as inputs with pull-down resistors
    pinMode(gpio_pins::KEY_PRESENT, INPUT_PULLDOWN);
    pinMode(gpio_pins::KEY_NOT_PRESENT, INPUT_PULLDOWN);
    pinMode(gpio_pins::LOCK, INPUT_PULLDOWN);
    pinMode(gpio_pins::LIGHTS, INPUT_PULLDOWN);

    // Attach interrupt handlers for state change detection (both edges)
    attachInterruptArg(gpio_pins::KEY_PRESENT, keyPresentIsrHandler, (void *)true, CHANGE);
    attachInterruptArg(gpio_pins::KEY_NOT_PRESENT, keyNotPresentIsrHandler, (void *)false, CHANGE);
    attachInterruptArg(gpio_pins::LOCK, lockStateIsrHandler, nullptr, CHANGE);
    attachInterruptArg(gpio_pins::LIGHTS, themeSwitchIsrHandler, nullptr, CHANGE);
    
    log_d("GPIO interrupts configured successfully");
}

// Static interrupt handlers

/// @brief ISR handler for key present GPIO pin changes
/// @param arg Unused parameter (required by ESP32 interrupt system)
/// @details **CRITICAL**: This runs in ISR context with severe restrictions:
/// - No logging, no printf, no blocking operations
/// - Limited time - must execute quickly
/// - Can only use ISR-safe FreeRTOS functions (ending in FromISR)
/// 
/// **Purpose**: Safely capture GPIO state change and queue event for task processing
/// **Flow**: GPIO Change → This ISR → Queue Event → TriggerMonitoringTask → Handle*Interrupt()
void IRAM_ATTR TriggerManager::keyPresentIsrHandler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // Read current pin state (HIGH = key present detected)
    bool pinState = digitalRead(gpio_pins::KEY_PRESENT);

    // Create event and queue it for safe processing in task context
    ISREvent event(ISREventType::KEY_PRESENT, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);

    // Yield to higher priority task if queue send woke one up
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/// @brief ISR handler for key not present GPIO pin changes
/// @details Same ISR pattern as keyPresentIsrHandler - queues event for safe task processing
void IRAM_ATTR TriggerManager::keyNotPresentIsrHandler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // Read current pin state (HIGH = key not present detected)
    bool pinState = digitalRead(gpio_pins::KEY_NOT_PRESENT);

    // Queue event for safe processing in task context
    ISREvent event(ISREventType::KEY_NOT_PRESENT, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/// @brief ISR handler for lock state GPIO pin changes  
/// @details Same ISR pattern - queues event for safe task processing
void IRAM_ATTR TriggerManager::lockStateIsrHandler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // Read current pin state (HIGH = lock engaged)
    bool pinState = digitalRead(gpio_pins::LOCK);

    // Queue event for safe processing in task context
    ISREvent event(ISREventType::LOCK_STATE_CHANGE, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/// @brief ISR handler for theme switch (lights) GPIO pin changes
/// @details Same ISR pattern - queues event for safe task processing
void IRAM_ATTR TriggerManager::themeSwitchIsrHandler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // Read current pin state (HIGH = lights on, night mode)
    bool pinState = digitalRead(gpio_pins::LIGHTS);

    // Queue event for safe processing in task context
    ISREvent event(ISREventType::THEME_SWITCH, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// C-style ISR wrappers (required for ESP32)
extern "C"
{
    void IRAM_ATTR gpio_key_present_isr(void *arg)
    {
        TriggerManager::keyPresentIsrHandler(arg);
    }

    void IRAM_ATTR gpio_key_not_present_isr(void *arg)
    {
        TriggerManager::keyNotPresentIsrHandler(arg);
    }

    void IRAM_ATTR gpio_lock_state_isr(void *arg)
    {
        TriggerManager::lockStateIsrHandler(arg);
    }

    void IRAM_ATTR gpio_theme_switch_isr(void *arg)
    {
        TriggerManager::themeSwitchIsrHandler(arg);
    }
}

/// @brief Convert GPIO state change to appropriate trigger action
/// @param state true if GPIO pin is HIGH, false if LOW
/// @param panelName Target panel name for this GPIO (e.g., "KeyPanel", "LockPanel")
/// @param triggerId Unique trigger identifier (e.g., "key_present", "lock_state")
/// @param priority Priority level for the trigger
/// @details **Core Logic**: This method implements the contextual trigger behavior:
/// 
/// **When GPIO goes HIGH (state=true)**:
/// - If not already showing target panel → Create "LoadPanel" trigger
/// - If already showing target panel → Do nothing (avoid redundant switches)
/// 
/// **When GPIO goes LOW (state=false)**:
/// - If currently showing target panel → Create "RestorePreviousPanel" trigger  
/// - If showing different panel → Do nothing (don't interfere)
/// 
/// **Example**: Key present pin goes HIGH while showing oil panel → Load key panel
///             Key present pin goes LOW while showing key panel → Restore oil panel
void TriggerManager::ProcessGpioStateChange(bool state, const char *panelName, const char *triggerId, TriggerPriority priority)
{
    const char *currentPanel = PanelManager::GetInstance().currentPanel;
    
    log_d("Processing GPIO state change: %s=%s, currentPanel=%s, targetPanel=%s", 
          triggerId, state ? "HIGH" : "LOW", currentPanel, panelName);

    if (state)
    {
        // GPIO pin is HIGH - trigger is active
        if (strcmp(currentPanel, panelName) != 0)
        {
            // Not showing target panel - create trigger to load it
            log_d("GPIO HIGH and not showing target panel - creating LoadPanel trigger");
            SetTriggerState(triggerId, ACTION_LOAD_PANEL, panelName, priority);
        }
        else
        {
            log_v("GPIO HIGH but already showing target panel - no action needed");
        }
    }
    else
    {
        // GPIO pin is LOW - trigger is inactive
        if (strcmp(currentPanel, panelName) == 0)
        {
            // Currently showing this trigger's panel - restore previous panel
            log_d("GPIO LOW and showing target panel - creating RestorePreviousPanel trigger");
            SetTriggerState(triggerId, ACTION_RESTORE_PREVIOUS_PANEL, "", TriggerPriority::NORMAL);
        }
        else
        {
            log_v("GPIO LOW but not showing target panel - no action needed");
        }
        // Note: Don't clear pending triggers when state goes inactive - let them execute
    }
}

/// @brief Determine if a trigger should replace the current highest priority candidate
/// @param trigger Trigger being evaluated
/// @param currentHighest Current best candidate (nullptr if none yet)
/// @param currentPriority Priority of current best candidate
/// @param currentTimestamp Timestamp of current best candidate
/// @return true if this trigger should become the new highest priority candidate
/// @details **Selection Logic**:
/// 1. If no current candidate → Select this trigger
/// 2. If this trigger has higher priority → Select this trigger
/// 3. If same priority but this trigger is older → Select this trigger (FIFO)
/// 4. Otherwise → Keep current candidate
/// 
/// **Priority Order**: CRITICAL (0) > IMPORTANT (1) > NORMAL (2) - lower number = higher priority
bool TriggerManager::ShouldUpdateHighestPriority(const TriggerState &trigger, TriggerState *currentHighest, TriggerPriority currentPriority, uint64_t currentTimestamp)
{
    // No current candidate - select this trigger
    if (currentHighest == nullptr)
    {
        return true;
    }

    // This trigger has higher priority - select it (lower enum value = higher priority)
    if (trigger.priority < currentPriority)
    {
        return true;
    }

    // Same priority but this trigger is older - select it (FIFO within priority level)
    if (trigger.priority == currentPriority && trigger.timestamp < currentTimestamp)
    {
        return true;
    }

    // Keep current candidate
    return false;
}
