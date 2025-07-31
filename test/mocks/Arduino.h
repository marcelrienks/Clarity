#ifndef ARDUINO_H_MOCK
#define ARDUINO_H_MOCK

// Mock Arduino header for native testing

#include <stdint.h>
#include <string>

// Arduino String class mock
class String {
public:
    String() : data_("") {}
    String(const char* str) : data_(str ? str : "") {}
    String(const std::string& str) : data_(str) {}
    String(int value) : data_(std::to_string(value)) {}
    String(float value) : data_(std::to_string(value)) {}
    
    size_t length() const { return data_.length(); }
    const char* c_str() const { return data_.c_str(); }
    
    String& operator=(const char* str) {
        data_ = str ? str : "";
        return *this;
    }
    
    String& operator=(const std::string& str) {
        data_ = str;
        return *this;
    }
    
    bool operator==(const String& other) const {
        return data_ == other.data_;
    }
    
    String operator+(const String& other) const {
        return String(data_ + other.data_);
    }
    
private:
    std::string data_;
};

// Arduino constants
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

// Arduino functions (mocked)
inline void pinMode(uint8_t pin, uint8_t mode) {}
inline void digitalWrite(uint8_t pin, uint8_t value) {}
inline int digitalRead(uint8_t pin) { return LOW; }
inline int analogRead(uint8_t pin) { return 0; }
inline void delay(unsigned long ms) {}
inline unsigned long millis() { return 0; }
inline unsigned long micros() { return 0; }

// Serial mock
class SerialClass {
public:
    void begin(unsigned long baud) {}
    void print(const char* str) {}
    void print(const String& str) {}
    void println(const char* str) {}
    void println(const String& str) {}
    template<typename T>
    void print(T value) {}
    template<typename T>
    void println(T value) {}
};

extern SerialClass Serial;

#endif // ARDUINO_H_MOCK