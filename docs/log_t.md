# log_t() Function Implementation

## Overview
Successfully implemented a custom `log_t()` function that provides timing/performance logging with a clean `[T]` prefix, replicating the behavior of the built-in ESP32 logging functions like `log_i()`.

## Implementation Details

### Files Created/Modified
1. **Created**: `/include/utilities/logging.h` - New logging utilities header
2. **Created**: `/src/utilities/logging.cpp` - Implementation with duplicate suppression
3. **Modified**: `/src/main.cpp` - Added include and test usage

### Function Signature
```cpp
// Header definition (controlled by TEST_LOGS flag)
#ifdef TEST_LOGS
    void log_t_impl(const char* format, ...);
    #define log_t(format, ...) log_t_impl(format, ##__VA_ARGS__)
#else
    #define log_t(format, ...) ((void)0)
#endif
```

### Output Format
- **Standard**: `[T] Your message here`
- **With variables**: `[T] Operation completed in 150 ms`

### Features
- ✅ **Clean prefix**: Shows only `[T]` (not `[I][T]`)
- ✅ **Same syntax**: Works exactly like `log_i()`, `log_d()`, etc.
- ✅ **Printf formatting**: Full support for format strings and variables
- ✅ **ESP32 compatible**: Works with Arduino framework and ESP32 platform
- ✅ **Build tested**: Successfully compiles with PlatformIO
- ✅ **Duplicate suppression**: Intelligent handling of repetitive messages
- ✅ **Build flag control**: Enabled/disabled via TEST_LOGS compiler flag
- ✅ **Independent logging**: Bypasses CORE_DEBUG_LEVEL for clean test output

## Usage Examples

```cpp
// Include the header
#include "utilities/logging.h"

// Basic usage
log_t("Panel loaded successfully");

// With variables
log_t("Operation completed in %d ms", duration);
log_t("Free heap: %d bytes", ESP.getFreeHeap());
log_t("Temperature: %d°C, Pressure: %d Bar", temp, pressure);
```

## Expected Output

### Normal Output
```
[T] Panel loaded successfully
[T] Operation completed in 150 ms
[T] Free heap: 180432 bytes
[T] Temperature: 85°C, Pressure: 3 Bar
```

### Duplicate Suppression Behavior
```
[T] Sensor reading: 25°C
[T] Sensor reading: 25°C (x25)    // Every 25th duplicate shown with count
[T] Sensor reading: 25°C (x50)    // Every 25th duplicate continues
[T] (total x73)                   // Final count when new message arrives
[T] Sensor reading: 26°C          // New message resets counter
```

## Duplicate Suppression Feature

The `log_t()` function includes intelligent duplicate message suppression to prevent log spam while maintaining visibility into repetitive events:

### How It Works
- **Silent suppression**: Duplicate messages are suppressed without output
- **Periodic updates**: Every 25th duplicate is shown with a count `(x25, x50, etc.)`
- **Final summary**: When a new message arrives, shows final count if suppression occurred
- **Reset behavior**: Counter resets when any new message is encountered

### Benefits
- **Cleaner logs**: Reduces noise from repetitive sensor readings or state checks
- **Performance**: Avoids overwhelming the serial output buffer
- **Test automation**: Makes automated test parsing more reliable
- **Debug visibility**: Still shows that repetitive events are occurring

### Implementation Details
- Tracks last message in a 256-byte static buffer
- Uses `strcmp()` for exact message matching
- Thread-safe for single-core ESP32 execution model
- Memory footprint: ~260 bytes of static RAM

### Use Cases
Perfect for:
- Sensor readings that don't change frequently
- State machine transitions that repeat
- Loop iteration counters
- Periodic health checks

## Alternative Options

The header also includes an optional timestamp version (currently commented out):
```cpp
// Option 2: With ESP32-style timestamp (uncomment to use instead)
// #define log_t(format, ...) printf("[%7u][T] " format "\n", (unsigned)millis(), ##__VA_ARGS__)
```

Which would output:
```
[   5432][T] Panel loaded successfully
[   5582][T] Operation completed in 150 ms
```

## Integration

The function is now available throughout the Clarity codebase by including:
```cpp
#include "utilities/logging.h"
```

### Build Configuration Control
The `log_t()` function is controlled by the `TEST_LOGS` compiler flag:

- **debug-local**: `CORE_DEBUG_LEVEL=5` + `TEST_LOGS=1` (full logging)
- **test-wokwi**: `CORE_DEBUG_LEVEL=0` + `TEST_LOGS=1` (test logs only, clean output)
- **release**: `CORE_DEBUG_LEVEL=0` (no logs, minimal footprint)

This allows `log_t()` to bypass the standard ESP32 log level system and provide clean test output independent of debug verbosity.

### Usage Guidelines
It follows the project's logging standards from `docs/patterns.md` and can be used for:
- Timing measurements
- Performance monitoring
- Optimization tracking
- Memory usage logging
- State transitions
- Test automation markers

## Test Status
✅ **Build**: Successfully compiles with `pio run -e debug-local`
✅ **Integration**: Properly integrated with existing codebase
✅ **Usage**: Test implementations added to `main.cpp` initialization
✅ **Duplicate suppression**: Active in production code to prevent log spam
✅ **Build environments**: Tested across debug-local, test-wokwi, and release builds

The `log_t()` function is now ready for use throughout the Clarity project for timing and performance logging needs, with intelligent duplicate suppression to maintain clean, readable logs during extended operation and testing.