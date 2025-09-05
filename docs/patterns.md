# Development Patterns

## MVP Architecture
Model-View-Presenter pattern with clear separation of concerns.

## State Management
Each panel manages BUSY or IDLE state due to proximity to LVGL and Timer logic.

## Error Handling
Early validation and return patterns with comprehensive error handling.

## Logging Standards

### Log Level Guidelines
- **Error logs (log_e)**: Critical errors that affect system operation
- **Warning logs (log_w)**: Important issues that should be addressed
- **Info logs (log_i)**: Major system actions and state changes
- **Debug logs (log_d)**: Development debugging (should be cleaned up after use)
- **Verbose logs (log_v)**: Method entry indication only

### Implementation Rules
1. **Verbose logs**: Only for method entry in methods with significant logic or control flow
   - Format: `log_v("MethodName() called")`
   - Should be the only location for verbose logs in the system
   
2. **Info logs**: For major system actions, state changes, and important events
   - Panel loading/transitions
   - Interrupt attachment/detachment  
   - System initialization milestones
   - Sensor state changes
   
3. **Debug logs**: Only during active development for issue solving
   - Should be removed after debugging is complete
   - Not permanent parts of the codebase
   
4. **Avoid**: Repetitive logs in frequently called methods (GPIO reads, display flushes, etc.)

### Examples
```cpp
// Good - Verbose entry log for significant method
void PanelManager::CreateAndLoadPanel(const char* panelName) {
    log_v("CreateAndLoadPanel() called");
    log_i("Panel transition requested: %s", panelName);
    // ... implementation
}

// Good - Info log for major action
void GpioProvider::AttachInterrupt(int pin, void (*callback)(), int mode) {
    log_v("AttachInterrupt() called");
    log_i("GPIO %d interrupt attached", pin);
    // ... implementation  
}

// Avoid - No verbose logs in simple accessors
bool GpioProvider::DigitalRead(int pin) {
    return ::digitalRead(pin);  // No logging needed
}
```