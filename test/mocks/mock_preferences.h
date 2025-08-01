#pragma once

#include <map>
#include <string>

// Mock Preferences class for unit testing
class MockPreferences {
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
    
    size_t putString(const char* key, const std::string& value) {
        storage[key] = value;
        return value.length();
    }
    
    std::string getString(const char* key, const std::string& defaultValue = std::string()) {
        auto it = storage.find(key);
        return (it != storage.end()) ? it->second : defaultValue;
    }
    
    // Mock-specific methods
    void reset() {
        storage.clear();
        initialized = false;
    }
    
    bool isInitialized() const {
        return initialized;
    }
    
    size_t size() const {
        return storage.size();
    }
};