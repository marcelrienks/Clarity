#include "managers/interrupt_manager.h"
#include "managers/error_manager.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "handlers/polled_handler.h"
#include "handlers/queued_handler.h"
#include "sensors/lights_sensor.h"
#include "sensors/lock_sensor.h"
#include "sensors/key_present_sensor.h"
#include "sensors/key_not_present_sensor.h"
#include "sensors/action_button_sensor.h"
#include "utilities/constants.h"
#include <Arduino.h>
#include <cstring>

#include "esp32-hal-log.h"

// External references to global managers (defined in main.cpp)
extern std::unique_ptr<PanelManager> panelManager;
extern std::unique_ptr<StyleManager> styleManager;

// Singleton implementation
InterruptManager& InterruptManager::Instance()
{
    static InterruptManager instance;
    log_d("InterruptManager::Instance() returning singleton reference");
    return instance;
}

void InterruptManager::Init(IGpioProvider* gpioProvider)
{
    log_v("Init() called");
    if (initialized_)
    {
        log_w("InterruptManager already initialized");
        return;
    }

    // Initialize interrupt storage and timing
    interruptCount_ = 0;
    handlers_.clear();
    lastEvaluationTime_ = millis();
    lastCheckTime_ = millis();
    checkCount_ = 0;

    // Store GPIO provider for later handler creation to avoid circular dependency
    gpioProvider_ = gpioProvider;
    
    // Mark as initialized before creating handlers to prevent circular dependency
    // during sensor initialization when they call RegisterInterrupt()
    initialized_ = true;
    
    // Create and register default handlers with GPIO provider
    if (gpioProvider) {
        auto polledHandler = std::make_shared<PolledHandler>(gpioProvider);
        auto queuedHandler = std::make_shared<QueuedHandler>(gpioProvider);
        
        if (polledHandler && queuedHandler)
        {
            RegisterHandler(polledHandler);
            RegisterHandler(queuedHandler);
            
            // Store references for direct access
            polledHandler_ = polledHandler;
            queuedHandler_ = queuedHandler;
            
            log_d("Registered default PolledHandler and QueuedHandler with GPIO provider");
        }
        else
        {
            log_e("Failed to create default handlers with GPIO provider");
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "InterruptManager", 
                                               "Failed to create default interrupt handlers");
        }
    } else {
        log_e("Cannot create handlers - GPIO provider is null");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "InterruptManager", 
                                           "Cannot create handlers - GPIO provider is null");
    }

    log_i("InterruptManager initialized with polled and queued interrupt handlers");
    
    // NOTE: Interrupt contexts are now set directly during registration in
    // ManagerFactory::RegisterSystemInterrupts(), eliminating the null context window
}

void InterruptManager::UpdateHandlerContexts()
{
    log_d("Updating interrupt contexts with handler-owned sensors");
    
    if (polledHandler_) {
        // Set contexts to the actual sensor instances owned by handlers
        UpdateInterruptContext("key_present", polledHandler_->GetKeyPresentSensor());
        UpdateInterruptContext("key_not_present", polledHandler_->GetKeyNotPresentSensor());
        UpdateInterruptContext("lock_state", polledHandler_->GetLockSensor());
        UpdateInterruptContext("lights_state", polledHandler_->GetLightsSensor());
        UpdateInterruptContext("error_occurred", &ErrorManager::Instance());
        log_d("Updated polled interrupt contexts with actual sensor pointers");
    }
    
    if (queuedHandler_) {
        UpdateInterruptContext("universal_short_press", queuedHandler_->GetActionButtonSensor());
        UpdateInterruptContext("universal_long_press", queuedHandler_->GetActionButtonSensor());
        log_d("Updated queued interrupt contexts with ActionButtonSensor pointer");
    }
}


// Core interrupt processing methods - implements 8-step flow
void InterruptManager::Process()
{
    log_v("InterruptManager::Process() called - 8-step flow entry");
    
    if (!initialized_)
    {
        return;
    }

    // Step 3: InterruptManager: Evaluate Queued (ALWAYS)
    EvaluateQueuedInterrupts();
    
    // Step 4: InterruptManager: Post Queued (ALWAYS) 
    PostQueuedInterrupts();
    
    // Step 5: InterruptManager: If Idle (check UI state)
    if (!IsUIIdle()) {
        log_v("UI not idle - skipping polled evaluation and execution");
        return; // Skip to step 8 (loop end)
    }
    
    // Step 6: InterruptManager: Evaluate and Action Polled (IDLE ONLY)
    EvaluateAndActionPolledInterrupts();
    
    // Step 7: InterruptManager: If Queue Action (IDLE ONLY)
    ProcessQueuedInterruptActions();
    
    log_v("InterruptManager::Process() completed 8-step flow");
}

// 8-step flow implementation methods

void InterruptManager::EvaluateQueuedInterrupts()
{
    log_v("Step 3: EvaluateQueuedInterrupts() called");
    
    if (!queuedHandler_ || !queuedHandler_->GetActionButtonSensor()) {
        return;
    }
    
    ActionButtonSensor* buttonSensor = queuedHandler_->GetActionButtonSensor();
    bool currentButtonState = buttonSensor->IsButtonPressed();
    unsigned long currentTime = millis();
    
    // Detect button press start (rising edge)
    if (currentButtonState && !buttonCurrentlyPressed_) {
        buttonPressStartTime_ = currentTime;
        buttonCurrentlyPressed_ = true;
        log_d("Button press started at %lu ms", buttonPressStartTime_);
    }
    // Detect button release (falling edge) 
    else if (!currentButtonState && buttonCurrentlyPressed_) {
        unsigned long pressDuration = currentTime - buttonPressStartTime_;
        buttonCurrentlyPressed_ = false;
        
        log_d("Button released after %lu ms", pressDuration);
        
        // Determine press type and queue for processing
        if (pressDuration >= 50 && pressDuration < 2000) {
            // Short press: 50ms to 2000ms
            log_i("Short press detected (%lu ms)", pressDuration);
            queuedInterruptsNeedProcessing_ = true;
            isLongPress_ = false;
        } else if (pressDuration >= 2000 && pressDuration < 5000) {
            // Long press: 2000ms to 5000ms  
            log_i("Long press detected (%lu ms)", pressDuration);
            queuedInterruptsNeedProcessing_ = true;
            isLongPress_ = true;
        }
        // Ignore presses outside valid ranges (debouncing)
    }
}

void InterruptManager::PostQueuedInterrupts()
{
    log_v("Step 4: PostQueuedInterrupts() called");
    
    if (queuedInterruptsNeedProcessing_) {
        log_d("Queued interrupts flagged for processing during idle time");
        // Button events are already queued by evaluation step
        // This step just confirms they're ready for processing
    }
}

bool InterruptManager::IsUIIdle() const
{
    log_v("Step 5: IsUIIdle() called");
    
    // For now, assume UI is always idle since we're called from LVGL idle callback
    // In a more sophisticated implementation, this could check LVGL task queue
    return true;
}

void InterruptManager::EvaluateAndActionPolledInterrupts()
{
    log_v("Step 6: EvaluateAndActionPolledInterrupts() called");
    
    if (!polledHandler_) {
        return;
    }
    
    // Let the polled handler evaluate and execute its interrupts
    polledHandler_->Process();
}

void InterruptManager::ProcessQueuedInterruptActions()
{
    log_v("Step 7: ProcessQueuedInterruptActions() called");
    
    if (!queuedInterruptsNeedProcessing_) {
        return;
    }
    
    if (!queuedHandler_ || !queuedHandler_->GetActionButtonSensor()) {
        queuedInterruptsNeedProcessing_ = false;
        return;
    }
    
    // Execute the appropriate button action based on stored press type
    if (isLongPress_) {
        // Long press action
        Interrupt* longPressInterrupt = FindInterrupt("universal_long_press");
        if (longPressInterrupt) {
            log_i("Executing queued long press action");
            ExecuteButtonAction(*longPressInterrupt);
        }
    } else {
        // Short press action  
        Interrupt* shortPressInterrupt = FindInterrupt("universal_short_press");
        if (shortPressInterrupt) {
            log_i("Executing queued short press action");
            ExecuteButtonAction(*shortPressInterrupt);
        }
    }
    
    // Clear the processing flag
    queuedInterruptsNeedProcessing_ = false;
}

bool InterruptManager::RegisterInterrupt(const Interrupt& interrupt)
{
    log_v("RegisterInterrupt() called for interrupt: %s", interrupt.id ? interrupt.id : "null");
    
    if (!interrupt.id || !interrupt.processFunc)
    {
        log_e("Invalid interrupt registration - missing required fields");
        return false;
    }
    
    if (interruptCount_ >= MAX_INTERRUPTS)
    {
        log_e("Cannot register interrupt - maximum capacity reached (%d)", MAX_INTERRUPTS);
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "InterruptManager", 
                                           "Maximum interrupt capacity exceeded");
        return false;
    }
    
    // Check for duplicate ID
    if (FindInterrupt(interrupt.id) != nullptr)
    {
        log_w("Interrupt with ID '%s' already exists", interrupt.id);
        return false;
    }
    
    // Add interrupt to array
    interrupts_[interruptCount_] = interrupt;
    interrupts_[interruptCount_].active = true;
    interrupts_[interruptCount_].lastEvaluation = 0;
    
    // Route interrupt to appropriate handler
    Interrupt* registeredInterrupt = &interrupts_[interruptCount_];
    interruptCount_++;
    
    if (interrupt.source == InterruptSource::POLLED && polledHandler_)
    {
        polledHandler_->RegisterInterrupt(registeredInterrupt);
        log_d("Routed polled interrupt '%s' to PolledHandler", interrupt.id);
    }
    else if (interrupt.source == InterruptSource::QUEUED && queuedHandler_)
    {
        // Queued interrupts are handled differently - they get queued when triggered
        log_d("Registered queued interrupt '%s' for QueuedHandler processing", interrupt.id);
    }
    
    log_d("Registered interrupt '%s' (total: %d)", interrupt.id, interruptCount_);
    return true;
}

void InterruptManager::UnregisterInterrupt(const char* id)
{
    log_v("UnregisterInterrupt() called for: %s", id ? id : "null");
    
    if (!id) return;
    
    for (size_t i = 0; i < interruptCount_; ++i)
    {
        if (interrupts_[i].id && strcmp(interrupts_[i].id, id) == 0)
        {
            // Unregister from handlers based on source type
            if (interrupts_[i].source == InterruptSource::POLLED && polledHandler_)
            {
                polledHandler_->UnregisterInterrupt(id);
                log_d("Unregistered polled interrupt '%s' from PolledHandler", id);
            }
            // Queued interrupts don't need explicit handler unregistration
            
            // Move last interrupt to this position to avoid gaps
            if (i < interruptCount_ - 1)
            {
                interrupts_[i] = interrupts_[interruptCount_ - 1];
            }
            interruptCount_--;
            log_d("Unregistered interrupt '%s' (remaining: %d)", id, interruptCount_);
            return;
        }
    }
    
    log_w("Interrupt '%s' not found for unregistration", id);
}

void InterruptManager::ActivateInterrupt(const char* id)
{
    log_v("ActivateInterrupt() called for: %s", id ? id : "null");
    
    Interrupt* interrupt = FindInterrupt(id);
    if (interrupt)
    {
        interrupt->active = true;
        log_d("Activated interrupt '%s'", id);
    }
    else
    {
        log_w("Interrupt '%s' not found for activation", id ? id : "null");
    }
}

void InterruptManager::DeactivateInterrupt(const char* id)
{
    log_v("DeactivateInterrupt() called for: %s", id ? id : "null");
    
    Interrupt* interrupt = FindInterrupt(id);
    if (interrupt)
    {
        interrupt->active = false;
        log_d("Deactivated interrupt '%s'", id);
        
        // When an interrupt is deactivated, check if we should restore or execute another interrupt
        HandleRestoration();
    }
    else
    {
        log_w("Interrupt '%s' not found for deactivation", id ? id : "null");
    }
}

void InterruptManager::UpdateInterruptContext(const char* id, void* context)
{
    log_v("UpdateInterruptContext() called for: %s", id ? id : "null");
    
    Interrupt* interrupt = FindInterrupt(id);
    if (interrupt)
    {
        interrupt->context = context;
        log_d("Updated context for interrupt '%s'", id);
    }
    else
    {
        log_w("Interrupt '%s' not found for context update", id ? id : "null");
    }
}

void InterruptManager::UpdateInterruptFunction(const char* id, void (*newFunc)(void* context))
{
    log_w("UpdateInterruptFunction() is deprecated with single-function interrupt design");
    log_w("Interrupt functions are now updated via button interrupt mechanism");
    // This method is kept for interface compatibility but is no longer functional
    // The single processFunc cannot be updated dynamically as it combines evaluation and execution
}

void InterruptManager::UpdateButtonInterrupts(void (*shortPressFunc)(void* context), 
                                             void (*longPressFunc)(void* context), 
                                             void* panelContext)
{
    log_v("UpdateButtonInterrupts() called");
    
    // Store the injected panel functions for use in ExecuteButtonAction
    currentShortPressFunc_ = shortPressFunc;
    currentLongPressFunc_ = longPressFunc;
    currentPanelContext_ = panelContext;
    
    log_d("Updated universal button interrupts with panel functions and context");
    log_d("Short press function: %p, Long press function: %p, Panel context: %p", 
          reinterpret_cast<void*>(shortPressFunc), reinterpret_cast<void*>(longPressFunc), panelContext);
}

void InterruptManager::RegisterHandler(std::shared_ptr<IHandler> handler)
{
    log_v("RegisterHandler() called");
    
    if (!handler)
    {
        log_w("Attempted to register null handler");
        return;
    }
    
    if (handlers_.size() >= MAX_HANDLERS)
    {
        log_e("Cannot register handler - maximum capacity reached (%d)", MAX_HANDLERS);
        return;
    }
    
    handlers_.push_back(handler);
    log_d("Registered handler (total: %d)", handlers_.size());
}

void InterruptManager::UnregisterHandler(std::shared_ptr<IHandler> handler)
{
    log_v("UnregisterHandler() called");
    
    if (!handler) return;
    
    auto it = std::find(handlers_.begin(), handlers_.end(), handler);
    if (it != handlers_.end())
    {
        handlers_.erase(it);
        log_d("Unregistered handler (remaining: %d)", handlers_.size());
    }
}

bool InterruptManager::HasActiveInterrupts() const
{
    for (size_t i = 0; i < interruptCount_; ++i)
    {
        if (interrupts_[i].active)
        {
            return true;
        }
    }
    return false;
}

size_t InterruptManager::GetInterruptCount() const
{
    return interruptCount_;
}

void InterruptManager::ExecuteEffect(const Interrupt& interrupt)
{
    log_v("ExecuteEffect() called for interrupt '%s' with effect %d", 
          interrupt.id, static_cast<int>(interrupt.effect));
    
    // Delegate to the private ExecuteByEffect method
    ExecuteByEffect(interrupt);
}

void InterruptManager::CheckRestoration()
{
    log_v("CheckRestoration() called");
    
    // Delegate to the private HandleRestoration method
    HandleRestoration();
}

// Private implementation methods

void InterruptManager::ExecuteInterrupt(Interrupt& interrupt)
{
    log_v("ExecuteInterrupt() called for: %s", interrupt.id ? interrupt.id : "unknown");
    ++totalExecutions_; // Track execution count
    
    // Handle queued interrupts by routing to QueuedHandler
    if (interrupt.source == InterruptSource::QUEUED && queuedHandler_)
    {
        if (queuedHandler_->QueueInterrupt(&interrupt))
        {
            log_d("Queued interrupt '%s' for deferred execution", interrupt.id);
        }
        else
        {
            log_w("Failed to queue interrupt '%s' - executing immediately", interrupt.id);
            // Fallback to immediate effect-based execution if queueing fails
            ExecuteByEffect(interrupt);
        }
        return;
    }
    
    // Handle immediate execution (polled interrupts) - now using effect-based routing
    log_d("Executing interrupt '%s' with effect %d", interrupt.id, static_cast<int>(interrupt.effect));
    ExecuteByEffect(interrupt);
}

void InterruptManager::ExecuteByEffect(const Interrupt& interrupt)
{
    log_v("ExecuteByEffect() called for interrupt '%s' with effect %d", 
          interrupt.id, static_cast<int>(interrupt.effect));
    
    switch (interrupt.effect)
    {
        case InterruptEffect::LOAD_PANEL:
            LoadPanelFromInterrupt(interrupt);
            CheckForRestoration(interrupt);
            break;
            
        case InterruptEffect::SET_THEME:
            ApplyThemeFromInterrupt(interrupt);
            break;
            
        case InterruptEffect::SET_PREFERENCE:
            ApplyPreferenceFromInterrupt(interrupt);
            break;
            
        case InterruptEffect::BUTTON_ACTION:
            ExecuteButtonAction(interrupt);
            break;
            
        default:
            log_w("Unknown interrupt effect: %d", static_cast<int>(interrupt.effect));
            // No fallback execution - all effects must be handled explicitly
            break;
    }
}

void InterruptManager::ProcessHandlers()
{
    log_v("ProcessHandlers() called");
    
    for (auto& handler : handlers_)
    {
        if (handler)
        {
            handler->Process();
        }
    }
}


Interrupt* InterruptManager::FindInterrupt(const char* id)
{
    if (!id) return nullptr;
    
    for (size_t i = 0; i < interruptCount_; ++i)
    {
        if (interrupts_[i].id && strcmp(interrupts_[i].id, id) == 0)
        {
            return &interrupts_[i];
        }
    }
    return nullptr;
}

bool InterruptManager::ShouldEvaluateInterrupt(const Interrupt& interrupt) const
{
    // Optimize evaluation frequency based on priority to reduce CPU usage
    unsigned long currentTime = millis();
    unsigned long timeSinceLastEvaluation = currentTime - interrupt.lastEvaluation;
    
    // Set evaluation intervals based on priority for CPU efficiency
    unsigned long minInterval = 0;
    
    switch (interrupt.priority)
    {
        case Priority::CRITICAL:
            minInterval = 10;  // Fast response for error conditions and security
            break;
        case Priority::IMPORTANT:
            minInterval = 25;  // Balanced response for user input and sensors
            break;
        case Priority::NORMAL:
            minInterval = 50;  // Slower response for background tasks and themes
            break;
    }
    
    // User input requires fast response for good UX
    if (interrupt.effect == InterruptEffect::BUTTON_ACTION)
    {
        minInterval = std::min(minInterval, 15UL); // Ensure responsive button handling
    }
    
    // UI changes can be evaluated less frequently to prevent flicker
    if (interrupt.effect == InterruptEffect::SET_THEME)
    {
        minInterval = std::max(minInterval, 100UL); // Prevent rapid theme switching
    }
    
    return timeSinceLastEvaluation >= minInterval;
}

void InterruptManager::UpdateLastEvaluation(Interrupt& interrupt)
{
    interrupt.lastEvaluation = millis();
}


// System monitoring and diagnostic methods
void InterruptManager::PrintSystemStatus() const
{
    log_i("=== Interrupt System Status ===");
    log_i("Total Registered Interrupts: %d/%d", interruptCount_, MAX_INTERRUPTS);
    log_i("Total Handlers: %d/%d", handlers_.size(), MAX_HANDLERS);
    log_i("Total Evaluations: %lu", totalEvaluations_);
    log_i("Total Executions: %lu", totalExecutions_);
    log_i("Last Evaluation Time: %lu ms", lastEvaluationTime_);
    
    log_i("--- Registered Interrupts ---");
    for (size_t i = 0; i < interruptCount_; ++i)
    {
        const Interrupt& interrupt = interrupts_[i];
        const char* priorityStr = interrupt.priority == Priority::CRITICAL ? "CRITICAL" :
                                 interrupt.priority == Priority::IMPORTANT ? "IMPORTANT" : "NORMAL";
        const char* sourceStr = interrupt.source == InterruptSource::POLLED ? "POLLED" : "QUEUED";
        const char* effectStr = interrupt.effect == InterruptEffect::LOAD_PANEL ? "LOAD_PANEL" :
                               interrupt.effect == InterruptEffect::SET_THEME ? "SET_THEME" :
                               interrupt.effect == InterruptEffect::SET_PREFERENCE ? "SET_PREFERENCE" : "BUTTON_ACTION";
        
        log_i("  [%d] %s: %s/%s/%s %s", 
              i, interrupt.id ? interrupt.id : "null", 
              priorityStr, sourceStr, effectStr,
              interrupt.active ? "ACTIVE" : "INACTIVE");
    }
    
    log_i("--- Handler Status ---");
    log_i("  Polled Handler: %s", polledHandler_ ? "REGISTERED" : "NOT REGISTERED");
    log_i("  Queued Handler: %s", queuedHandler_ ? "REGISTERED" : "NOT REGISTERED");
    log_i("  Total Handlers: %d", handlers_.size());
    
    log_i("================================");
}

size_t InterruptManager::GetRegisteredInterruptCount() const
{
    return interruptCount_;
}

void InterruptManager::GetInterruptStatistics(size_t& totalEvaluations, size_t& totalExecutions) const
{
    totalEvaluations = totalEvaluations_;
    totalExecutions = totalExecutions_;
}

// Memory management and performance optimization
void InterruptManager::OptimizeMemoryUsage()
{
    log_v("OptimizeMemoryUsage() called");
    
    // Remove inactive interrupts to free memory
    CompactInterruptArray();
    
    // Report optimization results for debugging
    log_d("Memory optimization complete - %d interrupts active", interruptCount_);
}

void InterruptManager::CompactInterruptArray()
{
    log_v("CompactInterruptArray() called");
    
    size_t writeIndex = 0;
    for (size_t readIndex = 0; readIndex < interruptCount_; ++readIndex)
    {
        // Only keep active interrupts with valid IDs
        if (interrupts_[readIndex].active && interrupts_[readIndex].id)
        {
            if (writeIndex != readIndex)
            {
                // Move interrupt to fill gap
                interrupts_[writeIndex] = interrupts_[readIndex];
                log_d("Moved interrupt %s from index %d to %d", 
                      interrupts_[writeIndex].id, readIndex, writeIndex);
            }
            ++writeIndex;
        }
        else
        {
            log_d("Removed inactive interrupt at index %d", readIndex);
        }
    }
    
    // Update count and clear remaining slots
    size_t oldCount = interruptCount_;
    interruptCount_ = writeIndex;
    
    // Clear unused slots
    for (size_t i = interruptCount_; i < oldCount; ++i)
    {
        memset(&interrupts_[i], 0, sizeof(Interrupt));
    }
    
    if (oldCount != interruptCount_)
    {
        log_i("Compacted interrupt array: %d -> %d interrupts", oldCount, interruptCount_);
    }
}

// Effect-specific execution methods
void InterruptManager::LoadPanelFromInterrupt(const Interrupt& interrupt)
{
    log_v("LoadPanelFromInterrupt() called for: %s", interrupt.id);
    
    if (!panelManager)
    {
        log_e("Cannot load panel - PanelManager is null");
        return;
    }
    
    // Determine panel name based on interrupt ID
    const char* panelName = nullptr;
    
    if (strcmp(interrupt.id, "key_present") == 0)
    {
        panelName = PanelNames::KEY;
    }
    else if (strcmp(interrupt.id, "key_not_present") == 0)
    {
        // Load key panel with red icon indication
        panelName = PanelNames::KEY;
    }
    else if (strcmp(interrupt.id, "lock_state") == 0)
    {
        // Only load lock panel if lock is actually engaged
        if (interrupt.context)
        {
            LockSensor* sensor = static_cast<LockSensor*>(interrupt.context);
            bool lockEngaged = std::get<bool>(sensor->GetReading());
            if (lockEngaged)
            {
                panelName = PanelNames::LOCK;
            }
            else
            {
                log_d("Lock disengaged - skipping panel load, allowing restoration logic to handle");
                return; // Don't load panel, let restoration handle it
            }
        }
    }
    else if (strcmp(interrupt.id, "error_occurred") == 0)
    {
        panelName = PanelNames::ERROR;
    }
    else
    {
        log_w("Unknown panel loading interrupt: %s", interrupt.id);
        return;
    }
    
    // === MEMORY CORRUPTION DETECTION IN INTERRUPT CONTEXT ===
    log_d("=== INTERRUPT MEMORY VALIDATION ===");
    log_d("Free heap before panel load: %d bytes", ESP.getFreeHeap());
    log_d("Largest free block: %d bytes", ESP.getMaxAllocHeap());
    
    // Validate panelManager pointer
    log_d("PanelManager validation:");
    log_d("  panelManager pointer: %p", panelManager.get());
    
    // Validate panel name string
    log_d("Panel name validation:");
    log_d("  panelName pointer: %p", panelName);
    log_d("  panelName string: '%s'", panelName ? panelName : "NULL");
    log_d("  panelName length: %d", panelName ? strlen(panelName) : 0);
    
    // Memory integrity check
    static const uint32_t INTERRUPT_PATTERN = 0xCAFEBABE;
    uint32_t interrupt_test = INTERRUPT_PATTERN;
    log_d("Interrupt context memory test: 0x%08X", interrupt_test);
    if (interrupt_test != INTERRUPT_PATTERN) {
        log_e("INTERRUPT MEMORY CORRUPTION: Pattern 0x%08X != 0x%08X!", interrupt_test, INTERRUPT_PATTERN);
    }
    
    // Check stack integrity (rough check)
    volatile uint32_t stack_marker = 0x12345678;
    log_d("Stack marker: 0x%08X at %p", stack_marker, &stack_marker);

    log_i("Loading panel '%s' triggered by interrupt '%s'", panelName, interrupt.id);
    
    // Call with memory tracking
    panelManager->CreateAndLoadPanel(panelName, true);  // Mark as trigger-driven
    
    log_d("=== POST-INTERRUPT PANEL LOAD ===");
    log_d("Free heap after panel load: %d bytes", ESP.getFreeHeap());
    
    // Verify stack marker wasn't corrupted
    if (stack_marker != 0x12345678) {
        log_e("STACK CORRUPTION: Stack marker changed from 0x12345678 to 0x%08X!", stack_marker);
    }
}

void InterruptManager::CheckForRestoration(const Interrupt& interrupt)
{
    log_v("CheckForRestoration() called for: %s", interrupt.id);
    
    // Check if this is a panel-loading interrupt that might trigger restoration
    if (interrupt.effect != InterruptEffect::LOAD_PANEL)
    {
        return; // Only panel-loading interrupts participate in restoration
    }
    
    // Delegate to centralized restoration logic
    HandleRestoration();
}

void InterruptManager::ApplyThemeFromInterrupt(const Interrupt& interrupt)
{
    log_v("ApplyThemeFromInterrupt() called for: %s", interrupt.id);
    
    if (!styleManager)
    {
        log_e("Cannot apply theme - StyleManager is null");
        return;
    }
    
    // Theme changes are determined by lights sensor context
    if (strcmp(interrupt.id, "lights_state") == 0)
    {
        // Get lights sensor from context and determine theme
        if (interrupt.context)
        {
            LightsSensor* sensor = static_cast<LightsSensor*>(interrupt.context);
            bool lightsOn = sensor->GetLightsState();
            const char* newTheme = lightsOn ? Themes::NIGHT : Themes::DAY;
            log_i("Lights %s - switching to %s theme", lightsOn ? "ON" : "OFF", newTheme);
            styleManager->SetTheme(newTheme);
        }
    }
}

void InterruptManager::ApplyPreferenceFromInterrupt(const Interrupt& interrupt)
{
    log_v("ApplyPreferenceFromInterrupt() called for: %s", interrupt.id);
    
    // TODO: Implement preference updates via interrupt system
    // This would handle configuration changes triggered by sensors or conditions
    log_d("Preference update for interrupt '%s' - placeholder implementation", interrupt.id);
}

void InterruptManager::ExecuteButtonAction(const Interrupt& interrupt)
{
    log_v("ExecuteButtonAction() called for: %s", interrupt.id);
    
    // Determine which button function to execute based on interrupt ID
    bool isShortPress = (interrupt.id && strcmp(interrupt.id, "universal_short_press") == 0);
    bool isLongPress = (interrupt.id && strcmp(interrupt.id, "universal_long_press") == 0);
    
    if (isShortPress && currentShortPressFunc_)
    {
        log_d("Executing injected short press function with panel context");
        currentShortPressFunc_(currentPanelContext_);
    }
    else if (isLongPress && currentLongPressFunc_)
    {
        log_d("Executing injected long press function with panel context");
        currentLongPressFunc_(currentPanelContext_);
    }
    else
    {
        log_w("No function injected for button interrupt '%s' (short: %p, long: %p)", 
              interrupt.id, 
              reinterpret_cast<void*>(currentShortPressFunc_), 
              reinterpret_cast<void*>(currentLongPressFunc_));
    }
}

void InterruptManager::HandleRestoration()
{
    log_v("HandleRestoration() called");
    
    if (!panelManager)
    {
        log_e("Cannot handle restoration - PanelManager is null");
        return;
    }
    
    // HYBRID ARCHITECTURE: Check for pending interrupts that were just processed
    // Instead of calling processFunc (which can be stateful), check if any interrupts
    // have stateChanged flag set, indicating they should be processed
    const Interrupt* highestPriorityActive = nullptr;
    int lowestPriorityValue = 3; // Lower number = higher priority (CRITICAL=0, IMPORTANT=1, NORMAL=2)
    
    for (size_t i = 0; i < interruptCount_; ++i)
    {
        const Interrupt& interrupt = interrupts_[i];
        
        // Only consider active panel-loading interrupts
        if (!interrupt.active || interrupt.effect != InterruptEffect::LOAD_PANEL)
            continue;
        
        // HYBRID ARCHITECTURE: Check if interrupt trigger condition is actually true
        // We need to verify the actual trigger state, not just stateChanged flag
        bool actuallyTriggered = false;
        
        // Check the actual trigger condition for each interrupt type
        if (strcmp(interrupt.id, "lock_state") == 0 && interrupt.context)
        {
            LockSensor* sensor = static_cast<LockSensor*>(interrupt.context);
            bool lockEngaged = std::get<bool>(sensor->GetReading());
            actuallyTriggered = lockEngaged; // Only triggered when lock is engaged
            log_i("HYBRID RESTORATION: Lock interrupt - engaged: %s, actuallyTriggered: %s", 
                  lockEngaged ? "true" : "false", actuallyTriggered ? "true" : "false");
        }
        else if (strcmp(interrupt.id, "key_present") == 0 && interrupt.context)
        {
            KeyPresentSensor* sensor = static_cast<KeyPresentSensor*>(interrupt.context);
            actuallyTriggered = sensor->GetKeyPresentState();
        }
        else if (strcmp(interrupt.id, "key_not_present") == 0 && interrupt.context)
        {
            KeyNotPresentSensor* sensor = static_cast<KeyNotPresentSensor*>(interrupt.context);
            actuallyTriggered = sensor->GetKeyNotPresentState();
        }
        else if (strcmp(interrupt.id, "error_occurred") == 0)
        {
            actuallyTriggered = ErrorManager::Instance().HasPendingErrors();
        }
        
        if (actuallyTriggered)
        {
            int priorityValue = static_cast<int>(interrupt.priority);
            if (priorityValue < lowestPriorityValue)
            {
                lowestPriorityValue = priorityValue;
                highestPriorityActive = &interrupt;
                log_i("HYBRID RESTORATION: Found active interrupt '%s' with priority %d", 
                      interrupt.id, priorityValue);
            }
        }
    }
    
    if (highestPriorityActive)
    {
        // There's an active panel-loading interrupt - ensure its panel is loaded
        const char* currentPanel = panelManager->GetCurrentPanel();
        const char* requiredPanel = nullptr;
        
        // Determine required panel based on interrupt ID
        if (strcmp(highestPriorityActive->id, "key_present") == 0)
            requiredPanel = PanelNames::KEY;
        else if (strcmp(highestPriorityActive->id, "key_not_present") == 0)
            requiredPanel = PanelNames::KEY;
        else if (strcmp(highestPriorityActive->id, "lock_state") == 0)
            requiredPanel = PanelNames::LOCK;
        else if (strcmp(highestPriorityActive->id, "error_occurred") == 0)
            requiredPanel = PanelNames::ERROR;
        
        if (requiredPanel && currentPanel && strcmp(currentPanel, requiredPanel) != 0)
        {
            log_i("Restoration: Loading priority panel '%s' for active interrupt '%s'", 
                  requiredPanel, highestPriorityActive->id);
            panelManager->CreateAndLoadPanel(requiredPanel, true);
        }
        else if (requiredPanel)
        {
            log_d("Restoration: Priority panel '%s' already loaded for interrupt '%s'", 
                  requiredPanel, highestPriorityActive->id);
        }
    }
    else
    {
        // No active panel-loading interrupts - restore to user's last panel
        const char* currentPanel = panelManager->GetCurrentPanel();
        const char* restorationPanel = panelManager->GetRestorationPanel();
        
        // Only restore if current panel is trigger-driven and different from restoration panel
        if (panelManager->IsCurrentPanelTriggerDriven() && 
            currentPanel && restorationPanel && 
            strcmp(currentPanel, restorationPanel) != 0)
        {
            log_i("Restoration: No active triggers - restoring to user panel '%s' from trigger panel '%s'", 
                  restorationPanel, currentPanel);
            panelManager->CreateAndLoadPanel(restorationPanel, true);
        }
        else
        {
            log_d("Restoration: No restoration needed - current panel '%s' is appropriate", 
                  currentPanel ? currentPanel : "null");
        }
    }
}

void InterruptManager::EvaluateAllInterrupts()
{
    log_v("EvaluateAllInterrupts() called");
    
    // Evaluate all registered interrupts
    for (size_t i = 0; i < interruptCount_; ++i)
    {
        Interrupt& interrupt = interrupts_[i];
        
        if (!interrupt.active || !interrupt.processFunc)
        {
            log_d("NEW ARCHITECTURE: Skipping interrupt '%s' - active:%s, processFunc:%p", 
                  interrupt.id ? interrupt.id : "null", 
                  interrupt.active ? "true" : "false",
                  interrupt.processFunc);
            continue;
        }
        
        log_d("NEW ARCHITECTURE: Evaluating interrupt '%s'", interrupt.id ? interrupt.id : "null");
            
        if (ShouldEvaluateInterrupt(interrupt))
        {
            log_d("NEW ARCHITECTURE: Should evaluate interrupt '%s' - proceeding", interrupt.id);
            // Evaluate once and cache result
            InterruptResult result = interrupt.processFunc(interrupt.context);
            interrupt.stateChanged = (result == InterruptResult::EXECUTE_EFFECT);
            interrupt.lastEvaluation = millis();
            
            log_d("NEW ARCHITECTURE: Interrupt '%s' result:%d, stateChanged:%s", 
                  interrupt.id, static_cast<int>(result), 
                  interrupt.stateChanged ? "true" : "false");
            
            if (interrupt.stateChanged)
            {
                log_i("NEW ARCHITECTURE: Interrupt '%s' state changed - marked for execution", interrupt.id);
            }
        }
        else
        {
            log_d("NEW ARCHITECTURE: Interrupt '%s' should not be evaluated yet", interrupt.id);
        }
    }
}

void InterruptManager::ExecuteInterruptsWithRules()
{
    log_i("NEW ARCHITECTURE: ExecuteInterruptsWithRules() called");
    
    // Sort interrupts by priority for execution order
    // Since we're using a static array, we'll iterate in priority order
    for (int priority = 0; priority <= 2; ++priority) // CRITICAL=0, IMPORTANT=1, NORMAL=2
    {
        for (size_t i = 0; i < interruptCount_; ++i)
        {
            Interrupt& interrupt = interrupts_[i];
            
            if (interrupt.stateChanged && 
                static_cast<int>(interrupt.priority) == priority &&
                CanExecute(interrupt))
            {
                log_i("NEW ARCHITECTURE: Executing interrupt '%s' (priority %d, mode %d)", 
                      interrupt.id, priority, static_cast<int>(interrupt.executionMode));
                      
                ExecuteInterrupt(interrupt);
                ++totalExecutions_;
                
                // Track exclusion group if applicable
                if (interrupt.executionMode == InterruptExecutionMode::EXCLUSIVE && 
                    interrupt.exclusionGroup)
                {
                    executedGroups_.push_back(interrupt.exclusionGroup);
                }
            }
        }
    }
}

bool InterruptManager::CanExecute(const Interrupt& interrupt) const
{
    log_d("NEW ARCHITECTURE: CanExecute() called for interrupt '%s'", interrupt.id);
    
    switch (interrupt.executionMode)
    {
        case InterruptExecutionMode::ALWAYS:
            log_i("NEW ARCHITECTURE: Interrupt '%s' has ALWAYS mode - can execute", interrupt.id);
            return true;
            
        case InterruptExecutionMode::EXCLUSIVE:
            if (interrupt.exclusionGroup && IsGroupExecuted(interrupt.exclusionGroup))
            {
                log_d("Interrupt '%s' blocked - exclusion group '%s' already executed", 
                      interrupt.id, interrupt.exclusionGroup);
                return false;
            }
            return true;
            
        case InterruptExecutionMode::CONDITIONAL:
            if (interrupt.canExecuteInContext)
            {
                bool canExecute = interrupt.canExecuteInContext(currentContext_);
                log_d("Interrupt '%s' conditional check returned: %s", 
                      interrupt.id, canExecute ? "true" : "false");
                return canExecute;
            }
            return true;
            
        default:
            return true;
    }
}

void InterruptManager::ClearStateChanges()
{
    log_v("ClearStateChanges() called");
    
    for (size_t i = 0; i < interruptCount_; ++i)
    {
        interrupts_[i].stateChanged = false;
    }
}

bool InterruptManager::IsGroupExecuted(const char* group) const
{
    if (!group) return false;
    
    for (const auto& executedGroup : executedGroups_)
    {
        if (executedGroup && strcmp(executedGroup, group) == 0)
        {
            return true;
        }
    }
    return false;
}

