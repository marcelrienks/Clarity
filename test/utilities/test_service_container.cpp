// Project Includes
#include "test_service_container.h"

TestServiceContainer::TestServiceContainer()
    : ServiceContainer()
{
}

void TestServiceContainer::reset()
{
    // Clear all registered services from the base container
    // This leverages the ServiceContainer's existing reset functionality
    ServiceContainer::reset();
    
    // Clear our tracking
    registeredTypes_.clear();
}

size_t TestServiceContainer::getServiceCount() const
{
    return registeredTypes_.size();
}

void TestServiceContainer::trackRegistration(const std::string& typeName)
{
    registeredTypes_.push_back(typeName);
}