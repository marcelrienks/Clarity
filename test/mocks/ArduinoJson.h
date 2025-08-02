#pragma once

// Mock ArduinoJson definitions for unit testing
#ifdef UNIT_TESTING

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <sstream>

// Forward declarations
class JsonVariant;
class JsonObject;
class JsonArray;

// Mock JSON value type
using JsonValue = std::variant<std::string, int, double, bool, JsonObject*, JsonArray*>;

// Mock JSON object
class JsonObject {
private:
    std::map<std::string, JsonValue> data;
    
public:
    JsonObject() = default;
    ~JsonObject() = default;
    
    void clear() {
        data.clear();
    }
    
    JsonVariant operator[](const std::string& key);
    JsonVariant operator[](const char* key) { return (*this)[std::string(key)]; }
    
    bool containsKey(const std::string& key) const {
        return data.find(key) != data.end();
    }
    
    bool containsKey(const char* key) const {
        return containsKey(std::string(key));
    }
    
    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
    
    // Iterator support
    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
    
    void setValue(const std::string& key, const JsonValue& value) {
        data[key] = value;
    }
    
    const JsonValue* getValue(const std::string& key) const {
        auto it = data.find(key);
        return it != data.end() ? &it->second : nullptr;
    }
};

// Mock JSON array
class JsonArray {
private:
    std::vector<JsonValue> data;
    
public:
    JsonArray() = default;
    ~JsonArray() = default;
    
    void clear() {
        data.clear();
    }
    
    JsonVariant operator[](size_t index);
    
    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
    
    void add(const JsonValue& value) {
        data.push_back(value);
    }
    
    // Iterator support
    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
    
    const JsonValue* getValue(size_t index) const {
        return index < data.size() ? &data[index] : nullptr;
    }
};

// Mock JSON variant (can hold any JSON value)
class JsonVariant {
private:
    JsonValue* value_ptr;
    JsonObject* parent_obj;
    JsonArray* parent_arr;
    std::string key;
    size_t index;
    bool is_object_member;
    
public:
    JsonVariant(JsonValue* val = nullptr) 
        : value_ptr(val), parent_obj(nullptr), parent_arr(nullptr), 
          index(0), is_object_member(false) {}
    
    JsonVariant(JsonObject* obj, const std::string& k) 
        : value_ptr(nullptr), parent_obj(obj), parent_arr(nullptr), 
          key(k), index(0), is_object_member(true) {}
    
    JsonVariant(JsonArray* arr, size_t i) 
        : value_ptr(nullptr), parent_obj(nullptr), parent_arr(arr), 
          index(i), is_object_member(false) {}
    
    // Assignment operators
    JsonVariant& operator=(const std::string& val) {
        if (is_object_member && parent_obj) {
            parent_obj->setValue(key, val);
        }
        return *this;
    }
    
    JsonVariant& operator=(const char* val) {
        return *this = std::string(val);
    }
    
    JsonVariant& operator=(int val) {
        if (is_object_member && parent_obj) {
            parent_obj->setValue(key, val);
        }
        return *this;
    }
    
    JsonVariant& operator=(double val) {
        if (is_object_member && parent_obj) {
            parent_obj->setValue(key, val);
        }
        return *this;
    }
    
    JsonVariant& operator=(bool val) {
        if (is_object_member && parent_obj) {
            parent_obj->setValue(key, val);
        }
        return *this;
    }
    
    // Template as() method
    template<typename T>
    T as() const {
        const JsonValue* val = nullptr;
        if (is_object_member && parent_obj) {
            val = parent_obj->getValue(key);
        } else if (parent_arr) {
            val = parent_arr->getValue(index);
        } else if (value_ptr) {
            val = value_ptr;
        }
        
        if (val) {
            if constexpr (std::is_same_v<T, std::string>) {
                if (std::holds_alternative<std::string>(*val)) {
                    return std::get<std::string>(*val);
                }
            } else if constexpr (std::is_same_v<T, const char*>) {
                if (std::holds_alternative<std::string>(*val)) {
                    static std::string temp = std::get<std::string>(*val);
                    return temp.c_str();
                }
            } else if constexpr (std::is_same_v<T, int>) {
                if (std::holds_alternative<int>(*val)) {
                    return std::get<int>(*val);
                }
            } else if constexpr (std::is_same_v<T, double>) {
                if (std::holds_alternative<double>(*val)) {
                    return std::get<double>(*val);
                }
            } else if constexpr (std::is_same_v<T, bool>) {
                if (std::holds_alternative<bool>(*val)) {
                    return std::get<bool>(*val);
                }
            }
        }
        return T{};
    }
    
    // Conversion operators
    operator std::string() const { return as<std::string>(); }
    operator int() const { return as<int>(); }
    operator double() const { return as<double>(); }
    operator bool() const { return as<bool>(); }
    
    // Type checking
    bool isNull() const {
        const JsonValue* val = nullptr;
        if (is_object_member && parent_obj) {
            val = parent_obj->getValue(key);
        } else if (parent_arr) {
            val = parent_arr->getValue(index);
        } else if (value_ptr) {
            val = value_ptr;
        }
        return val == nullptr;
    }
    
    template<typename T>
    bool is() const {
        const JsonValue* val = nullptr;
        if (is_object_member && parent_obj) {
            val = parent_obj->getValue(key);
        }
        
        if (val) {
            if constexpr (std::is_same_v<T, std::string>) {
                return std::holds_alternative<std::string>(*val);
            } else if constexpr (std::is_same_v<T, int>) {
                return std::holds_alternative<int>(*val);
            } else if constexpr (std::is_same_v<T, bool>) {
                return std::holds_alternative<bool>(*val);
            }
        }
        return false;
    }
};

// Implementation of JsonObject::operator[]
inline JsonVariant JsonObject::operator[](const std::string& key) {
    return JsonVariant(this, key);
}

// Implementation of JsonArray::operator[]
inline JsonVariant JsonArray::operator[](size_t index) {
    return JsonVariant(this, index);
}

// Mock JSON document
class JsonDocument {
private:
    JsonObject root;
    
public:
    JsonDocument() = default;
    JsonDocument(size_t capacity) {} // Ignore capacity for mock
    
    void clear() {
        root.clear();
    }
    
    JsonVariant operator[](const std::string& key) {
        return root[key];
    }
    
    JsonVariant operator[](const char* key) {
        return root[key];
    }
    
    template<typename T>
    T& as() { 
        if constexpr (std::is_same_v<T, JsonObject>) {
            return root;
        }
    }
    
    template<typename T>
    const T& as() const { 
        if constexpr (std::is_same_v<T, JsonObject>) {
            return root;
        }
    }
    
    bool containsKey(const std::string& key) const {
        return root.containsKey(key);
    }
    
    bool containsKey(const char* key) const {
        return root.containsKey(key);
    }
    
    size_t size() const { return root.size(); }
    bool isNull() const { return root.empty(); }
    
    template<typename T>
    T& to() { 
        if constexpr (std::is_same_v<T, JsonObject>) {
            return root;
        }
    }
    
    // Access the root object
    JsonObject& getRoot() { return root; }
    const JsonObject& getRoot() const { return root; }
};

// Common document types
template<size_t N>
using StaticJsonDocument = JsonDocument;

using DynamicJsonDocument = JsonDocument;

// Error handling
enum class DeserializationError {
    Ok,
    InvalidInput,
    NoMemory
};

// Mock serialization functions
inline size_t serializeJson(const JsonDocument& doc, std::string& output) {
    output = "{";
    bool first = true;
    const auto& root = doc.getRoot();
    for (const auto& [key, value] : root) {
        if (!first) output += ",";
        output += "\"" + key + "\":";
        
        std::visit([&output](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::string>) {
                output += "\"" + v + "\"";
            } else if constexpr (std::is_same_v<T, int>) {
                output += std::to_string(v);
            } else if constexpr (std::is_same_v<T, double>) {
                output += std::to_string(v);
            } else if constexpr (std::is_same_v<T, bool>) {
                output += v ? "true" : "false";
            } else {
                output += "null";
            }
        }, value);
        
        first = false;
    }
    output += "}";
    return output.length();
}

inline size_t serializeJson(const JsonDocument& doc, char* buffer, size_t size) {
    std::string output;
    size_t len = serializeJson(doc, output);
    if (len < size) {
        strcpy(buffer, output.c_str());
        return len;
    }
    return 0;
}

inline DeserializationError deserializeJson(JsonDocument& doc, const std::string& input) {
    doc.clear();
    // Simple mock deserialization - parse basic key-value pairs
    // This is a very simplified parser for testing purposes
    if (input.empty() || input == "{}") {
        return DeserializationError::Ok;
    }
    
    // For testing, we'll support simple formats like {"key":"value","num":123}
    size_t pos = input.find('{');
    if (pos == std::string::npos) return DeserializationError::InvalidInput;
    
    pos++;
    while (pos < input.length()) {
        // Skip whitespace
        while (pos < input.length() && std::isspace(input[pos])) pos++;
        if (pos >= input.length() || input[pos] == '}') break;
        
        // Parse key
        if (input[pos] != '"') return DeserializationError::InvalidInput;
        pos++;
        size_t key_start = pos;
        while (pos < input.length() && input[pos] != '"') pos++;
        if (pos >= input.length()) return DeserializationError::InvalidInput;
        std::string key = input.substr(key_start, pos - key_start);
        pos++;
        
        // Skip to colon
        while (pos < input.length() && std::isspace(input[pos])) pos++;
        if (pos >= input.length() || input[pos] != ':') return DeserializationError::InvalidInput;
        pos++;
        while (pos < input.length() && std::isspace(input[pos])) pos++;
        
        // Parse value
        if (pos >= input.length()) return DeserializationError::InvalidInput;
        
        if (input[pos] == '"') {
            // String value
            pos++;
            size_t val_start = pos;
            while (pos < input.length() && input[pos] != '"') pos++;
            if (pos >= input.length()) return DeserializationError::InvalidInput;
            std::string value = input.substr(val_start, pos - val_start);
            doc[key] = value;
            pos++;
        } else if (std::isdigit(input[pos]) || input[pos] == '-') {
            // Number value
            size_t val_start = pos;
            if (input[pos] == '-') pos++;
            while (pos < input.length() && std::isdigit(input[pos])) pos++;
            std::string value = input.substr(val_start, pos - val_start);
            doc[key] = std::stoi(value);
        } else if (input.substr(pos, 4) == "true") {
            doc[key] = true;
            pos += 4;
        } else if (input.substr(pos, 5) == "false") {
            doc[key] = false;
            pos += 5;
        }
        
        // Skip to next item or end
        while (pos < input.length() && std::isspace(input[pos])) pos++;
        if (pos < input.length() && input[pos] == ',') pos++;
    }
    
    return DeserializationError::Ok;
}

inline DeserializationError deserializeJson(JsonDocument& doc, const char* input) {
    return deserializeJson(doc, std::string(input));
}

#endif // UNIT_TESTING