#pragma once

// System/Library Includes
#include <functional>
#include <string>

// Project Includes
#include "interfaces/i_preference_service.h"
#include "utilities/types.h"

/**
 * @class MockPreferenceService
 * @brief Mock implementation of IPreferenceService for testing
 * 
 * @details Provides a testable implementation of the preference service interface
 * with controllable behavior for unit tests. Uses in-memory storage instead of
 * actual NVS operations, allowing for predictable test scenarios.
 * 
 * @testability Allows verification of config save/load operations
 * @dependency_free No actual NVS dependency - uses in-memory storage
 */
class MockPreferenceService : public IPreferenceService
{
public:
    MockPreferenceService();
    virtual ~MockPreferenceService() = default;

    // Core Functionality Methods
    void init() override;
    void saveConfig() override;
    void loadConfig() override;
    void createDefaultConfig() override;

    // Configuration Access Methods
    Configs& getConfig() override;
    const Configs& getConfig() const override;
    void setConfig(const Configs& config) override;

    // Test Helper Methods
    bool wasInitCalled() const { return initCalled_; }
    bool wasSaveConfigCalled() const { return saveConfigCalled_; }
    bool wasLoadConfigCalled() const { return loadConfigCalled_; }
    bool wasCreateDefaultConfigCalled() const { return createDefaultConfigCalled_; }
    int getSaveCount() const { return saveCount_; }
    int getLoadCount() const { return loadCount_; }

    // Test Control Methods
    void reset();
    void simulateLoadFailure(bool shouldFail) { simulateLoadFailure_ = shouldFail; }
    void simulateSaveFailure(bool shouldFail) { simulateSaveFailure_ = shouldFail; }
    void setInitCallback(std::function<void()> callback) { initCallback_ = callback; }
    void setSaveCallback(std::function<void(const Configs&)> callback) { saveCallback_ = callback; }
    void setLoadCallback(std::function<void()> callback) { loadCallback_ = callback; }

    // Test Data Access
    const Configs& getLastSavedConfig() const { return lastSavedConfig_; }
    bool hasStoredConfig() const { return hasStoredConfig_; }

private:
    // Configuration Storage
    Configs currentConfig_;
    Configs lastSavedConfig_;
    bool hasStoredConfig_;

    // State Tracking
    bool initCalled_;
    bool saveConfigCalled_;
    bool loadConfigCalled_;
    bool createDefaultConfigCalled_;
    int saveCount_;
    int loadCount_;

    // Test Control
    bool simulateLoadFailure_;
    bool simulateSaveFailure_;

    // Test Callbacks
    std::function<void()> initCallback_;
    std::function<void(const Configs&)> saveCallback_;
    std::function<void()> loadCallback_;

    void createDefaults();
};