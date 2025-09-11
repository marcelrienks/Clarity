# log_t() Function Implementation

## Overview
Successfully implemented a custom `log_t()` function that provides timing/performance logging with a clean `[T]` prefix, replicating the behavior of the built-in ESP32 logging functions like `log_i()`.

## Implementation Details

### Files Created/Modified
1. **Created**: `/include/utilities/logging.h` - New logging utilities header
2. **Modified**: `/src/main.cpp` - Added include and test usage

### Function Signature
```cpp
#define log_t(format, ...) printf("[T] " format "\n", ##__VA_ARGS__)
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
```
[T] Panel loaded successfully
[T] Operation completed in 150 ms
[T] Free heap: 180432 bytes
[T] Temperature: 85°C, Pressure: 3 Bar
```

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

It follows the project's logging standards from `docs/patterns.md` and can be used for:
- Timing measurements
- Performance monitoring
- Optimization tracking
- Memory usage logging

## Test Status
✅ **Build**: Successfully compiles with `pio run -e debug-local`
✅ **Integration**: Properly integrated with existing codebase
✅ **Usage**: Test implementations added to `main.cpp` initialization

The `log_t()` function is now ready for use throughout the Clarity project for timing and performance logging needs.