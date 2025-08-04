#pragma once

// Mock Arduino definitions for unit testing
#ifdef UNIT_TESTING

#include <stdint.h>
#include <cstddef>
#include <map>
#include <functional>
#include <string>

// Basic Arduino constants
#define HIGH 1
#define LOW  0

#define INPUT             0x0
#define OUTPUT            0x1
#define INPUT_PULLUP      0x2
#define INPUT_PULLDOWN    0x3

// Interrupt trigger modes
#define RISING   0x1
#define FALLING  0x2
#define CHANGE   0x3

// ADC pin constants
#define A0  0

// ADC attenuation (ESP32 specific)
typedef enum {
    ADC_0db,
    ADC_2_5db,
    ADC_6db,
    ADC_11db
} adc_attenuation_t;

// ESP32 return types
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_ERR_NOT_FOUND 0x0105

// Mock hardware state management
class MockHardwareState {
public:
    static MockHardwareState& instance() {
        static MockHardwareState inst;
        return inst;
    }
    
    // GPIO state management
    void setDigitalPin(uint8_t pin, int value) { digitalPins[pin] = value; }
    void setAnalogPin(uint8_t pin, int value) { analogPins[pin] = value; }
    void setPinMode(uint8_t pin, uint8_t mode) { pinModes[pin] = mode; }
    
    int getDigitalPin(uint8_t pin) { return digitalPins.count(pin) ? digitalPins[pin] : LOW; }
    int getAnalogPin(uint8_t pin) { return analogPins.count(pin) ? analogPins[pin] : 0; }
    uint8_t getPinMode(uint8_t pin) { return pinModes.count(pin) ? pinModes[pin] : INPUT; }
    
    // Time management
    void setMillis(uint32_t time) { currentMillis = time; }
    uint32_t getMillis() const { return currentMillis; }
    void advanceTime(uint32_t ms) { currentMillis += ms; }
    
    // ADC configuration
    void setAnalogResolution(uint8_t bits) { analogResolution = bits; }
    void setAnalogAttenuation(adc_attenuation_t attenuation) { analogAtten = attenuation; }
    uint8_t getAnalogResolution() const { return analogResolution; }
    adc_attenuation_t getAnalogAttenuation() const { return analogAtten; }
    
    // Reset state for testing
    void reset() {
        digitalPins.clear();
        analogPins.clear();
        pinModes.clear();
        currentMillis = 0;
        analogResolution = 10;
        analogAtten = ADC_11db;
    }
    
    // Interrupt simulation
    void registerInterrupt(uint8_t pin, std::function<void()> callback) {
        if (callback) {
            interruptCallbacks[pin] = callback;
        } else {
            interruptCallbacks.erase(pin);
        }
    }
    void triggerInterrupt(uint8_t pin) {
        if (interruptCallbacks.count(pin)) {
            interruptCallbacks[pin]();
        }
    }
    
private:
    std::map<uint8_t, int> digitalPins;
    std::map<uint8_t, int> analogPins;
    std::map<uint8_t, uint8_t> pinModes;
    std::map<uint8_t, std::function<void()>> interruptCallbacks;
    uint32_t currentMillis = 0;
    uint8_t analogResolution = 10;
    adc_attenuation_t analogAtten = ADC_11db;
};

// Mock Arduino functions
inline uint32_t millis() { 
    return MockHardwareState::instance().getMillis(); 
}

inline void delay(uint32_t ms) { 
    MockHardwareState::instance().advanceTime(ms); 
}

inline void analogReadResolution(uint8_t bits) { 
    MockHardwareState::instance().setAnalogResolution(bits); 
}

inline void analogSetAttenuation(adc_attenuation_t attenuation) { 
    MockHardwareState::instance().setAnalogAttenuation(attenuation); 
}

inline int digitalRead(uint8_t pin) { 
    return MockHardwareState::instance().getDigitalPin(pin); 
}

inline int analogRead(uint8_t pin) { 
    return MockHardwareState::instance().getAnalogPin(pin); 
}

inline void pinMode(uint8_t pin, uint8_t mode) { 
    MockHardwareState::instance().setPinMode(pin, mode); 
}

inline void digitalWrite(uint8_t pin, uint8_t value) {
    MockHardwareState::instance().setDigitalPin(pin, value);
}

// Mock interrupt functions
inline int digitalPinToInterrupt(int pin) {
    return pin; // In mock, interrupt number equals pin number
}

inline void attachInterrupt(int interrupt, void (*callback)(), int mode) {
    MockHardwareState::instance().registerInterrupt(interrupt, callback);
}

inline void detachInterrupt(int interrupt) {
    MockHardwareState::instance().registerInterrupt(interrupt, nullptr);
}

// Serial mock
class MockSerial {
public:
    void begin(unsigned long baud) {}
    void print(const char* str) {}
    void println(const char* str) {}
    void print(int value) {}
    void println(int value) {}
    void print(float value) {}
    void println(float value) {}
    size_t write(uint8_t byte) { return 1; }
    int available() { return 0; }
    int read() { return -1; }
};

extern MockSerial Serial;

// SPI mock
class MockSPI {
public:
    void begin() {}
    void end() {}
    void beginTransaction(uint32_t settings) {}
    void endTransaction() {}
    uint8_t transfer(uint8_t data) { return data; }
    void transfer(void* buf, size_t count) {}
};

extern MockSPI SPI;

// Arduino String class mock
class String {
private:
    std::string data;

public:
    String() = default;
    String(const char* str) : data(str ? str : "") {}
    String(const std::string& str) : data(str) {}
    String(int value) : data(std::to_string(value)) {}
    String(float value) : data(std::to_string(value)) {}
    
    // Assignment operators
    String& operator=(const char* str) { data = str ? str : ""; return *this; }
    String& operator=(const std::string& str) { data = str; return *this; }
    String& operator=(const String& str) { data = str.data; return *this; }
    
    // Concatenation
    String operator+(const String& str) const { return String(data + str.data); }
    String operator+(const char* str) const { return String(data + (str ? str : "")); }
    String& operator+=(const String& str) { data += str.data; return *this; }
    String& operator+=(const char* str) { data += (str ? str : ""); return *this; }
    
    // Comparison
    bool operator==(const String& str) const { return data == str.data; }
    bool operator==(const char* str) const { return data == (str ? str : ""); }
    bool operator!=(const String& str) const { return data != str.data; }
    bool operator!=(const char* str) const { return data != (str ? str : ""); }
    
    // Access
    char operator[](unsigned int index) const { return index < data.length() ? data[index] : 0; }
    char charAt(unsigned int index) const { return index < data.length() ? data[index] : 0; }
    
    // Size and capacity
    unsigned int length() const { return data.length(); }
    unsigned int size() const { return data.size(); }
    bool isEmpty() const { return data.empty(); }
    
    // Conversion
    const char* c_str() const { return data.c_str(); }
    int toInt() const { try { return std::stoi(data); } catch(...) { return 0; } }
    float toFloat() const { try { return std::stof(data); } catch(...) { return 0.0f; } }
    
    // String manipulation
    void clear() { data.clear(); }
    String substring(unsigned int beginIndex) const { 
        return beginIndex < data.length() ? String(data.substr(beginIndex)) : String(); 
    }
    String substring(unsigned int beginIndex, unsigned int endIndex) const {
        if (beginIndex >= data.length()) return String();
        if (endIndex > data.length()) endIndex = data.length();
        if (beginIndex >= endIndex) return String();
        return String(data.substr(beginIndex, endIndex - beginIndex));
    }
    
    // Search
    int indexOf(char ch) const { 
        size_t pos = data.find(ch); 
        return pos != std::string::npos ? static_cast<int>(pos) : -1; 
    }
    int indexOf(const String& str) const { 
        size_t pos = data.find(str.data); 
        return pos != std::string::npos ? static_cast<int>(pos) : -1; 
    }
    
    // Conversion to std::string for interoperability
    operator std::string() const { return data; }
    std::string toStdString() const { return data; }
};

// WiFi status constants
#define WL_CONNECTED 3
#define WL_NO_SSID_AVAIL 1
#define WL_CONNECT_FAILED 4
#define WL_IDLE_STATUS 0

#endif // UNIT_TESTING