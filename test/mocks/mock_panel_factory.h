#pragma once
#include "interfaces/i_panel_factory.h"
#include <gmock/gmock.h>

class MockPanelFactory : public IPanelFactory {
public:
    MOCK_METHOD(std::unique_ptr<IPanel>, CreateSplashPanel, 
                (IGpioProvider*, IDisplayProvider*, IStyleService*), (override));
    MOCK_METHOD(std::unique_ptr<IPanel>, CreateOemOilPanel, 
                (IGpioProvider*, IDisplayProvider*, IStyleService*), (override));
    MOCK_METHOD(std::unique_ptr<IPanel>, CreateErrorPanel, 
                (IGpioProvider*, IDisplayProvider*, IStyleService*), (override));
    MOCK_METHOD(std::unique_ptr<IPanel>, CreateConfigPanel, 
                (IGpioProvider*, IDisplayProvider*, IStyleService*), (override));
    MOCK_METHOD(std::unique_ptr<IPanel>, CreateKeyPanel, 
                (IGpioProvider*, IDisplayProvider*, IStyleService*), (override));
    MOCK_METHOD(std::unique_ptr<IPanel>, CreateLockPanel, 
                (IGpioProvider*, IDisplayProvider*, IStyleService*), (override));
};