#pragma once

#ifdef UNIT_TESTING

#include <unity.h>
#include <cstring>
#include <string>
#include <memory>

// Common test utilities for the Clarity ESP32 project

// ============================================================================
// COMMON TEST MACROS
// ============================================================================

// Unity extension macros for enhanced string comparison  
#define TEST_ASSERT_NOT_EQUAL_STRING(expected, actual) \
    TEST_ASSERT_FALSE(strcmp(expected, actual) == 0)

// Performance testing macros
#define TEST_ASSERT_PERFORMANCE_THRESHOLD(operation, max_time_ms) \
    do { \
        uint32_t start = millis(); \
        operation; \
        uint32_t elapsed = millis() - start; \
        TEST_ASSERT_LESS_THAN(max_time_ms, elapsed); \
    } while(0)

// Memory testing macros
#define TEST_ASSERT_MEMORY_STABLE(operation) \
    do { \
        size_t before = getFreeHeap(); \
        operation; \
        size_t after = getFreeHeap(); \
        TEST_ASSERT_GREATER_OR_EQUAL(before - 1024, after); /* Allow 1KB tolerance */ \
    } while(0)

// State validation macros
#define TEST_ASSERT_VALID_STATE(state_expr, valid_states...) \
    do { \
        auto state = state_expr; \
        bool valid = false; \
        for (auto validState : {valid_states}) { \
            if (state == validState) { valid = true; break; } \
        } \
        TEST_ASSERT_TRUE(valid); \
    } while(0)

// ============================================================================
// COMMON TEST HELPER FUNCTIONS
// ============================================================================

namespace TestCommon {

    // Timing helpers
    inline uint32_t getCurrentTime() {
        return millis();
    }
    
    inline void waitForTime(uint32_t ms) {
        delay(ms);
    }
    
    // Memory helpers
    inline size_t getFreeHeap() {
        #ifdef ESP32
            return ESP.getFreeHeap();
        #else
            return 50000; // Mock value for native testing
        #endif
    }
    
    // String validation helpers
    inline bool isValidString(const std::string& str, size_t minLen = 1, size_t maxLen = 1000) {
        return str.length() >= minLen && str.length() <= maxLen;
    }
    
    inline bool isValidCString(const char* str, size_t minLen = 1, size_t maxLen = 1000) {
        if (!str) return false;
        size_t len = strlen(str);
        return len >= minLen && len <= maxLen;
    }
    
    // Performance measurement
    class PerformanceTimer {
    private:
        uint32_t startTime_;
        std::string operation_;
        
    public:
        PerformanceTimer(const std::string& operation) 
            : operation_(operation), startTime_(getCurrentTime()) {}
            
        ~PerformanceTimer() {
            uint32_t elapsed = getCurrentTime() - startTime_;
            // In a real implementation, you might log this
        }
        
        uint32_t getElapsed() const {
            return getCurrentTime() - startTime_;
        }
        
        void assertThreshold(uint32_t maxMs) const {
            TEST_ASSERT_LESS_THAN(maxMs, getElapsed());
        }
    };
    
    // Test data generators
    inline std::string generateTestString(size_t length, const std::string& prefix = "test") {
        std::string result = prefix;
        while (result.length() < length) {
            result += std::to_string(result.length() % 10);
        }
        return result.substr(0, length);
    }
    
    inline std::vector<std::string> generateTestStrings(size_t count, size_t length = 10) {
        std::vector<std::string> result;
        for (size_t i = 0; i < count; ++i) {
            result.push_back(generateTestString(length, "test" + std::to_string(i)));
        }
        return result;
    }
    
    // State machine testing helpers
    template<typename StateType>
    class StateMachineValidator {
    private:
        std::vector<StateType> validStates_;
        StateType currentState_;
        bool stateTransitions_[32][32]; // Max 32 states
        
    public:
        StateMachineValidator(const std::vector<StateType>& validStates)
            : validStates_(validStates) {
            memset(stateTransitions_, 0, sizeof(stateTransitions_));
        }
        
        void recordTransition(StateType from, StateType to) {
            int fromIdx = static_cast<int>(from);
            int toIdx = static_cast<int>(to);
            if (fromIdx < 32 && toIdx < 32) {
                stateTransitions_[fromIdx][toIdx] = true;
            }
        }
        
        void validateTransition(StateType from, StateType to) {
            recordTransition(from, to);
            bool validTransition = true; // Implement your validation logic
            TEST_ASSERT_TRUE(validTransition);
        }
        
        void assertValidState(StateType state) {
            for (const auto& validState : validStates_) {
                if (state == validState) return;
            }
            TEST_FAIL_MESSAGE("Invalid state detected");
        }
    };
    
    // Error injection helpers
    class ErrorInjector {
    private:
        bool injectErrors_;
        uint32_t errorRate_; // Percentage 0-100
        
    public:
        ErrorInjector(uint32_t errorRate = 0) 
            : injectErrors_(errorRate > 0), errorRate_(errorRate) {}
            
        bool shouldInjectError() {
            if (!injectErrors_) return false;
            return (rand() % 100) < errorRate_;
        }
        
        void setErrorRate(uint32_t rate) {
            errorRate_ = rate;
            injectErrors_ = rate > 0;
        }
    };
    
    // Resource monitoring
    class ResourceMonitor {
    private:
        size_t initialHeap_;
        uint32_t startTime_;
        
    public:
        ResourceMonitor() : initialHeap_(getFreeHeap()), startTime_(getCurrentTime()) {}
        
        void assertNoMemoryLeak(size_t tolerance = 1024) {
            size_t currentHeap = getFreeHeap();
            TEST_ASSERT_GREATER_OR_EQUAL(initialHeap_ - tolerance, currentHeap);
        }
        
        void assertPerformanceThreshold(uint32_t maxMs) {
            uint32_t elapsed = getCurrentTime() - startTime_;
            TEST_ASSERT_LESS_THAN(maxMs, elapsed);
        }
    };
}

// ============================================================================
// COMMON TEST PATTERNS
// ============================================================================

// Pattern for testing object lifecycle
#define TEST_OBJECT_LIFECYCLE(ObjectType, constructor_args...) \
    do { \
        TestCommon::ResourceMonitor monitor; \
        { \
            ObjectType obj(constructor_args); \
            TEST_ASSERT_NOT_NULL(&obj); \
        } \
        monitor.assertNoMemoryLeak(); \
    } while(0)

// Pattern for testing performance requirements
#define TEST_PERFORMANCE_REQUIREMENT(operation, max_ms, description) \
    do { \
        TestCommon::PerformanceTimer timer(description); \
        operation; \
        timer.assertThreshold(max_ms); \
    } while(0)

// Pattern for testing error recovery
#define TEST_ERROR_RECOVERY(setup, error_injection, recovery, validation) \
    do { \
        setup; \
        error_injection; \
        recovery; \
        validation; \
    } while(0)

#endif // UNIT_TESTING