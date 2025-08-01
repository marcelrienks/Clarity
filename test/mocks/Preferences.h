#pragma once

#ifdef UNIT_TESTING

#include <string>
#include <map>

// Mock Arduino String class
class String {
private:
    std::string data;
public:
    String() = default;
    String(const char* str) : data(str ? str : "") {}
    String(const std::string& str) : data(str) {}
    
    size_t length() const { return data.length(); }
    const char* c_str() const { return data.c_str(); }
    String& operator=(const char* str) { data = str ? str : ""; return *this; }
    String& operator=(const std::string& str) { data = str; return *this; }
};

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