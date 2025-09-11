# Clarity Testing Guide

This directory contains two testing environments for the Clarity automotive gauge system, each optimized for different use cases.

## Testing Environments Overview

| Environment | Purpose | Log Level | Location | Use Case |
|-------------|---------|-----------|----------|----------|
| **Integration Testing** | Automated CI/CD testing | INFO only | `test/wokwi/` | Automated test validation |
| **Manual Debug Testing** | Interactive development | VERBOSE (all levels) | `test/manual/` | Manual debugging & development |

---

## 🔧 Manual Debug Testing (`test/manual/`)

**Purpose:** Interactive manual testing with full verbose logging for development and debugging.

### Features:
- ✅ **Full verbose logging** - See all `[V]`, `[D]`, `[I]`, `[W]`, `[E]`, and `[T]` messages
- ✅ **No timeout by default** - Manual control over testing session
- ✅ **Interactive hardware simulation** - Full control over buttons, switches, and potentiometers
- ✅ **Complete debug information** - Perfect for troubleshooting and development

### Quick Start:
```bash
# Navigate to manual testing directory
cd test/manual

# Start manual debug session (no timeout)
./wokwi_debug.sh

# Or with a timeout (optional)
TIMEOUT=300000 ./wokwi_debug.sh
```

### Build Environment:
- **PlatformIO Environment:** `debug-local`
- **Log Level:** `CORE_DEBUG_LEVEL=5` (VERBOSE)
- **Firmware Path:** `.pio/build/debug-local/firmware.bin`

### Hardware Controls (Wokwi Browser Interface):
- **Action Button (btn1):** GPIO 32 - Short/long press for panel navigation
- **Debug Button (btn2):** GPIO 34 - Trigger debug errors  
- **Pressure Potentiometer (pot1):** GPIO 36 - Simulate oil pressure (0-4095)
- **Temperature Potentiometer (pot2):** GPIO 39 - Simulate oil temperature (0-4095)
- **DIP Switch (sw1):** 4-position trigger control
  - **Position 1:** Key Present (GPIO 25)
  - **Position 2:** Key Not Present (GPIO 26) 
  - **Position 3:** Lock Engaged (GPIO 27)
  - **Position 4:** Lights On/Night Mode (GPIO 33)

### Expected Output:
```
[V] Verbose debug messages
[D] Debug information
[I] General information
[W] Warnings
[E] Errors
[T] Timing/performance messages ← Your custom log_t() function
```

---

## 🤖 Integration Testing (`test/wokwi/`)

**Purpose:** Automated testing for CI/CD pipelines with clean, filtered output.

### Features:
- ✅ **Clean log output** - Only `[I]`, `[W]`, `[E]`, and `[T]` messages
- ✅ **Automated test patterns** - Validates specific functionality sequences
- ✅ **5-minute timeout** - Prevents hanging in CI environments
- ✅ **Test phase tracking** - Progress monitoring with pass/fail detection

### Quick Start:
```bash
# Navigate to integration testing directory  
cd test/wokwi

# Run simple shell-based integration test
./wokwi_run.sh

# Run advanced Python-based test with progress tracking
python3 integration_test.py
```

### Build Environment:
- **PlatformIO Environment:** `test-wokwi`
- **Log Level:** `CORE_DEBUG_LEVEL=3` (INFO and above only)
- **Firmware Path:** `.pio/build/test-wokwi/firmware.bin`

### Test Phases Validated:
1. **System Startup** - Panel loading and initialization
2. **Sensor Interaction** - Hardware input simulation  
3. **Theme & Trigger System** - Priority-based panel switching
4. **Error System Integration** - Error handling and navigation
5. **Configuration System** - Settings and preferences

### Expected Output (Filtered):
```
[I] Standard information messages
[W] Warning messages  
[E] Error messages
[T] Timing/performance messages ← Your custom log_t() function
```

---

## 🛠️ Development Workflow

### For Feature Development:
1. **Code Changes** → Build with `debug-local`
2. **Manual Testing** → Use `test/manual/wokwi_debug.sh` 
3. **Interactive Debugging** → Full verbose output + manual hardware control
4. **Fix Issues** → Repeat until stable

### For CI/CD Integration:
1. **Code Complete** → Build with `test-wokwi`
2. **Integration Testing** → Use `test/wokwi/wokwi_run.sh`
3. **Automated Validation** → Clean output + automated pass/fail
4. **Deploy** → If all tests pass

---

## 📋 Build Commands

### Manual Debug Environment:
```bash
# Build for manual testing (verbose logging)
pio run -e debug-local

# Build and start manual debug session
cd test/manual && ./wokwi_debug.sh
```

### Integration Test Environment:  
```bash
# Build for integration testing (clean logging)
pio run -e test-wokwi

# Build and run integration tests
cd test/wokwi && ./wokwi_run.sh
```

---

## 🧪 Testing the log_t() Function

Both environments now support your custom `log_t()` function. You should see messages like:

```
[T] SplashPanel loaded successfully
[T] OemOilPanel loaded successfully  
[T] KeyPresentActivate() - Loading KEY panel
[T] LightsOnActivate() - Setting NIGHT theme
[T] Service initialization completed in 89 ms
```

These `[T]` messages are perfect for:
- ⏱️ **Performance measurement** - Timing critical operations
- 🔍 **Optimization tracking** - Before/after performance comparisons  
- 🏷️ **Filtered logging** - Easy to grep/filter for timing data
- 📊 **Metrics collection** - Automated performance monitoring

---

## 🚨 Troubleshooting

### Manual Testing Issues:
- **No wokwi-cli found:** Install from [Wokwi CLI releases](https://github.com/wokwi/wokwi-cli/releases)
- **Firmware not found:** Run `pio run -e debug-local` from project root
- **Simulation won't start:** Check that `diagram.json` and `wokwi.toml` exist

### Integration Testing Issues:
- **Test timeout:** Increase timeout with `TIMEOUT=600000 ./wokwi_run.sh`
- **Pattern mismatch:** Check that test patterns match actual `[T]` output
- **Build failure:** Ensure `test-wokwi` environment builds successfully

### General Issues:
- **WSL Path Issues:** Ensure you're using WSL paths (`/mnt/c/...`)
- **Permission Denied:** Run `chmod +x *.sh` to make scripts executable
- **Port Conflicts:** Only run one Wokwi session at a time

---

## 📁 Directory Structure

```
test/
├── README.md                    # This guide
├── manual/                      # Manual debug testing
│   ├── wokwi.toml              # Debug-local firmware config
│   ├── diagram.json            # Hardware simulation setup
│   └── wokwi_debug.sh          # Manual testing script
└── wokwi/                      # Integration testing  
    ├── wokwi.toml              # Test-wokwi firmware config
    ├── diagram.json            # Hardware simulation setup
    ├── wokwi_run.sh            # Simple integration test
    ├── integration_test.py     # Advanced test runner
    └── integration-test.yaml   # Test specification
```

Both environments are now fully configured and ready to use! 🎉