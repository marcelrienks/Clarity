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
#include "sensors/button_sensor.h"
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
        UpdateInterruptContext("universal_short_press", queuedHandler_->GetButtonSensor());
        UpdateInterruptContext("universal_long_press", queuedHandler_->GetButtonSensor());
        log_d("Updated queued interrupt contexts with ButtonSensor pointer");
    }
}


// Core interrupt processing - clean separation of evaluation and execution
void InterruptManager::Process()
{
    log_v("InterruptManager::Process() called");
    
    if (!initialized_)
    {
        return;
    }

    // Phase 1: Evaluate all interrupts (check for state changes)
    EvaluateInterrupts();
    
    // Phase 2: Execute interrupts that need action (only during idle)
    if (IsUIIdle()) {
        ExecuteInterrupts();
    }
    
    log_v("InterruptManager::Process() completed");
}

void InterruptManager::EvaluateInterrupts()
{
    log_v("EvaluateInterrupts() - checking all interrupt conditions");
    
    // Always evaluate queued interrupts (button presses)
    EvaluateQueuedInterrupts();
    
    // Evaluate polled interrupts only during idle
    if (IsUIIdle()) {
        EvaluatePolledInterrupts();
    }
    
    totalEvaluations_++;
}

void InterruptManager::EvaluateQueuedInterrupts()
{
    log_v("EvaluateQueuedInterrupts() - checking button interrupts");
    
    for (size_t i = 0; i < interruptCount_; i++)
    {
        Interrupt& interrupt = interrupts_[i];
        
        // Only process QUEUED interrupts that are active
        if (interrupt.source != InterruptSource::QUEUED || !interrupt.IsActive())
            continue;
        
        // Call evaluation function to check if state changed
        if (interrupt.evaluationFunc && interrupt.evaluationFunc(interrupt.context))
        {
            interrupt.SetNeedsExecution(true);
            log_d("Queued interrupt '%s' needs execution", interrupt.id);
        }
    }
}

void InterruptManager::EvaluatePolledInterrupts()
{
    log_v("EvaluatePolledInterrupts() - checking GPIO interrupts");
    
    for (size_t i = 0; i < interruptCount_; i++)
    {
        Interrupt& interrupt = interrupts_[i];
        
        // Only process POLLED interrupts that are active
        if (interrupt.source != InterruptSource::POLLED || !interrupt.IsActive())
            continue;
        
        // Call evaluation function to check if state changed
        if (interrupt.evaluationFunc && interrupt.evaluationFunc(interrupt.context))
        {
            interrupt.SetNeedsExecution(true);
            log_d("Polled interrupt '%s' needs execution", interrupt.id);
        }
    }
}

void InterruptManager::ExecuteInterrupts()
{
    log_v("ExecuteInterrupts() - executing pending interrupts");
    
    // Execute polled interrupts first (higher priority)
    ExecutePolledInterrupts();
    
    // Then execute queued interrupts
    ExecuteQueuedInterrupts();
    
    // Check for panel restoration after all executions
    HandleRestoration();
}

void InterruptManager::ExecutePolledInterrupts()
{
    log_v("ExecutePolledInterrupts() - executing GPIO-based interrupts");
    
    for (size_t i = 0; i < interruptCount_; i++)
    {
        Interrupt& interrupt = interrupts_[i];
        
        // Only execute POLLED interrupts that need execution
        if (interrupt.source != InterruptSource::POLLED || !interrupt.NeedsExecution())
            continue;
        
        // Execute the interrupt
        if (interrupt.executionFunc)
        {
            log_d("Executing polled interrupt '%s'", interrupt.id);
            ExecuteByEffect(interrupt);
            interrupt.SetNeedsExecution(false);
            totalExecutions_++;
        }
    }
}

void InterruptManager::ExecuteQueuedInterrupts()
{
    log_v("ExecuteQueuedInterrupts() - executing button interrupts");
    
    for (size_t i = 0; i < interruptCount_; i++)
    {
        Interrupt& interrupt = interrupts_[i];
        
        // Only execute QUEUED interrupts that need execution
        if (interrupt.source != InterruptSource::QUEUED || !interrupt.NeedsExecution())
            continue;
        
        // Execute the interrupt
        if (interrupt.executionFunc)
        {
            log_d("Executing queued interrupt '%s'", interrupt.id);
            ExecuteByEffect(interrupt);
            interrupt.SetNeedsExecution(false);
            totalExecutions_++;
        }
    }
}

bool InterruptManager::IsUIIdle() const
{
    // Check if UI is idle - for now always return true since we're called from LVGL idle
    // In future, this could check LVGL task queue or animation state
    return true;
}

bool InterruptManager::RegisterInterrupt(const Interrupt& interrupt)
{
    log_v("RegisterInterrupt() called for interrupt: %s", interrupt.id ? interrupt.id : "null");
    
    if (!interrupt.id || !interrupt.evaluationFunc || !interrupt.executionFunc)
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
    interrupts_[interruptCount_].SetActive(true);
    interrupts_[interruptCount_].SetNeedsExecution(false);
    
    interruptCount_++;
    
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
        interrupt->SetActive(true);
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
        interrupt->SetActive(false);
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

void InterruptManager::UpdateInterruptEvaluation(const char* id, bool (*evaluationFunc)(void*))
{
    log_v("UpdateInterruptEvaluation() called for: %s", id ? id : "null");
    
    Interrupt* interrupt = FindInterrupt(id);
    if (interrupt)
    {
        interrupt->evaluationFunc = evaluationFunc;
        log_d("Updated evaluation function for interrupt '%s'", id);
    }
    else
    {
        log_w("Interrupt '%s' not found for evaluation update", id ? id : "null");
    }
}

void InterruptManager::UpdateInterruptExecution(const char* id, void (*executionFunc)(void*))
{
    log_v("UpdateInterruptExecution() called for: %s", id ? id : "null");
    
    Interrupt* interrupt = FindInterrupt(id);
    if (interrupt)
    {
        interrupt->executionFunc = executionFunc;
        log_d("Updated execution function for interrupt '%s'", id);
    }
    else
    {
        log_w("Interrupt '%s' not found for execution update", id ? id : "null");
    }
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
        if (interrupts_[i].IsActive())
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

void InterruptManager::ExecuteByEffect(const Interrupt& interrupt)
{
    log_v("ExecuteByEffect() called for interrupt '%s' with effect %d", 
          interrupt.id, static_cast<int>(interrupt.effect));
    
    switch (interrupt.effect)
    {
        case InterruptEffect::LOAD_PANEL:
            LoadPanelFromInterrupt(interrupt);
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
              interrupt.IsActive() ? "ACTIVE" : "INACTIVE");
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
        if (interrupts_[readIndex].IsActive() && interrupts_[readIndex].id)
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
    
    // Simple restoration: Check if any restoration-triggering interrupts are active
    bool hasActiveRestorationTrigger = false;
    
    for (size_t i = 0; i < interruptCount_; ++i)
    {
        const Interrupt& interrupt = interrupts_[i];
        
        // Only check panel-loading interrupts that track restoration
        if (!interrupt.IsActive() || interrupt.effect != InterruptEffect::LOAD_PANEL)
            continue;
            
        if (!interrupt.data.panel.trackForRestore)
            continue;
        
        // If we have an active restoration-tracking interrupt, no restoration needed
        hasActiveRestorationTrigger = true;
        log_d("Active restoration trigger found: %s", interrupt.id);
        break;
    }
    
    // If no restoration triggers are active and we're on a trigger panel, restore
    if (!hasActiveRestorationTrigger && panelManager->IsCurrentPanelTriggerDriven())
    {
        log_i("No active restoration triggers - restoring to previous panel");
        // TODO: Implement panel restoration logic
        log_d("Panel restoration needed but method not yet implemented");
    }
}

