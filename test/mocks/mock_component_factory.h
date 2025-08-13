#pragma once
#include "interfaces/i_component_factory.h"
#include <gmock/gmock.h>

class MockComponentFactory : public IComponentFactory {
public:
    MOCK_METHOD(std::unique_ptr<ClarityComponent>, CreateClarityComponent, (IStyleService*), (override));
    MOCK_METHOD(std::unique_ptr<IComponent>, CreateOilPressureComponent, (IStyleService*), (override));
    MOCK_METHOD(std::unique_ptr<IComponent>, CreateOilTemperatureComponent, (IStyleService*), (override));
    MOCK_METHOD(std::unique_ptr<ErrorComponent>, CreateErrorComponent, (IStyleService*), (override));
    MOCK_METHOD(std::unique_ptr<KeyComponent>, CreateKeyComponent, (IStyleService*), (override));
    MOCK_METHOD(std::unique_ptr<LockComponent>, CreateLockComponent, (IStyleService*), (override));
    MOCK_METHOD(std::unique_ptr<ConfigComponent>, CreateConfigComponent, (IStyleService*), (override));
};