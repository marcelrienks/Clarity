#include <unity.h>
#include "sensors/light_sensor.h"
#include "mocks/mock_gpio_provider.h"
#include "hardware/gpio_pins.h"
#include "utilities/types.h"

static MockGpioProvider* lightMockGpio;
static LightSensor* lightSensor;

void setUp_light_sensor() {
    lightMockGpio = new MockGpioProvider();
    lightSensor = new LightSensor(lightMockGpio);
}

void tearDown_light_sensor() {
    delete lightSensor;
    delete lightMockGpio;
}

void test_light_sensor_init() {
    // Test initialization doesn't crash
    lightSensor->init();
    TEST_ASSERT_TRUE(true);
}

void test_light_sensor_reading_conversion() {
    lightSensor->init();
    
    // Set lights ON (digital HIGH)
    lightMockGpio->setDigitalValue(gpio_pins::LIGHTS, true);
    
    // Get the reading
    Reading lightReading = lightSensor->getReading();
    bool lightsOn = std::get<bool>(lightReading);
    
    // Lights should be on
    TEST_ASSERT_TRUE(lightsOn);
    
    // Set lights OFF (digital LOW)
    lightMockGpio->setDigitalValue(gpio_pins::LIGHTS, false);
    Reading lightReading2 = lightSensor->getReading();
    bool lightsOff = std::get<bool>(lightReading2);
    
    // Lights should be off
    TEST_ASSERT_FALSE(lightsOff);
}

void test_light_sensor_boundary_values() {
    lightSensor->init();
    
    // Test lights OFF state (digital LOW)
    lightMockGpio->setDigitalValue(gpio_pins::LIGHTS, false);
    Reading offReading = lightSensor->getReading();
    bool lightsOff = std::get<bool>(offReading);
    TEST_ASSERT_FALSE(lightsOff);
    
    // Test lights ON state (digital HIGH)
    lightMockGpio->setDigitalValue(gpio_pins::LIGHTS, true);
    Reading onReading = lightSensor->getReading();
    bool lightsOn = std::get<bool>(onReading);
    TEST_ASSERT_TRUE(lightsOn);
}

void test_light_sensor_value_change_detection() {
    lightSensor->init();
    
    // Set initial value (lights OFF)
    lightMockGpio->setDigitalValue(gpio_pins::LIGHTS, false);
    Reading lightReading1 = lightSensor->getReading();
    bool reading1 = std::get<bool>(lightReading1);
    
    // Same value should give same reading
    Reading lightReading2 = lightSensor->getReading();
    bool reading2 = std::get<bool>(lightReading2);
    TEST_ASSERT_EQUAL(reading1, reading2);
    
    // Different value should give different reading (lights ON)
    lightMockGpio->setDigitalValue(gpio_pins::LIGHTS, true);
    Reading lightReading3 = lightSensor->getReading();
    bool reading3 = std::get<bool>(lightReading3);
    TEST_ASSERT_NOT_EQUAL(reading1, reading3);
}

void test_light_sensor_construction() {
    // Test that lightSensor can be created and destroyed
    TEST_ASSERT_NOT_NULL(lightSensor);
}

void test_light_sensor_reading_consistency() {
    lightSensor->init();
    
    // Set a known value (lights ON)
    lightMockGpio->setDigitalValue(gpio_pins::LIGHTS, true);
    
    // Multiple readings should be consistent
    Reading reading1 = lightSensor->getReading();
    Reading reading2 = lightSensor->getReading();
    
    TEST_ASSERT_EQUAL(std::get<bool>(reading1), std::get<bool>(reading2));
    TEST_ASSERT_TRUE(std::get<bool>(reading1));
}

void test_light_sensor_state_transitions() {
    lightSensor->init();
    
    // Test OFF to ON transition
    lightMockGpio->setDigitalValue(gpio_pins::LIGHTS, false);
    Reading offReading = lightSensor->getReading();
    bool lightsOff = std::get<bool>(offReading);
    TEST_ASSERT_FALSE(lightsOff);
    
    // Switch to ON
    lightMockGpio->setDigitalValue(gpio_pins::LIGHTS, true);
    Reading onReading = lightSensor->getReading();
    bool lightsOn = std::get<bool>(onReading);
    TEST_ASSERT_TRUE(lightsOn);
    
    // Switch back to OFF
    lightMockGpio->setDigitalValue(gpio_pins::LIGHTS, false);
    Reading offReading2 = lightSensor->getReading();
    bool lightsOff2 = std::get<bool>(offReading2);
    TEST_ASSERT_FALSE(lightsOff2);
}

void runLightSensorTests() {
    setUp_light_sensor();
    RUN_TEST(test_light_sensor_construction);
    RUN_TEST(test_light_sensor_init);
    RUN_TEST(test_light_sensor_reading_conversion);
    RUN_TEST(test_light_sensor_boundary_values);
    RUN_TEST(test_light_sensor_value_change_detection);
    RUN_TEST(test_light_sensor_reading_consistency);
    RUN_TEST(test_light_sensor_state_transitions);
    tearDown_light_sensor();
}