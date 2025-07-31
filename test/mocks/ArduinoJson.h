#ifndef ARDUINOJSON_MOCK_H
#define ARDUINOJSON_MOCK_H

// Mock ArduinoJson header for native testing

#include <string>
#include <map>
#include <stdint.h>
#include <cstring>

// Mock ArduinoJson namespace
namespace ArduinoJson {
    
// Mock JsonDocument class
class JsonDocument {
public:
    JsonDocument() = default;
    virtual ~JsonDocument() = default;
    
    // Mock JSON access methods
    template<typename T>
    T as() const { return T{}; }
    
    bool isNull() const { return true; }
    size_t size() const { return 0; }
    void clear() {}
    
    // Mock array/object access
    JsonDocument operator[](const char* key) const { return JsonDocument(); }
    JsonDocument operator[](size_t index) const { return JsonDocument(); }
    
    // Mock assignment operators
    template<typename T>
    JsonDocument& operator=(const T& value) { return *this; }
};

// Mock DynamicJsonDocument
class DynamicJsonDocument : public JsonDocument {
public:
    DynamicJsonDocument(size_t capacity) : capacity_(capacity) {}
    
    size_t capacity() const { return capacity_; }
    size_t memoryUsage() const { return 0; }
    
private:
    size_t capacity_;
};

// Mock StaticJsonDocument
template<size_t Capacity>
class StaticJsonDocument : public JsonDocument {
public:
    StaticJsonDocument() = default;
    size_t capacity() const { return Capacity; }
};

} // namespace ArduinoJson

// Global using declarations for compatibility
using ArduinoJson::JsonDocument;
using ArduinoJson::DynamicJsonDocument;
using ArduinoJson::StaticJsonDocument;

// Mock serialization functions
template<typename T>
size_t serializeJson(const T& doc, char* output, size_t size) {
    if (output && size > 0) {
        strcpy(output, "{}");
        return 2;
    }
    return 0;
}

template<typename T>
size_t serializeJson(const T& doc, std::string& output) {
    output = "{}";
    return 2;
}

// Mock deserialization functions
template<typename T>
void deserializeJson(T& doc, const char* input) {}

template<typename T>
void deserializeJson(T& doc, const std::string& input) {}

#endif // ARDUINOJSON_MOCK_H