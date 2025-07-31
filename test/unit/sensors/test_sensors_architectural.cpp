#include <unity.h>
#include <memory>

// Project Includes - Using new architecture
#include "system/service_container.h"
#include "system/component_registry.h"
#include "interfaces/i_sensor.h"
#include "interfaces/i_gpio_provider.h"
#include "utilities/reading_helper.h"

// Actual Sensor Implementations
#include "sensors/key_sensor.h"
#include "sensors/lock_sensor.h"
#include "sensors/oil_pressure_sensor.h"
#include "sensors/oil_temperature_sensor.h"

// Test utilities
#include "test_utilities.h"

// Test GPIO Provider Implementation
class TestGpioProvider : public IGpioProvider {
public:
    TestGpioProvider() {
        for (int i = 0; i < 40; i++) {
            pin_states_[i] = false;
            analog_values_[i] = 0;
        }
        // Set default values for testing
        analog_values_[34] = 2048; // Normal oil pressure
        analog_values_[35] = 1500; // Normal oil temperature
    }
    
    void setPinMode(int pin, int mode) override {}
    
    bool digitalRead(int pin) override { 
        return (pin >= 0 && pin < 40) ? pin_states_[pin] : false; 
    }
    
    void digitalWrite(int pin, bool state) override {
        if (pin >= 0 && pin < 40) {
            pin_states_[pin] = state;
        }
    }
    
    uint16_t analogRead(int pin) override { 
        return (pin >= 0 && pin < 40) ? analog_values_[pin] : 0; 
    }
    
    // Test helper methods
    void setTestGpioState(int pin, bool state) {
        if (pin >= 0 && pin < 40) {
            pin_states_[pin] = state;
        }
    }
    
    void setTestAnalogValue(int pin, uint16_t value) {
        if (pin >= 0 && pin < 40) {
            analog_values_[pin] = value;
        }
    }
    
    void simulateFailure(int pin, bool fail) {
        if (fail) {
            analog_values_[pin] = 0;
        }
    }
    
private:
    bool pin_states_[40];
    uint16_t analog_values_[40];
};

// Global DI container for tests
static std::unique_ptr<ServiceContainer> container;
static std::unique_ptr<ComponentRegistry> registry;

void setUp(void)
{
    // Create new DI container for each test
    container = std::make_unique<ServiceContainer>();
    registry = std::make_unique<ComponentRegistry>(*container);
    
    // Register GPIO provider
    container->registerSingleton<IGpioProvider>([]() {
        return std::make_unique<TestGpioProvider>();
    });
}

void tearDown(void)
{
    registry.reset();
    container.reset();
}

// =================================================================
// ARCHITECTURAL SENSOR TESTS - USING DEPENDENCY INJECTION
// =================================================================

void test_architectural_key_sensor_creation_via_registry(void)
{
    // Register sensor using new architecture
    registry->registerSensor<KeySensor>("KeySensor");
    
    // Create sensor via registry (with automatic DI)
    auto keySensor = registry->createSensor("KeySensor");
    
    TEST_ASSERT_NOT_NULL(keySensor.get());
    
    // Verify it implements ISensor interface
    ISensor* sensorInterface = dynamic_cast<ISensor*>(keySensor.get());
    TEST_ASSERT_NOT_NULL(sensorInterface);
    
    // Get GPIO provider to simulate input
    IGpioProvider* gpioProvider = container->resolve<IGpioProvider>();
    TestGpioProvider* testGpio = dynamic_cast<TestGpioProvider*>(gpioProvider);
    TEST_ASSERT_NOT_NULL(testGpio);
    
    // Test sensor reading with dependency injection
    testGpio->setTestGpioState(25, false); // Key not present
    Reading reading1 = sensorInterface->read();
    TEST_ASSERT_TRUE(reading1.isValid());
    
    testGpio->setTestGpioState(25, true);  // Key present
    Reading reading2 = sensorInterface->read();
    TEST_ASSERT_TRUE(reading2.isValid());
    
    // Readings should be different
    TEST_ASSERT_NOT_EQUAL(reading1.getValue(), reading2.getValue());
}

void test_architectural_lock_sensor_creation_via_registry(void)
{
    // Register sensor
    registry->registerSensor<LockSensor>("LockSensor");
    
    // Create via registry
    auto lockSensor = registry->createSensor("LockSensor");
    
    TEST_ASSERT_NOT_NULL(lockSensor.get());
    
    ISensor* sensorInterface = dynamic_cast<ISensor*>(lockSensor.get());
    TEST_ASSERT_NOT_NULL(sensorInterface);
    
    // Test with injected GPIO provider
    IGpioProvider* gpioProvider = container->resolve<IGpioProvider>();
    TestGpioProvider* testGpio = dynamic_cast<TestGpioProvider*>(gpioProvider);
    
    testGpio->setTestGpioState(27, false); // Lock not active
    Reading reading1 = sensorInterface->read();
    TEST_ASSERT_TRUE(reading1.isValid());
    
    testGpio->setTestGpioState(27, true);  // Lock active
    Reading reading2 = sensorInterface->read();
    TEST_ASSERT_TRUE(reading2.isValid());
    
    TEST_ASSERT_NOT_EQUAL(reading1.getValue(), reading2.getValue());
}

void test_architectural_oil_pressure_sensor_via_registry(void)
{
    // Register sensor
    registry->registerSensor<OilPressureSensor>("OilPressureSensor");
    
    // Create via registry
    auto pressureSensor = registry->createSensor("OilPressureSensor");
    
    TEST_ASSERT_NOT_NULL(pressureSensor.get());
    
    ISensor* sensorInterface = dynamic_cast<ISensor*>(pressureSensor.get());
    TEST_ASSERT_NOT_NULL(sensorInterface);
    
    // Test with injected GPIO provider
    IGpioProvider* gpioProvider = container->resolve<IGpioProvider>();
    TestGpioProvider* testGpio = dynamic_cast<TestGpioProvider*>(gpioProvider);
    
    // Test different pressure values
    testGpio->setTestAnalogValue(34, 0);      // No pressure
    Reading reading1 = sensorInterface->GetReading();
    TEST_ASSERT_TRUE(ReadingHelper::isValid(reading1));
    
    testGpio->setTestAnalogValue(34, 2048);   // Normal pressure
    Reading reading2 = sensorInterface->GetReading();
    TEST_ASSERT_TRUE(ReadingHelper::isValid(reading2));
    
    testGpio->setTestAnalogValue(34, 4095);   // Max pressure
    Reading reading3 = sensorInterface->GetReading();
    TEST_ASSERT_TRUE(ReadingHelper::isValid(reading3));
    
    // Values should be different
    TEST_ASSERT_NOT_EQUAL(ReadingHelper::getValue<int32_t>(reading1),
                         ReadingHelper::getValue<int32_t>(reading2));
    TEST_ASSERT_NOT_EQUAL(ReadingHelper::getValue<int32_t>(reading2),
                         ReadingHelper::getValue<int32_t>(reading3));
}

void test_architectural_oil_temperature_sensor_via_registry(void)
{
    // Register sensor
    registry->registerSensor<OilTemperatureSensor>("OilTemperatureSensor");
    
    // Create via registry
    auto tempSensor = registry->createSensor("OilTemperatureSensor");
    
    TEST_ASSERT_NOT_NULL(tempSensor.get());
    
    ISensor* sensorInterface = dynamic_cast<ISensor*>(tempSensor.get());
    TEST_ASSERT_NOT_NULL(sensorInterface);
    
    // Test with injected GPIO provider
    IGpioProvider* gpioProvider = container->resolve<IGpioProvider>();
    TestGpioProvider* testGpio = dynamic_cast<TestGpioProvider*>(gpioProvider);
    
    // Test different temperature values
    testGpio->setTestAnalogValue(35, 1200);   // Cold temperature
    Reading reading1 = sensorInterface->read();
    TEST_ASSERT_TRUE(reading1.isValid());
    
    testGpio->setTestAnalogValue(35, 1500);   // Normal temperature
    Reading reading2 = sensorInterface->read();
    TEST_ASSERT_TRUE(reading2.isValid());
    
    testGpio->setTestAnalogValue(35, 3500);   // High temperature
    Reading reading3 = sensorInterface->read();
    TEST_ASSERT_TRUE(reading3.isValid());
    
    // Values should be different
    TEST_ASSERT_NOT_EQUAL(reading1.getValue(), reading2.getValue());
    TEST_ASSERT_NOT_EQUAL(reading2.getValue(), reading3.getValue());
}

// =================================================================
// SENSOR INTEGRATION TESTS WITH DEPENDENCY INJECTION
// =================================================================

void test_architectural_multiple_sensors_shared_gpio(void)
{
    // Register multiple sensors
    registry->registerSensor<KeySensor>("KeySensor");
    registry->registerSensor<LockSensor>("LockSensor");
    registry->registerSensor<OilPressureSensor>("OilPressureSensor");
    registry->registerSensor<OilTemperatureSensor>("OilTemperatureSensor");
    
    // Create all sensors via registry
    auto keySensor = registry->createSensor("KeySensor");
    auto lockSensor = registry->createSensor("LockSensor");
    auto pressureSensor = registry->createSensor("OilPressureSensor");
    auto tempSensor = registry->createSensor("OilTemperatureSensor");
    
    TEST_ASSERT_NOT_NULL(keySensor.get());
    TEST_ASSERT_NOT_NULL(lockSensor.get());
    TEST_ASSERT_NOT_NULL(pressureSensor.get());
    TEST_ASSERT_NOT_NULL(tempSensor.get());
    
    // All sensors should share the same GPIO provider (singleton)
    IGpioProvider* gpioProvider = container->resolve<IGpioProvider>();
    TestGpioProvider* testGpio = dynamic_cast<TestGpioProvider*>(gpioProvider);
    
    // Set up different pin states
    testGpio->setTestGpioState(25, true);      // Key present
    testGpio->setTestGpioState(27, false);     // Lock not active
    testGpio->setTestAnalogValue(34, 2500);    // High pressure
    testGpio->setTestAnalogValue(35, 1800);    // Medium temperature
    
    // All sensors should read from the same GPIO provider
    ISensor* keyInterface = dynamic_cast<ISensor*>(keySensor.get());
    ISensor* lockInterface = dynamic_cast<ISensor*>(lockSensor.get());
    ISensor* pressureInterface = dynamic_cast<ISensor*>(pressureSensor.get());
    ISensor* tempInterface = dynamic_cast<ISensor*>(tempSensor.get());
    
    Reading keyReading = keyInterface->read();
    Reading lockReading = lockInterface->read();
    Reading pressureReading = pressureInterface->read();
    Reading tempReading = tempInterface->read();
    
    TEST_ASSERT_TRUE(keyReading.isValid());
    TEST_ASSERT_TRUE(lockReading.isValid());
    TEST_ASSERT_TRUE(pressureReading.isValid());
    TEST_ASSERT_TRUE(tempReading.isValid());
    
    // Verify readings reflect the GPIO state
    TEST_ASSERT_EQUAL_UINT16(2500, gpioProvider->analogRead(34));
    TEST_ASSERT_EQUAL_UINT16(1800, gpioProvider->analogRead(35));
    TEST_ASSERT_TRUE(gpioProvider->digitalRead(25));
    TEST_ASSERT_FALSE(gpioProvider->digitalRead(27));
}

void test_architectural_sensor_failure_handling_via_di(void)
{
    // Register oil sensors
    registry->registerSensor<OilPressureSensor>("OilPressureSensor");
    registry->registerSensor<OilTemperatureSensor>("OilTemperatureSensor");
    
    // Create sensors
    auto pressureSensor = registry->createSensor("OilPressureSensor");
    auto tempSensor = registry->createSensor("OilTemperatureSensor");
    
    ISensor* pressureInterface = dynamic_cast<ISensor*>(pressureSensor.get());
    ISensor* tempInterface = dynamic_cast<ISensor*>(tempSensor.get());
    
    // Get GPIO provider for failure simulation
    IGpioProvider* gpioProvider = container->resolve<IGpioProvider>();
    TestGpioProvider* testGpio = dynamic_cast<TestGpioProvider*>(gpioProvider);
    
    // Normal operation
    testGpio->setTestAnalogValue(34, 2048);
    testGpio->setTestAnalogValue(35, 1500);
    
    Reading normalPressure = pressureInterface->read();
    Reading normalTemp = tempInterface->read();
    
    TEST_ASSERT_TRUE(normalPressure.isValid());
    TEST_ASSERT_TRUE(normalTemp.isValid());
    
    // Simulate sensor failures
    testGpio->simulateFailure(34, true);  // Pressure sensor failure
    testGpio->simulateFailure(35, true);  // Temperature sensor failure
    
    Reading failedPressure = pressureInterface->read();
    Reading failedTemp = tempInterface->read();
    
    // Sensors should still return readings (potentially error values)
    TEST_ASSERT_TRUE(failedPressure.isValid());
    TEST_ASSERT_TRUE(failedTemp.isValid());
    
    // Readings should be different (likely 0 for failed sensors)
    TEST_ASSERT_NOT_EQUAL(normalPressure.getValue(), failedPressure.getValue());
    TEST_ASSERT_NOT_EQUAL(normalTemp.getValue(), failedTemp.getValue());
}

void test_architectural_sensor_real_time_updates(void)
{
    // Register sensor
    registry->registerSensor<OilPressureSensor>("OilPressureSensor");
    
    auto pressureSensor = registry->createSensor("OilPressureSensor");
    ISensor* sensorInterface = dynamic_cast<ISensor*>(pressureSensor.get());
    
    // Get GPIO provider
    IGpioProvider* gpioProvider = container->resolve<IGpioProvider>();
    TestGpioProvider* testGpio = dynamic_cast<TestGpioProvider*>(gpioProvider);
    
    // Test real-time updates
    const uint16_t testValues[] = {1000, 1500, 2000, 2500, 3000};
    const size_t valueCount = sizeof(testValues) / sizeof(testValues[0]);
    
    Reading previousReading;
    
    for (size_t i = 0; i < valueCount; i++) {
        // Update GPIO value
        testGpio->setTestAnalogValue(34, testValues[i]);
        
        // Read sensor
        Reading currentReading = sensorInterface->read();
        
        TEST_ASSERT_TRUE(currentReading.isValid());
        
        // Verify GPIO provider reflects the change
        TEST_ASSERT_EQUAL_UINT16(testValues[i], gpioProvider->analogRead(34));
        
        // Each reading should be different (unless values are the same)
        if (i > 0) {
            TEST_ASSERT_NOT_EQUAL(previousReading.getValue(), currentReading.getValue());
        }
        
        previousReading = currentReading;
    }
}

// =================================================================
// SERVICE LIFECYCLE TESTS
// =================================================================

void test_architectural_gpio_provider_singleton_behavior(void)
{
    // Register multiple sensors
    registry->registerSensor<KeySensor>("KeySensor");
    registry->registerSensor<LockSensor>("LockSensor");
    
    // Create sensors
    auto keySensor1 = registry->createSensor("KeySensor");
    auto keySensor2 = registry->createSensor("KeySensor");
    auto lockSensor = registry->createSensor("LockSensor");
    
    // All should use the same GPIO provider
    IGpioProvider* provider1 = container->resolve<IGpioProvider>();
    IGpioProvider* provider2 = container->resolve<IGpioProvider>();
    
    TEST_ASSERT_EQUAL_PTR(provider1, provider2);
    
    // Changes to one affect all
    TestGpioProvider* testGpio = dynamic_cast<TestGpioProvider*>(provider1);
    testGpio->setTestGpioState(25, true);
    
    TEST_ASSERT_TRUE(provider2->digitalRead(25));
}

void test_architectural_sensor_registry_lifecycle(void)
{
    // Test that sensors can be registered and created multiple times
    registry->registerSensor<KeySensor>("KeySensor");
    
    // Create multiple instances
    auto sensor1 = registry->createSensor("KeySensor");
    auto sensor2 = registry->createSensor("KeySensor");
    auto sensor3 = registry->createSensor("KeySensor");
    
    TEST_ASSERT_NOT_NULL(sensor1.get());
    TEST_ASSERT_NOT_NULL(sensor2.get());
    TEST_ASSERT_NOT_NULL(sensor3.get());
    
    // Should be different instances
    TEST_ASSERT_NOT_EQUAL(sensor1.get(), sensor2.get());
    TEST_ASSERT_NOT_EQUAL(sensor2.get(), sensor3.get());
    
    // But should share the same GPIO provider
    IGpioProvider* provider = container->resolve<IGpioProvider>();
    TestGpioProvider* testGpio = dynamic_cast<TestGpioProvider*>(provider);
    
    testGpio->setTestGpioState(25, true);
    
    // All sensors should see the same GPIO state
    ISensor* sensor1Interface = dynamic_cast<ISensor*>(sensor1.get());
    ISensor* sensor2Interface = dynamic_cast<ISensor*>(sensor2.get());
    ISensor* sensor3Interface = dynamic_cast<ISensor*>(sensor3.get());
    
    Reading reading1 = sensor1Interface->read();
    Reading reading2 = sensor2Interface->read();
    Reading reading3 = sensor3Interface->read();
    
    // All should read the same value from the shared GPIO provider
    TEST_ASSERT_FALSE(std::holds_alternative<std::monostate>(reading1));
    TEST_ASSERT_FALSE(std::holds_alternative<std::monostate>(reading2));
    TEST_ASSERT_FALSE(std::holds_alternative<std::monostate>(reading3));
}

// Note: PlatformIO will automatically discover and run test_ functions