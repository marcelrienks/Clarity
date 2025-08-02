#pragma once

#ifdef UNIT_TESTING

#include <string>
#include <map>
#include "Arduino.h"  // Use String from Arduino.h instead of redefining

// Mock Preferences class
class Preferences {
private:
    std::map<std::string, std::string> storage;
    bool initialized = false;

public:
    bool begin(const char* name, bool readOnly = false) {
        initialized = true;
        return true;
    }
    
    void end() {
        initialized = false;
    }
    
    bool clear() {
        storage.clear();
        return true;
    }
    
    bool remove(const char* key) {
        auto it = storage.find(key);
        if (it != storage.end()) {
            storage.erase(it);
            return true;
        }
        return false;
    }
    
    size_t putString(const char* key, const String& value) {
        storage[key] = value.c_str();
        return std::string(value.c_str()).length();
    }
    
    String getString(const char* key, const String& defaultValue = String()) {
        auto it = storage.find(key);
        return (it != storage.end()) ? String(it->second.c_str()) : defaultValue;
    }
};

#endif // UNIT_TESTING