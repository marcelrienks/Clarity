// Simple build verification for DI test system
// This file includes all the key headers to verify compilation

#include "system/service_container.h"
#include "interfaces/i_style_service.h"
#include "interfaces/i_preference_service.h"
#include "interfaces/i_trigger_service.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_component_factory.h"

#include "mock_style_service.h"
#include "mock_preference_service.h"
#include "mock_trigger_service.h"
#include "mock_panel_service.h"
#include "mock_component_factory.h"

#include "components/key_component.h"
#include "components/lock_component.h"
#include "components/clarity_component.h"

#include <memory>
#include <iostream>

int main() {
    std::cout << "DI Test System Compilation Verification" << std::endl;
    
    // Test service container creation
    auto container = std::make_unique<ServiceContainer>();
    std::cout << "✓ ServiceContainer created" << std::endl;
    
    // Test mock service creation
    auto mockStyle = std::make_unique<MockStyleService>();
    auto mockPref = std::make_unique<MockPreferenceService>();
    auto mockTrigger = std::make_unique<MockTriggerService>();
    auto mockPanel = std::make_unique<MockPanelService>();
    auto mockFactory = std::make_unique<MockComponentFactory>();
    std::cout << "✓ Mock services created" << std::endl;
    
    // Test component creation with DI
    auto keyComponent = std::make_unique<KeyComponent>(mockStyle.get());
    auto lockComponent = std::make_unique<LockComponent>(mockStyle.get());
    auto clarityComponent = std::make_unique<ClarityComponent>(mockStyle.get());
    std::cout << "✓ Components created with dependency injection" << std::endl;
    
    std::cout << "✓ All DI system components compile successfully!" << std::endl;
    
    return 0;
}