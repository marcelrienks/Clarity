#pragma once

// System/Library Includes
#include <functional>
#include <vector>
#include <string>
#include <memory>

// Project Includes
#include "interfaces/i_panel_service.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include "utilities/types.h"

/**
 * @class MockPanelService
 * @brief Mock implementation of IPanelService for testing
 * 
 * @details Provides a testable implementation of the panel service interface
 * with controllable behavior for unit tests. Simulates panel loading and
 * transitions without requiring actual panel implementations.
 * 
 * @testability Allows verification of panel lifecycle management
 * @dependency_free No actual panel dependency - uses mock implementations
 */
class MockPanelService : public IPanelService
{
public:
    struct PanelLoadEvent {
        std::string panelName;
        bool isTriggerDriven;
        bool withSplash;
        uint32_t timestamp;
    };

    MockPanelService();
    virtual ~MockPanelService() = default;

    // Core Functionality Methods
    void init() override;
    void init(IGpioProvider* gpio, IDisplayProvider* display) override;
    void createAndLoadPanel(const char* panelName, 
                           std::function<void()> completionCallback = nullptr,
                           bool isTriggerDriven = false) override;
    void createAndLoadPanelWithSplash(const char* panelName) override;
    void updatePanel() override;

    // State Management Methods
    void setUiState(UIState state) override;
    const char* getCurrentPanel() const override;
    const char* getRestorationPanel() const override;

    // Trigger Integration Methods
    void triggerPanelSwitchCallback(const char* triggerId) override;

    // Test Helper Methods
    bool wasInitCalled() const { return initCalled_; }
    bool wasInitWithProvidersCalled() const { return initWithProvidersCalled_; }
    bool wasCreateAndLoadPanelCalled() const { return createAndLoadPanelCalled_; }
    bool wasCreateAndLoadPanelWithSplashCalled() const { return createAndLoadPanelWithSplashCalled_; }
    bool wasUpdatePanelCalled() const { return updatePanelCalled_; }
    bool wasTriggerPanelSwitchCallbackCalled() const { return triggerPanelSwitchCallbackCalled_; }
    
    int getUpdatePanelCallCount() const { return updatePanelCallCount_; }
    int getPanelLoadCount() const { return panelLoadCount_; }
    UIState getCurrentUiState() const { return currentUiState_; }

    // Test Control Methods
    void reset();
    void setCurrentPanel(const char* panelName);
    void setRestorationPanel(const char* panelName);
    void simulateLoadFailure(bool shouldFail) { simulateLoadFailure_ = shouldFail; }
    void setLoadDelay(int milliseconds) { loadDelay_ = milliseconds; }
    
    // Test Callbacks
    void setInitCallback(std::function<void()> callback) { initCallback_ = callback; }
    void setLoadCallback(std::function<void(const char*)> callback) { loadCallback_ = callback; }
    void setUpdateCallback(std::function<void()> callback) { updateCallback_ = callback; }
    void setTriggerCallback(std::function<void(const char*)> callback) { triggerCallback_ = callback; }

    // Test Data Access
    const std::vector<PanelLoadEvent>& getPanelLoadHistory() const { return panelLoadHistory_; }
    const std::vector<std::string>& getTriggerCallbackHistory() const { return triggerCallbackHistory_; }
    std::function<void()> getLastCompletionCallback() const { return lastCompletionCallback_; }

private:
    // Mock State
    std::string currentPanel_;
    std::string restorationPanel_;
    UIState currentUiState_;
    IGpioProvider* mockGpioProvider_;
    IDisplayProvider* mockDisplayProvider_;

    // State Tracking
    bool initCalled_;
    bool initWithProvidersCalled_;
    bool createAndLoadPanelCalled_;
    bool createAndLoadPanelWithSplashCalled_;
    bool updatePanelCalled_;
    bool triggerPanelSwitchCallbackCalled_;
    
    int updatePanelCallCount_;
    int panelLoadCount_;

    // Test Control
    bool simulateLoadFailure_;
    int loadDelay_;

    // Test Data
    std::vector<PanelLoadEvent> panelLoadHistory_;
    std::vector<std::string> triggerCallbackHistory_;
    std::function<void()> lastCompletionCallback_;

    // Test Callbacks
    std::function<void()> initCallback_;
    std::function<void(const char*)> loadCallback_;
    std::function<void()> updateCallback_;
    std::function<void(const char*)> triggerCallback_;

    void recordPanelLoad(const char* panelName, bool isTriggerDriven, bool withSplash);
    void executeCompletionCallback();
};