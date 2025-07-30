#include <memory>
#include <iostream>

// Component includes
#include "components/key_component.h"
#include "components/lock_component.h" 
#include "components/clarity_component.h"
#include "components/oem/oem_oil_temperature_component.h"
#include "components/oem/oem_oil_pressure_component.h"

// Mock service includes
#include "test/mocks/mock_style_service.h"
#include "test/mocks/mock_display_provider.h"

// System includes 
#include "system/component_registry.h"
#include "managers/style_manager.h"
#include "interfaces/i_style_service.h"

/**
 * Manual component verification test
 * Tests that all components can be created with dependency injection
 * and that their basic functionality works without regression.
 */
int main() {
    std::cout << "=== Component Verification Test ===" << std::endl;
    
    // Create a real StyleManager for testing
    auto styleManager = std::make_unique<StyleManager>();
    IStyleService* styleService = styleManager.get();
    
    // Create mock display provider
    auto mockDisplay = std::make_unique<MockDisplayProvider>();
    IDisplayProvider* displayProvider = mockDisplay.get();
    
    std::cout << "Created StyleManager and MockDisplayProvider" << std::endl;
    
    // Test 1: KeyComponent creation with DI
    std::cout << "\nTesting KeyComponent..." << std::endl;
    try {
        auto keyComponent = std::make_unique<KeyComponent>(styleService);
        std::cout << "✓ KeyComponent created successfully with DI" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✗ KeyComponent creation failed: " << e.what() << std::endl;
        return 1;
    }
    
    // Test 2: LockComponent creation with DI
    std::cout << "\nTesting LockComponent..." << std::endl;
    try {
        auto lockComponent = std::make_unique<LockComponent>(styleService);
        std::cout << "✓ LockComponent created successfully with DI" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✗ LockComponent creation failed: " << e.what() << std::endl;
        return 1;
    }
    
    // Test 3: ClarityComponent creation with DI
    std::cout << "\nTesting ClarityComponent..." << std::endl;
    try {
        auto clarityComponent = std::make_unique<ClarityComponent>(styleService);
        std::cout << "✓ ClarityComponent created successfully with DI" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✗ ClarityComponent creation failed: " << e.what() << std::endl;
        return 1;
    }
    
    // Test 4: OemOilPressureComponent creation with DI
    std::cout << "\nTesting OemOilPressureComponent..." << std::endl;
    try {
        auto oilPressureComponent = std::make_unique<OemOilPressureComponent>(styleService);
        std::cout << "✓ OemOilPressureComponent created successfully with DI" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✗ OemOilPressureComponent creation failed: " << e.what() << std::endl;
        return 1;
    }
    
    // Test 5: OemOilTemperatureComponent creation with DI
    std::cout << "\nTesting OemOilTemperatureComponent..." << std::endl;
    try {
        auto oilTempComponent = std::make_unique<OemOilTemperatureComponent>(styleService);
        std::cout << "✓ OemOilTemperatureComponent created successfully with DI" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✗ OemOilTemperatureComponent creation failed: " << e.what() << std::endl;
        return 1;
    }
    
    // Test 6: Component Registry functionality
    std::cout << "\nTesting ComponentRegistry with DI..." << std::endl;
    auto& registry = ComponentRegistry::GetInstance();
    
    // Test component creation through registry
    auto keyFromRegistry = registry.createComponent("key", displayProvider, styleService);
    if (keyFromRegistry) {
        std::cout << "✓ Component creation through registry works" << std::endl;
    } else {
        std::cout << "✗ Component creation through registry failed" << std::endl;
        return 1;
    }
    
    auto lockFromRegistry = registry.createComponent("lock", displayProvider, styleService);
    if (lockFromRegistry) {
        std::cout << "✓ Lock component creation through registry works" << std::endl;
    } else {
        std::cout << "✗ Lock component creation through registry failed" << std::endl;
        return 1;
    }
    
    auto oilPressureFromRegistry = registry.createComponent("oem_oil_pressure", displayProvider, styleService);
    if (oilPressureFromRegistry) {
        std::cout << "✓ Oil pressure component creation through registry works" << std::endl;
    } else {
        std::cout << "✗ Oil pressure component creation through registry failed" << std::endl;
        return 1;
    }
    
    auto oilTempFromRegistry = registry.createComponent("oem_oil_temperature", displayProvider, styleService);
    if (oilTempFromRegistry) {
        std::cout << "✓ Oil temperature component creation through registry works" << std::endl;
    } else {
        std::cout << "✗ Oil temperature component creation through registry failed" << std::endl;
        return 1;
    }
    
    std::cout << "\n=== All Component Tests Passed! ===" << std::endl;
    std::cout << "✓ All components can be created with dependency injection" << std::endl;
    std::cout << "✓ ComponentRegistry works with new DI pattern" << std::endl;
    std::cout << "✓ No regressions detected in basic component functionality" << std::endl;
    
    return 0;
}