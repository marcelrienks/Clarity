#include <iostream>
#include <memory>
#include <string>

// Project Includes
#include "system/service_container.h"

// Test interfaces
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
        : name_(name), value_(value) 
    {
        std::cout << "Creating TestService: " << name_ << " with value " << value_ << std::endl;
    }
    
    ~TestService() 
    {
        std::cout << "Destroying TestService: " << name_ << std::endl;
    }
        
    std::string getName() const override { return name_; }
    int getValue() const override { return value_; }
};

class TestDependency : public ITestDependency
{
private:
    std::string info_;
    
public:
    TestDependency(const std::string& info) : info_(info) 
    {
        std::cout << "Creating TestDependency: " << info_ << std::endl;
    }
    
    ~TestDependency() 
    {
        std::cout << "Destroying TestDependency: " << info_ << std::endl;
    }
    
    std::string getInfo() const override { return info_; }
};

class TestServiceWithDependency : public ITestService
{
private:
    std::string name_;
    ITestDependency* dependency_;
    
public:
    TestServiceWithDependency(const std::string& name, ITestDependency* dep)
        : name_(name), dependency_(dep) 
    {
        std::cout << "Creating TestServiceWithDependency: " << name_ << std::endl;
    }
    
    ~TestServiceWithDependency() 
    {
        std::cout << "Destroying TestServiceWithDependency: " << name_ << std::endl;
    }
        
    std::string getName() const override { 
        return name_ + " (with " + dependency_->getInfo() + ")"; 
    }
    int getValue() const override { return 42; }
};

void testSingletonService() 
{
    std::cout << "\n=== Testing Singleton Service ===\n";
    
    ServiceContainer container;
    
    // Register a singleton service
    container.registerSingleton<ITestService>([]() {
        return std::make_unique<TestService>("SingletonService", 123);
    });
    
    std::cout << "Service registered: " << (container.isRegistered<ITestService>() ? "YES" : "NO") << std::endl;
    
    // Resolve the service twice - should get same instance
    ITestService* service1 = container.resolve<ITestService>();
    ITestService* service2 = container.resolve<ITestService>();
    
    std::cout << "Service1 address: " << service1 << std::endl;
    std::cout << "Service2 address: " << service2 << std::endl;
    std::cout << "Same instance: " << (service1 == service2 ? "YES" : "NO") << std::endl;
    std::cout << "Service1 name: " << service1->getName() << std::endl;
    std::cout << "Service1 value: " << service1->getValue() << std::endl;
}

void testTransientService() 
{
    std::cout << "\n=== Testing Transient Service ===\n";
    
    ServiceContainer container;
    
    // Register a transient service
    container.registerTransient<ITestService>([](IServiceContainer* c) {
        return std::make_unique<TestService>("TransientService", 456);
    });
    
    std::cout << "Service registered: " << (container.isRegistered<ITestService>() ? "YES" : "NO") << std::endl;
    
    // Create two instances - should get different instances
    auto service1 = container.create<ITestService>();
    auto service2 = container.create<ITestService>();
    
    std::cout << "Service1 address: " << service1.get() << std::endl;
    std::cout << "Service2 address: " << service2.get() << std::endl;
    std::cout << "Different instances: " << (service1.get() != service2.get() ? "YES" : "NO") << std::endl;
    std::cout << "Service1 name: " << service1->getName() << std::endl;
    std::cout << "Service2 name: " << service2->getName() << std::endl;
}

void testDependencyInjection() 
{
    std::cout << "\n=== Testing Dependency Injection ===\n";
    
    ServiceContainer container;
    
    // Register dependency as singleton
    container.registerSingleton<ITestDependency>([]() {
        return std::make_unique<TestDependency>("Dependency1");
    });
    
    // Register service that uses the dependency
    container.registerTransient<ITestService>([](IServiceContainer* c) {
        ITestDependency* dep = c->resolve<ITestDependency>();
        return std::make_unique<TestServiceWithDependency>("ServiceWithDep", dep);
    });
    
    // Create the service with dependency
    auto service = container.create<ITestService>();
    
    std::cout << "Service created successfully: " << (service.get() != nullptr ? "YES" : "NO") << std::endl;
    std::cout << "Service name: " << service->getName() << std::endl;
    std::cout << "Service value: " << service->getValue() << std::endl;
}

void testExceptionHandling() 
{
    std::cout << "\n=== Testing Exception Handling ===\n";
    
    ServiceContainer container;
    
    try {
        // Try to resolve unregistered service
        container.resolve<ITestService>();
        std::cout << "ERROR: Should have thrown exception" << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << "Correctly caught exception: " << e.what() << std::endl;
    }
    
    // Register transient service
    container.registerTransient<ITestService>([](IServiceContainer* c) {
        return std::make_unique<TestService>("TransientService", 789);
    });
    
    try {
        // Try to resolve transient service (should throw)
        container.resolve<ITestService>();
        std::cout << "ERROR: Should have thrown exception for transient resolve" << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << "Correctly caught transient resolve exception: " << e.what() << std::endl;
    }
}

int main() 
{
    std::cout << "ServiceContainer Manual Test\n";
    std::cout << "============================\n";
    
    testSingletonService();
    testTransientService();
    testDependencyInjection();
    testExceptionHandling();
    
    std::cout << "\n=== All Tests Completed ===\n";
    return 0;
}