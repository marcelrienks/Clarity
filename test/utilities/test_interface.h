#pragma once

#ifdef UNIT_TESTING

#include "test_common.h"

// ============================================================================
// STANDARDIZED TEST INTERFACE
// ============================================================================

// Standard test suite interface to eliminate cross-file dependencies
namespace TestInterface {

    // Base class for all test suites
    class TestSuiteBase {
    public:
        virtual ~TestSuiteBase() = default;
        virtual void setUp() {}
        virtual void tearDown() {}
        virtual void runTests() = 0;
        virtual const char* getSuiteName() const = 0;
        virtual int getTestCount() const = 0;
    };

    // Test registration system
    class TestRegistry {
    private:
        static std::vector<TestSuiteBase*> suites_;
        
    public:
        static void registerSuite(TestSuiteBase* suite) {
            suites_.push_back(suite);
        }
        
        static void runAllSuites() {
            printf("\\n=== RUNNING %zu TEST SUITES ===\\n", suites_.size());
            int totalTests = 0;
            
            for (auto* suite : suites_) {
                printf("\\n--- %s (%d tests) ---\\n", 
                       suite->getSuiteName(), suite->getTestCount());
                       
                suite->setUp();
                suite->runTests();
                suite->tearDown();
                
                totalTests += suite->getTestCount();
            }
            
            printf("\\n=== COMPLETED %d TOTAL TESTS ===\\n", totalTests);
        }
        
        static void cleanup() {
            for (auto* suite : suites_) {
                delete suite;
            }
            suites_.clear();
        }
    };

    // Auto-registration helper
    template<typename T>
    class TestSuiteRegistrar {
    public:
        TestSuiteRegistrar() {
            TestRegistry::registerSuite(new T());
        }
    };
}

// Standard test suite implementation macros
#define DECLARE_TEST_SUITE(SuiteName, TestCount) \\
    class SuiteName : public TestInterface::TestSuiteBase { \\
    public: \\
        const char* getSuiteName() const override { return #SuiteName; } \\
        int getTestCount() const override { return TestCount; } \\
        void runTests() override; \\
    }; \\
    static TestInterface::TestSuiteRegistrar<SuiteName> g_##SuiteName##_registrar;

#define IMPLEMENT_TEST_SUITE(SuiteName) \\
    void SuiteName::runTests()

// Standard test naming conventions
#define CORE_TEST(name) test_core_##name
#define SENSOR_TEST(sensor, name) test_sensor_##sensor##_##name
#define MANAGER_TEST(manager, name) test_manager_##manager##_##name
#define PROVIDER_TEST(provider, name) test_provider_##provider##_##name
#define FACTORY_TEST(factory, name) test_factory_##factory##_##name
#define INTEGRATION_TEST(name) test_integration_##name
#define PERFORMANCE_TEST(name) test_performance_##name

// Standard test categories
namespace TestCategories {
    constexpr const char* CORE_LOGIC = "CoreLogic";
    constexpr const char* SENSOR = "Sensor";
    constexpr const char* MANAGER = "Manager";
    constexpr const char* PROVIDER = "Provider";
    constexpr const char* FACTORY = "Factory";
    constexpr const char* INTEGRATION = "Integration";
    constexpr const char* PERFORMANCE = "Performance";
    constexpr const char* ERROR_HANDLING = "ErrorHandling";
}

// ============================================================================
// DEPENDENCY ELIMINATION HELPERS
// ============================================================================

// Standardized mock interfaces to eliminate dependencies
namespace StandardMocks {

    // Standard GPIO provider interface
    class IGpioProviderMock {
    public:
        virtual ~IGpioProviderMock() = default;
        virtual void pinMode(uint8_t pin, uint8_t mode) = 0;
        virtual bool digitalRead(uint8_t pin) = 0;
        virtual uint16_t analogRead(uint8_t pin) = 0;
        virtual void digitalWrite(uint8_t pin, uint8_t value) = 0;
    };

    // Standard display provider interface
    class IDisplayProviderMock {
    public:
        virtual ~IDisplayProviderMock() = default;
        virtual void initialize() = 0;
        virtual void* getScreen() = 0;
        virtual bool isInitialized() const = 0;
    };

    // Standard preference service interface
    class IPreferenceServiceMock {
    public:
        virtual ~IPreferenceServiceMock() = default;
        virtual bool load(const std::string& key, std::string& value) = 0;
        virtual bool save(const std::string& key, const std::string& value) = 0;
        virtual void clear() = 0;
    };

    // Standard style service interface
    class IStyleServiceMock {
    public:
        virtual ~IStyleServiceMock() = default;
        virtual void initializeStyles() = 0;
        virtual void applyTheme(const std::string& theme) = 0;
        virtual std::string getCurrentTheme() const = 0;
    };
}

// ============================================================================
// CROSS-FILE DEPENDENCY ELIMINATION
// ============================================================================

// Standardized test fixture base to replace custom fixtures
class StandardTestFixture {
protected:
    std::unique_ptr<StandardMocks::IGpioProviderMock> gpioProvider_;
    std::unique_ptr<StandardMocks::IDisplayProviderMock> displayProvider_;
    std::unique_ptr<StandardMocks::IPreferenceServiceMock> preferenceService_;
    std::unique_ptr<StandardMocks::IStyleServiceMock> styleService_;
    
public:
    virtual ~StandardTestFixture() = default;
    
    virtual void SetUp() {
        // Initialize standard mocks
        // Implementation would create concrete mock instances
    }
    
    virtual void TearDown() {
        // Cleanup
        gpioProvider_.reset();
        displayProvider_.reset();
        preferenceService_.reset();
        styleService_.reset();
    }
    
    // Standardized accessor methods
    StandardMocks::IGpioProviderMock* getGpioProvider() { return gpioProvider_.get(); }
    StandardMocks::IDisplayProviderMock* getDisplayProvider() { return displayProvider_.get(); }
    StandardMocks::IPreferenceServiceMock* getPreferenceService() { return preferenceService_.get(); }
    StandardMocks::IStyleServiceMock* getStyleService() { return styleService_.get(); }
};

// ============================================================================
// STANDARDIZED TEST UTILITIES
// ============================================================================

// Replace scattered utility functions with standardized versions
namespace StandardTestUtils {

    // Time management
    inline void setMockTime(uint32_t timeMs) {
        MockHardwareState::instance().setMillis(timeMs);
    }
    
    inline uint32_t getMockTime() {
        return millis();
    }
    
    inline void advanceMockTime(uint32_t deltaMs) {
        setMockTime(getMockTime() + deltaMs);
    }
    
    // Memory management
    inline size_t getCurrentMemoryUsage() {
        return TestCommon::getFreeHeap();
    }
    
    // Test data generation
    inline std::string generateUniqueTestId() {
        static int counter = 0;
        return "test_" + std::to_string(++counter) + "_" + std::to_string(getMockTime());
    }
    
    // Error simulation
    class StandardErrorSimulator {
    private:
        bool errorMode_;
        std::string errorType_;
        
    public:
        StandardErrorSimulator() : errorMode_(false) {}
        
        void enableErrorMode(const std::string& errorType) {
            errorMode_ = true;
            errorType_ = errorType;
        }
        
        void disableErrorMode() {
            errorMode_ = false;
            errorType_.clear();
        }
        
        bool shouldSimulateError(const std::string& operation) const {
            return errorMode_ && (errorType_.empty() || errorType_ == operation);
        }
    };
    
    // Performance measurement
    class StandardPerformanceMeasurement {
    private:
        uint32_t startTime_;
        std::string testName_;
        
    public:
        StandardPerformanceMeasurement(const std::string& testName) 
            : testName_(testName), startTime_(getMockTime()) {}
            
        ~StandardPerformanceMeasurement() {
            uint32_t elapsed = getMockTime() - startTime_;
            printf("Performance: %s took %ums\\n", testName_.c_str(), elapsed);
        }
        
        uint32_t getElapsedTime() const {
            return getMockTime() - startTime_;
        }
    };
}

#endif // UNIT_TESTING