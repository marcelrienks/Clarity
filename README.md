[![Wokwi Basic Hardware Test](https://github.com/marcelrienks/Clarity/actions/workflows/wokwi-basic-test.yml/badge.svg)](https://github.com/marcelrienks/Clarity/actions/workflows/wokwi-basic-test.yml)
[![Wokwi Full System Test](https://github.com/marcelrienks/Clarity/actions/workflows/wokwi-full-test.yml/badge.svg)](https://github.com/marcelrienks/Clarity/actions/workflows/wokwi-full-test.yml)

# Clarity Digital Gauge System

**A sophisticated ESP32 automotive gauge system featuring advanced interrupt architecture, LVGL UI on round displays, and comprehensive optimization for production deployment.**

## 🚀 **System Highlights**

- **🏭 Professional Architecture**: MVP pattern with factory-based dependency injection, optimized for embedded systems
- **⚡ Advanced Interrupt System**: TriggerHandler/ActionHandler separation with priority-based execution
- **🎯 Production Optimized**: 30.7KB flash reduction, 40-60% performance improvement across critical paths
- **🎨 Modern UI**: LVGL-based interface designed for 240x240 round displays
- **🔧 Comprehensive Testing**: Automated CI/CD with Wokwi simulation and integration testing
- **📱 Multiple Panels**: Oil monitoring, error handling, configuration, and trigger-driven displays

---

## 📚 **Complete Documentation**

### **🏗️ Core Architecture**
- **[Architecture Overview](docs/architecture.md)** - High-level MVP pattern and system design
- **[Interrupt Architecture](docs/interrupts.md)** - Advanced TriggerHandler/ActionHandler system with priority-based execution
- **[Sensor Architecture](docs/sensor.md)** - Single ownership model with BaseSensor change detection pattern
- **[Development Patterns](docs/patterns.md)** - Design patterns, string memory management, and coding practices

### **🔧 Implementation Details**
- **[Hardware Configuration](docs/hardware.md)** - ESP32 setup, GPIO mapping, input system architecture, and button behaviors
- **[System Requirements](docs/requirements.md)** - Comprehensive specifications with actual optimization results (30.7KB saved)
- **[Configuration System](docs/config.md)** - Dynamic self-registering configuration with IConfig interface and alphabetical organization
- **[Error Handling](docs/error.md)** - Error management, reporting, and panel integration
- **[Performance Logging](docs/log_t.md)** - Custom timing measurement system for optimization

### **🧪 Development & Testing**
- **[Testing Guide](docs/test.md)** - Complete Wokwi simulation setup, GitHub Actions CI/CD, and debugging procedures
- **[Coding Standards](docs/standards.md)** - Naming conventions, file organization, and code quality guidelines
- **[Usage Scenarios](docs/scenarios.md)** - User workflows, panel behaviors, and system interactions
- **[TODO List](docs/todo.md)** - Remaining tasks and future improvements

### **📊 System Diagrams**
- **[Architecture Overview](docs/diagrams/architecture-overview.md)** - Complete component relationships and dependencies
- **[Application Flow](docs/diagrams/application-flow.md)** - Startup sequence, runtime processing, and interrupt handling
- **[Interrupt Handling Flow](docs/diagrams/interrupt-handling-flow.md)** - Detailed trigger/action processing workflow

### **⚙️ CI/CD & Workflows**
- **[GitHub Actions Setup](.github/workflows/README.md)** - Automated testing configuration with Wokwi integration

---

## 🚀 **Quick Start**

### **Build Commands**
```bash
# Development builds
pio run -e debug-local     # Fast compilation for local testing
pio run -e test-wokwi      # Clean integration testing build

# Hardware deployment
pio run -e debug-upload    # Debug build with display inversion
pio run -e release         # Optimized production build
```

### **Testing & Simulation**
```bash
# Interactive manual testing with full verbose logs
cd test/manual && ./wokwi_debug.sh

# Automated integration testing with clean logs
cd test/wokwi && ./wokwi_run.sh
```

---

## 🎯 **System Architecture**

### **MVP Pattern Implementation**
- **Models**: Sensors (GPIO/ADC data acquisition with change detection)
- **Views**: Components (LVGL UI rendering - display-only for interrupt-driven panels)
- **Presenters**: Panels (business logic orchestration and lifecycle management)

### **Advanced Interrupt Architecture**
```
InterruptManager (Central Coordinator)
├── TriggerHandler (GPIO State Monitoring)
│   ├── KeyPresentSensor (GPIO 25)
│   ├── KeyNotPresentSensor (GPIO 26)
│   ├── LockSensor (GPIO 27)
│   └── LightsSensor (GPIO 33)
└── ActionHandler (Button Event Processing)
    └── ButtonSensor (GPIO 32)
```

### **Key Features**
- **Priority-Based Execution**: CRITICAL > IMPORTANT > NORMAL with blocking logic
- **Type-Based Restoration**: Panel/Style trigger restoration on deactivation
- **LVGL Compatibility**: UI operations only during IDLE state
- **Memory Optimized**: Static arrays, zero hot path allocations

---

## 🛠️ **Hardware Configuration**

### **Target Hardware**
- **Microcontroller**: NodeMCU-32S (ESP32-WROOM-32, 4MB Flash, 320KB RAM)
- **Display**: Waveshare 1.28" Round LCD (GC9A01, 240x240, SPI interface)
- **Sensors**: Oil pressure/temperature (ADC), GPIO triggers (Key, Lock, Lights)
- **Input**: Single button navigation (GPIO 32) with short/long press detection

### **Panel System**
- **Oil Panel**: Primary monitoring with dual gauges and continuous sensor readings
- **Key/Lock Panels**: Trigger-driven security status displays
- **Error Panel**: System error management with cycling and clearing
- **Config Panel**: Settings management with persistent storage
- **Splash Panel**: Startup animation with skip functionality

---

## 📈 **Performance Optimization Results**

**✅ All 10 Optimization Steps Completed**

### **Flash Memory Savings: 30.7KB Total**
- **Error Handling**: 15KB (largest - fixed-size buffers replace dynamic strings)
- **Hardware Providers**: 3KB (GPIO hot path optimization)
- **Component System**: 2.5KB (removed hot path logging)
- **Sensor Architecture**: 2.1KB (eliminated double ADC reads)
- **Panel System**: 1.8KB (animation state optimization)
- **Other optimizations**: 6.3KB combined

### **Performance Improvements**
- **Main Loop**: ~25% cycle time reduction (removed critical path logging)
- **Interrupt Processing**: ~40% improvement (cached UI idle checks)
- **Sensor Reading**: ~60% speed increase (fixed double reads)
- **Memory Safety**: Zero hot path allocations, heap fragmentation prevention

---

## 🧪 **Testing & Quality Assurance**

### **Automated CI/CD Pipeline**
- **Basic Hardware Test**: GPIO, sensors, timing validation (5 test phases)
- **Full System Test**: Complete integration with panels, triggers, themes (7 test phases)
- **Wokwi Simulation**: Hardware emulation with automated pass/fail validation
- **Performance Monitoring**: Custom `log_t()` timing measurements throughout

### **Test Coverage**
- **Hardware Integration**: GPIO interrupt handling, ADC readings, button debouncing
- **System Integration**: Panel switching, trigger priorities, error handling
- **Performance Testing**: Memory usage validation, timing benchmarks
- **Regression Testing**: Automated on every commit and pull request

---

## 🔧 **Technical Specifications**

### **Memory Architecture**
- **Available RAM**: 320KB (250KB after system overhead)
- **LVGL Buffers**: 120KB dual-buffer configuration optimized for smooth rendering
- **Static Allocation**: All critical structures use fixed-size arrays for predictable memory usage
- **Zero Fragmentation**: Eliminated dynamic string allocations in hot paths

### **Development Standards**
- **Google C++ Style Guide**: With project-specific adaptations for embedded systems
- **Interface-Based Design**: Comprehensive abstraction layers for testability
- **RAII Compliance**: Automatic resource management throughout
- **Const Correctness**: Type safety and optimization through proper const usage

---

## 📦 **Dependencies**

### **Core Libraries**
- **[Arduino Framework](https://github.com/espressif/arduino-esp32)** - ESP32 hardware abstraction
- **[LVGL 9.3.0](https://docs.lvgl.io/master/)** - Advanced graphics library optimized for embedded
- **[LovyanGFX 1.2.7](https://github.com/lovyan03/LovyanGFX)** - High-performance display driver
- **[ArduinoJson 7.4.2](https://arduinojson.org/)** - Efficient JSON parsing for configuration
- **[Preferences 2.0.0](https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences)** - ESP32 NVS storage wrapper

### **Development Tools**
- **[PlatformIO](https://platformio.org/)** - Cross-platform build system
- **[Wokwi](https://wokwi.com/)** - ESP32 hardware simulation platform
- **[GitHub Actions](https://github.com/features/actions)** - Automated CI/CD pipeline

---

## 🏆 **Credits & Inspiration**

### **Commercial Inspiration**
- **[Rotarytronics](https://www.rotarytronics.com/)** - Professional automotive gauge products

### **Technical Foundations**
- **[Garage Tinkering](https://github.com/garagetinkering)** - ESP32 automotive applications
- **[fbiego](https://github.com/fbiego)** - LVGL implementation patterns

### **Development Contributions**
- **[Eugene Petersen](https://github.com/gino247)** - Code review and architectural feedback

---

## 📝 **Project Philosophy**

_**Note:** This project represents a comprehensive exploration of professional software architecture patterns applied to embedded systems. While deliberately over-engineered for simple gauge applications, it demonstrates sophisticated design patterns, extensive testing frameworks, and production-ready optimization techniques. The result is a highly maintainable, testable, and performant embedded system that serves as an excellent foundation for complex automotive instrumentation projects._

**Key Achievements:**
- ✅ **Professional Architecture** - Factory patterns, dependency injection, interface-based design
- ✅ **Production Optimization** - 30.7KB flash savings, 40-60% performance improvements
- ✅ **Comprehensive Testing** - Automated CI/CD, hardware simulation, performance monitoring
- ✅ **Extensive Documentation** - 14 documentation files covering every system aspect
- ✅ **Embedded Best Practices** - Memory safety, static allocation, zero fragmentation patterns