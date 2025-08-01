#pragma once

#ifdef UNIT_TESTING

#include <string>
#include <map>
#include <variant>

// Mock ArduinoJson for unit testing
class JsonDocument {
private:
    std::map<std::string, std::variant<std::string, int, double, bool>> data;

public:
    class JsonVariant {
    private:
        std::variant<std::string, int, double, bool>* valuePtr;
        std::string key;
        JsonDocument* doc;

    public:
        JsonVariant(JsonDocument* d, const std::string& k) : doc(d), key(k), valuePtr(nullptr) {
            auto it = doc->data.find(key);
            if (it != doc->data.end()) {
                valuePtr = &it->second;
            }
        }

        bool isNull() const { return valuePtr == nullptr; }
        
        const char* as() const {
            if (valuePtr && std::holds_alternative<std::string>(*valuePtr)) {
                return std::get<std::string>(*valuePtr).c_str();
            }
            return "";
        }
        
        template<typename T>
        T as() const {
            if (valuePtr) {
                if constexpr (std::is_same_v<T, const char*>) {
                    if (std::holds_alternative<std::string>(*valuePtr)) {
                        return std::get<std::string>(*valuePtr).c_str();
                    }
                } else if constexpr (std::is_same_v<T, std::string>) {
                    if (std::holds_alternative<std::string>(*valuePtr)) {
                        return std::get<std::string>(*valuePtr);
                    }
                }
            }
            return T{};
        }
        
        JsonVariant& operator=(const char* value) {
            doc->data[key] = std::string(value);
            return *this;
        }
    };

    JsonVariant operator[](const char* key) {
        return JsonVariant(this, key);
    }
};

enum class DeserializationError {
    Ok
};

inline DeserializationError deserializeJson(JsonDocument& doc, const std::string& json) {
    // Simple mock - just return Ok
    return DeserializationError::Ok;
}

inline void serializeJson(const JsonDocument& doc, std::string& output) {
    // Simple mock - create basic JSON
    output = "{}";
}

// String version for compatibility
class String;
inline void serializeJson(const JsonDocument& doc, String& output);

#endif // UNIT_TESTING