#ifndef PREFERENCES_MOCK_H
#define PREFERENCES_MOCK_H

// Mock Preferences header for native testing (ESP32 Preferences library)

#include <string>
#include <map>

class Preferences {
public:
    bool begin(const char* name, bool readOnly = false) { return true; }
    void end() {}
    
    bool clear() { return true; }
    bool remove(const char* key) { return true; }
    
    size_t putChar(const char* key, int8_t value) { storage_[key] = std::to_string(value); return 1; }
    size_t putUChar(const char* key, uint8_t value) { storage_[key] = std::to_string(value); return 1; }
    size_t putShort(const char* key, int16_t value) { storage_[key] = std::to_string(value); return 1; }
    size_t putUShort(const char* key, uint16_t value) { storage_[key] = std::to_string(value); return 1; }
    size_t putInt(const char* key, int32_t value) { storage_[key] = std::to_string(value); return 1; }
    size_t putUInt(const char* key, uint32_t value) { storage_[key] = std::to_string(value); return 1; }
    size_t putLong(const char* key, int32_t value) { storage_[key] = std::to_string(value); return 1; }
    size_t putULong(const char* key, uint32_t value) { storage_[key] = std::to_string(value); return 1; }
    size_t putLong64(const char* key, int64_t value) { storage_[key] = std::to_string(value); return 1; }
    size_t putULong64(const char* key, uint64_t value) { storage_[key] = std::to_string(value); return 1; }
    size_t putFloat(const char* key, float value) { storage_[key] = std::to_string(value); return 1; }
    size_t putDouble(const char* key, double value) { storage_[key] = std::to_string(value); return 1; }
    size_t putBool(const char* key, bool value) { storage_[key] = value ? "1" : "0"; return 1; }
    size_t putString(const char* key, const char* value) { storage_[key] = value ? value : ""; return strlen(value); }
    size_t putString(const char* key, const std::string& value) { storage_[key] = value; return value.length(); }
    
    int8_t getChar(const char* key, int8_t defaultValue = 0) {
        auto it = storage_.find(key);
        return it != storage_.end() ? std::stoi(it->second) : defaultValue;
    }
    
    uint8_t getUChar(const char* key, uint8_t defaultValue = 0) {
        auto it = storage_.find(key);
        return it != storage_.end() ? std::stoul(it->second) : defaultValue;
    }
    
    int16_t getShort(const char* key, int16_t defaultValue = 0) {
        auto it = storage_.find(key);
        return it != storage_.end() ? std::stoi(it->second) : defaultValue;
    }
    
    uint16_t getUShort(const char* key, uint16_t defaultValue = 0) {
        auto it = storage_.find(key);
        return it != storage_.end() ? std::stoul(it->second) : defaultValue;
    }
    
    int32_t getInt(const char* key, int32_t defaultValue = 0) {
        auto it = storage_.find(key);
        return it != storage_.end() ? std::stoi(it->second) : defaultValue;
    }
    
    uint32_t getUInt(const char* key, uint32_t defaultValue = 0) {
        auto it = storage_.find(key);
        return it != storage_.end() ? std::stoul(it->second) : defaultValue;
    }
    
    int32_t getLong(const char* key, int32_t defaultValue = 0) {
        auto it = storage_.find(key);
        return it != storage_.end() ? std::stoi(it->second) : defaultValue;
    }
    
    uint32_t getULong(const char* key, uint32_t defaultValue = 0) {
        auto it = storage_.find(key);
        return it != storage_.end() ? std::stoul(it->second) : defaultValue;
    }
    
    int64_t getLong64(const char* key, int64_t defaultValue = 0) {
        auto it = storage_.find(key);
        return it != storage_.end() ? std::stoll(it->second) : defaultValue;
    }
    
    uint64_t getULong64(const char* key, uint64_t defaultValue = 0) {
        auto it = storage_.find(key);
        return it != storage_.end() ? std::stoull(it->second) : defaultValue;
    }
    
    float getFloat(const char* key, float defaultValue = 0.0f) {
        auto it = storage_.find(key);
        return it != storage_.end() ? std::stof(it->second) : defaultValue;
    }
    
    double getDouble(const char* key, double defaultValue = 0.0) {
        auto it = storage_.find(key);
        return it != storage_.end() ? std::stod(it->second) : defaultValue;
    }
    
    bool getBool(const char* key, bool defaultValue = false) {
        auto it = storage_.find(key);
        return it != storage_.end() ? (it->second == "1") : defaultValue;
    }
    
    std::string getString(const char* key, const std::string& defaultValue = "") {
        auto it = storage_.find(key);
        return it != storage_.end() ? it->second : defaultValue;
    }
    
    size_t getString(const char* key, char* value, size_t maxLen) {
        auto it = storage_.find(key);
        if (it != storage_.end()) {
            strncpy(value, it->second.c_str(), maxLen - 1);
            value[maxLen - 1] = '\0';
            return it->second.length();
        }
        value[0] = '\0';
        return 0;
    }
    
    bool isKey(const char* key) {
        return storage_.find(key) != storage_.end();
    }
    
private:
    std::map<std::string, std::string> storage_;
};

#endif // PREFERENCES_MOCK_H