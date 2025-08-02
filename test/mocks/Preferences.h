#pragma once

#ifdef UNIT_TESTING

#include <string>
#include <map>
#include "Arduino.h"  // Use String from Arduino.h instead of redefining

// Global mock preferences storage
class MockPreferencesStorage {
private:
    std::map<std::string, std::string> storage;
    bool initialized = false;
    
public:
    static MockPreferencesStorage& getInstance() {
        static MockPreferencesStorage instance;
        return instance;
    }
    
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
    
    void reset() {
        storage.clear();
        initialized = false;
    }
    
    void setString(const char* key, const String& value) {
        storage[key] = value.c_str();
    }
    
    bool isInitialized() const {
        return initialized;
    }
};

// Mock Preferences class that uses the singleton storage
class Preferences {
public:
    bool begin(const char* name, bool readOnly = false) {
        return MockPreferencesStorage::getInstance().begin(name, readOnly);
    }
    
    void end() {
        MockPreferencesStorage::getInstance().end();
    }
    
    bool clear() {
        return MockPreferencesStorage::getInstance().clear();
    }
    
    bool remove(const char* key) {
        return MockPreferencesStorage::getInstance().remove(key);
    }
    
    size_t putString(const char* key, const String& value) {
        return MockPreferencesStorage::getInstance().putString(key, value);
    }
    
    String getString(const char* key, const String& defaultValue = String()) {
        return MockPreferencesStorage::getInstance().getString(key, defaultValue);
    }
};

#endif // UNIT_TESTING