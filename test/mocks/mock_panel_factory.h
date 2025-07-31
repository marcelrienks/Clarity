#pragma once

// System/Library Includes
#include <memory>
#include <string>
#include <map>
#include <functional>

// Project Includes
#include "interfaces/i_panel_factory.h"
#include "interfaces/i_panel.h"
#include "mock_panel.h"

/**
 * @class MockPanelFactory
 * @brief Mock implementation of IPanelFactory for testing
 * 
 * @details Provides a controllable mock implementation of the panel factory
 * interface for unit testing. Supports configurable panel creation behavior,
 * panel type support checking, and call tracking for verification.
 * 
 * @testing_features
 * - Configurable panel creation via lambda functions
 * - Support checking for panel types
 * - Call counting for verification
 * - Easy integration with Unity test framework
 */
class MockPanelFactory : public IPanelFactory
{
public:
    MockPanelFactory();
    virtual ~MockPanelFactory() = default;

    // IPanelFactory interface implementation
    std::unique_ptr<IPanel> createPanel(const std::string& panelType) override;
    bool supportsPanel(const std::string& panelType) const override;

    // Mock configuration methods
    void setCreatePanelBehavior(const std::string& panelType, 
                               std::function<std::unique_ptr<IPanel>()> creator);
    void setSupportedPanel(const std::string& panelType, bool supported = true);
    void setDefaultCreator(std::function<std::unique_ptr<IPanel>()> creator);

    // Test verification methods
    int getCreatePanelCallCount() const { return createPanelCallCount_; }
    int getSupportsPanelCallCount() const { return supportsPanelCallCount_; }
    std::string getLastRequestedPanelType() const { return lastRequestedPanelType_; }
    
    // Reset methods for test isolation
    void reset();

private:
    std::map<std::string, std::function<std::unique_ptr<IPanel>()>> panelCreators_;
    std::map<std::string, bool> supportedPanels_;
    std::function<std::unique_ptr<IPanel>()> defaultCreator_;
    
    // Call tracking
    mutable int createPanelCallCount_;
    mutable int supportsPanelCallCount_;
    mutable std::string lastRequestedPanelType_;
};