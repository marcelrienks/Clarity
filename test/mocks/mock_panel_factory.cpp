// System/Library Includes
#include <stdexcept>

// Project Includes
#include "mock_panel_factory.h"

MockPanelFactory::MockPanelFactory()
    : createPanelCallCount_(0)
    , supportsPanelCallCount_(0)
{
    // Set up default creator that returns a basic mock panel
    defaultCreator_ = []() {
        return std::make_unique<MockPanel>("MockPanel");
    };
    
    // Set up default supported panels
    setSupportedPanel("splash", true);
    setSupportedPanel("oil", true);
    setSupportedPanel("key", true);
    setSupportedPanel("lock", true);
}

std::unique_ptr<IPanel> MockPanelFactory::createPanel(const std::string& panelType)
{
    createPanelCallCount_++;
    lastRequestedPanelType_ = panelType;
    
    auto creator = panelCreators_.find(panelType);
    if (creator != panelCreators_.end()) {
        return creator->second();
    }
    
    if (defaultCreator_) {
        return defaultCreator_();
    }
    
    throw std::runtime_error("MockPanelFactory: No creator configured for panel type: " + panelType);
}

bool MockPanelFactory::supportsPanel(const std::string& panelType) const
{
    supportsPanelCallCount_++;
    
    auto it = supportedPanels_.find(panelType);
    return (it != supportedPanels_.end()) ? it->second : false;
}

void MockPanelFactory::setCreatePanelBehavior(const std::string& panelType, 
                                              std::function<std::unique_ptr<IPanel>()> creator)
{
    panelCreators_[panelType] = creator;
}

void MockPanelFactory::setSupportedPanel(const std::string& panelType, bool supported)
{
    supportedPanels_[panelType] = supported;
}

void MockPanelFactory::setDefaultCreator(std::function<std::unique_ptr<IPanel>()> creator)
{
    defaultCreator_ = creator;
}

void MockPanelFactory::reset()
{
    createPanelCallCount_ = 0;
    supportsPanelCallCount_ = 0;
    lastRequestedPanelType_.clear();
    panelCreators_.clear();
    supportedPanels_.clear();
    
    // Restore defaults
    defaultCreator_ = []() {
        return std::make_unique<MockPanel>("MockPanel");
    };
    
    setSupportedPanel("splash", true);
    setSupportedPanel("oil", true);
    setSupportedPanel("key", true);
    setSupportedPanel("lock", true);
}