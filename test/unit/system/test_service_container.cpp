#include <unity.h>
#include <memory>
#include <string>

// Project Includes
#include "system/service_container.h"

// Test interfaces for testing
class ITestService
{
public:
    virtual ~ITestService() = default;
    virtual std::string getName() const = 0;
    virtual int getValue() const = 0;
};

class ITestDependency
{
public:
    virtual ~ITestDependency() = default;
    virtual std::string getInfo() const = 0;
};

// Test implementations
class TestService : public ITestService
{
private:
    std::string name_;
    int value_;
    
public:
    TestService(const std::string& name, int value) 
        : name_(name), value_(value) {}
        
    std::string getName() const override { return name_; }
    int getValue() const override { return value_; }
};

class TestDependency : public ITestDependency
{
private:
    std::string info_;
    
public:
    TestDependency(const std::string& info) : info_(info) {}
    std::string getInfo() const override { return info_; }
};

class TestServiceWithDependency : public ITestService
{
private:
    std::string name_;
    ITestDependency* dependency_;
    
public:
    TestServiceWithDependency(const std::string& name, ITestDependency* dep)
        : name_(name), dependency_(dep) {}
        
    std::string getName() const override { 
        return name_ + " (with " + dependency_->getInfo() + ")"; 
    }
    int getValue() const override { return 42; }
};

// Global container for tests
static std::unique_ptr<ServiceContainer> container;

void setUp(void)
{
    container = std::make_unique<ServiceContainer>();
}

void tearDown(void)
{
    container.reset();
}

// Test singleton registration and resolution
void test_singleton_registration_and_resolution(void)
{
    // Register a singleton service
    container->registerSingleton<ITestService>([]() {
        return std::make_unique<TestService>("SingletonService", 123);
    });
    
    // Check that service is registered
    TEST_ASSERT_TRUE(container->isRegistered<ITestService>());
    
    // Resolve the service twice - should get same instance
    ITestService* service1 = container->resolve<ITestService>();
    ITestService* service2 = container->resolve<ITestService>();
    
    TEST_ASSERT_NOT_NULL(service1);
    TEST_ASSERT_NOT_NULL(service2);
    TEST_ASSERT_EQUAL_PTR(service1, service2); // Same instance
    TEST_ASSERT_EQUAL_STRING("SingletonService", service1->getName().c_str());
    TEST_ASSERT_EQUAL_INT(123, service1->getValue());
}

// Test transient registration and creation
void test_transient_registration_and_creation(void)
{
    // Register a transient service
    container->registerTransient<ITestService>([](IServiceContainer* c) {
        return std::make_unique<TestService>("TransientService", 456);
    });
    
    // Check that service is registered
    TEST_ASSERT_TRUE(container->isRegistered<ITestService>());
    
    // Create two instances - should get different instances
    auto service1 = container->create<ITestService>();
    auto service2 = container->create<ITestService>();
    
    TEST_ASSERT_NOT_NULL(service1.get());
    TEST_ASSERT_NOT_NULL(service2.get());
    TEST_ASSERT_NOT_EQUAL(service1.get(), service2.get()); // Different instances
    TEST_ASSERT_EQUAL_STRING("TransientService", service1->getName().c_str());
    TEST_ASSERT_EQUAL_STRING("TransientService", service2->getName().c_str());
    TEST_ASSERT_EQUAL_INT(456, service1->getValue());
    TEST_ASSERT_EQUAL_INT(456, service2->getValue());
}

// Test transient resolve throws exception
void test_transient_resolve_throws_exception(void)
{
    // Register a transient service
    container->registerTransient<ITestService>([](IServiceContainer* c) {
        return std::make_unique<TestService>("TransientService", 456);
    });
    
    // Attempting to resolve transient should throw
    try {
        container->resolve<ITestService>();
        TEST_FAIL_MESSAGE("Expected exception when resolving transient service");
    } catch (const std::runtime_error& e) {
        // Expected behavior
        TEST_ASSERT_TRUE(std::string(e.what()).find("transient") != std::string::npos);
    }
}

// Test service with dependency injection
void test_service_with_dependency_injection(void)
{
    // Register dependency as singleton
    container->registerSingleton<ITestDependency>([]() {
        return std::make_unique<TestDependency>("Dependency1");
    });
    
    // Register service that uses the dependency
    container->registerSingleton<ITestService>([](/* no container param for singleton */) {
        // For this test, we'll create a simplified version
        return std::make_unique<TestService>("ServiceWithoutDep", 789);
    });
    
    // Register a transient service that can access the container
    container->registerTransient<ITestService>([](IServiceContainer* c) {
        ITestDependency* dep = c->resolve<ITestDependency>();
        return std::make_unique<TestServiceWithDependency>("ServiceWithDep", dep);
    });
    
    // Create the service with dependency
    auto service = container->create<ITestService>();
    
    TEST_ASSERT_NOT_NULL(service.get());
    TEST_ASSERT_EQUAL_STRING("ServiceWithDep (with Dependency1)", service->getName().c_str());
    TEST_ASSERT_EQUAL_INT(42, service->getValue());
}

// Test unregistered service throws exception
void test_unregistered_service_throws_exception(void)
{
    // Try to resolve unregistered service
    TEST_ASSERT_FALSE(container->isRegistered<ITestService>());
    
    try {
        container->resolve<ITestService>();
        TEST_FAIL_MESSAGE("Expected exception when resolving unregistered service");
    } catch (const std::runtime_error& e) {
        // Expected behavior
        TEST_ASSERT_TRUE(std::string(e.what()).find("not registered") != std::string::npos);
    }
    
    try {
        container->create<ITestService>();
        TEST_FAIL_MESSAGE("Expected exception when creating unregistered service");
    } catch (const std::runtime_error& e) {
        // Expected behavior
        TEST_ASSERT_TRUE(std::string(e.what()).find("not registered") != std::string::npos);
    }
}

// Test container clear functionality
void test_container_clear(void)
{
    // Register some services
    container->registerSingleton<ITestService>([]() {
        return std::make_unique<TestService>("Service1", 100);
    });
    
    container->registerSingleton<ITestDependency>([]() {
        return std::make_unique<TestDependency>("Dependency1");
    });
    
    TEST_ASSERT_TRUE(container->isRegistered<ITestService>());
    TEST_ASSERT_TRUE(container->isRegistered<ITestDependency>());
    
    // Clear the container
    container->clear();
    
    // Services should no longer be registered
    TEST_ASSERT_FALSE(container->isRegistered<ITestService>());
    TEST_ASSERT_FALSE(container->isRegistered<ITestDependency>());
}

// Test singleton via create method
void test_singleton_via_create_method(void)
{
    // Register a singleton service
    container->registerSingleton<ITestService>([]() {
        return std::make_unique<TestService>("SingletonViaCreate", 999);
    });
    
    // Create instances via create() method - should get new instances even for singletons
    auto service1 = container->create<ITestService>();
    auto service2 = container->create<ITestService>();
    
    TEST_ASSERT_NOT_NULL(service1.get());
    TEST_ASSERT_NOT_NULL(service2.get());
    TEST_ASSERT_NOT_EQUAL(service1.get(), service2.get()); // Different instances
    TEST_ASSERT_EQUAL_STRING("SingletonViaCreate", service1->getName().c_str());
    TEST_ASSERT_EQUAL_STRING("SingletonViaCreate", service2->getName().c_str());
    
    // But resolve() should still return the cached singleton
    ITestService* resolved = container->resolve<ITestService>();
    TEST_ASSERT_NOT_NULL(resolved);
    TEST_ASSERT_NOT_EQUAL(service1.get(), resolved); // Different from created instances
    TEST_ASSERT_NOT_EQUAL(service2.get(), resolved); // Different from created instances
}

// Unity test runner setup
void run_service_container_tests(void)
{
    RUN_TEST(test_singleton_registration_and_resolution);
    RUN_TEST(test_transient_registration_and_creation);
    RUN_TEST(test_transient_resolve_throws_exception);
    RUN_TEST(test_service_with_dependency_injection);
    RUN_TEST(test_unregistered_service_throws_exception);
    RUN_TEST(test_container_clear);
    RUN_TEST(test_singleton_via_create_method);
}