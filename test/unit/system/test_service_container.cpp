#include <unity.h>
#include "system/service_container.h"
#include "utilities/types.h"

ServiceContainer* container = nullptr;

void setUp_service_container() {
    container = new ServiceContainer();
}

void tearDown_service_container() {
    delete container;
    container = nullptr;
}

void test_service_container_construction() {
    // Test that container can be created and destroyed
    TEST_ASSERT_NOT_NULL(container);
}

void test_service_container_singleton_instance() {
    // Test getting singleton instance
    ServiceContainer& instance1 = ServiceContainer::getInstance();
    ServiceContainer& instance2 = ServiceContainer::getInstance();
    
    // Should be the same instance
    TEST_ASSERT_EQUAL_PTR(&instance1, &instance2);
}

void test_service_container_register_service() {
    ServiceContainer& instance = ServiceContainer::getInstance();
    
    // Create a mock service
    auto mockService = std::make_shared<int>(42); // Simple int as mock service
    
    // Register service
    instance.registerService<int>("TestService", mockService);
    
    // Service should be registered
    TEST_ASSERT_TRUE(instance.hasService("TestService"));
}

void test_service_container_get_service() {
    ServiceContainer& instance = ServiceContainer::getInstance();
    
    // Create and register a mock service
    auto mockService = std::make_shared<int>(123);
    instance.registerService<int>("GetTestService", mockService);
    
    // Get service back
    auto retrievedService = instance.getService<int>("GetTestService");
    TEST_ASSERT_NOT_NULL(retrievedService);
    TEST_ASSERT_EQUAL(123, *retrievedService);
}

void test_service_container_has_service() {
    ServiceContainer& instance = ServiceContainer::getInstance();
    
    // Initially should not have service
    TEST_ASSERT_FALSE(instance.hasService("NonExistentService"));
    
    // Register service
    auto mockService = std::make_shared<double>(3.14);
    instance.registerService<double>("HasTestService", mockService);
    
    // Now should have service
    TEST_ASSERT_TRUE(instance.hasService("HasTestService"));
}

void test_service_container_unregister_service() {
    ServiceContainer& instance = ServiceContainer::getInstance();
    
    // Register service
    auto mockService = std::make_shared<std::string>("test");
    instance.registerService<std::string>("UnregisterTest", mockService);
    TEST_ASSERT_TRUE(instance.hasService("UnregisterTest"));
    
    // Unregister service
    instance.unregisterService("UnregisterTest");
    TEST_ASSERT_FALSE(instance.hasService("UnregisterTest"));
}

void test_service_container_multiple_services() {
    ServiceContainer& instance = ServiceContainer::getInstance();
    
    // Register multiple services of different types
    auto intService = std::make_shared<int>(42);
    auto stringService = std::make_shared<std::string>("hello");
    auto doubleService = std::make_shared<double>(2.71);
    
    instance.registerService<int>("IntService", intService);
    instance.registerService<std::string>("StringService", stringService);
    instance.registerService<double>("DoubleService", doubleService);
    
    // All should be registered
    TEST_ASSERT_TRUE(instance.hasService("IntService"));
    TEST_ASSERT_TRUE(instance.hasService("StringService"));
    TEST_ASSERT_TRUE(instance.hasService("DoubleService"));
    
    // Retrieve and verify
    auto retrievedInt = instance.getService<int>("IntService");
    auto retrievedString = instance.getService<std::string>("StringService");
    auto retrievedDouble = instance.getService<double>("DoubleService");
    
    TEST_ASSERT_EQUAL(42, *retrievedInt);
    TEST_ASSERT_EQUAL_STRING("hello", retrievedString->c_str());
    TEST_ASSERT_EQUAL_DOUBLE(2.71, *retrievedDouble);
}

void test_service_container_service_replacement() {
    ServiceContainer& instance = ServiceContainer::getInstance();
    
    // Register initial service
    auto service1 = std::make_shared<int>(100);
    instance.registerService<int>("ReplaceTest", service1);
    
    auto retrieved1 = instance.getService<int>("ReplaceTest");
    TEST_ASSERT_EQUAL(100, *retrieved1);
    
    // Replace with new service
    auto service2 = std::make_shared<int>(200);
    instance.registerService<int>("ReplaceTest", service2);
    
    auto retrieved2 = instance.getService<int>("ReplaceTest");
    TEST_ASSERT_EQUAL(200, *retrieved2);
}

void runServiceContainerTests() {
    setUp_service_container();
    RUN_TEST(test_service_container_construction);
    RUN_TEST(test_service_container_singleton_instance);
    RUN_TEST(test_service_container_register_service);
    RUN_TEST(test_service_container_get_service);
    RUN_TEST(test_service_container_has_service);
    RUN_TEST(test_service_container_unregister_service);
    RUN_TEST(test_service_container_multiple_services);
    RUN_TEST(test_service_container_service_replacement);
    tearDown_service_container();
}