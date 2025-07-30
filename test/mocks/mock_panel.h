#pragma once

// System/Library Includes
#include <string>
#include <functional>

// Project Includes
#include "interfaces/i_panel.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"

/**
 * @class MockPanel
 * @brief Mock implementation of IPanel for testing
 */
class MockPanel : public IPanel
{
public:
    MockPanel(const std::string& name) 
        : name_(name)
        , initCalled_(false)
        , loadCalled_(false)
        , updateCalled_(false)
        , loadCallCount_(0)
        , updateCallCount_(0)
    {}

    virtual ~MockPanel() = default;

    // IPanel Methods
    void init(IGpioProvider* gpio, IDisplayProvider* display) override 
    {
        initCalled_ = true;
        lastGpio_ = gpio;
        lastDisplay_ = display;
    }

    void load(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display) override 
    {
        loadCalled_ = true;
        loadCallCount_++;
        callbackFunction_ = callbackFunction;
        lastGpio_ = gpio;
        lastDisplay_ = display;
        
        // Execute callback immediately unless test specifies otherwise
        if (executeCallbackImmediately_ && callbackFunction_) {
            callbackFunction_();
        }
    }

    void update(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display) override 
    {
        updateCalled_ = true;
        updateCallCount_++;
        callbackFunction_ = callbackFunction;
        lastGpio_ = gpio;
        lastDisplay_ = display;
        
        // Execute callback immediately unless test specifies otherwise
        if (executeCallbackImmediately_ && callbackFunction_) {
            callbackFunction_();
        }
    }

    // Test Helper Methods
    const std::string& getName() const { return name_; }
    bool wasInitCalled() const { return initCalled_; }
    bool wasLoadCalled() const { return loadCalled_; }
    bool wasUpdateCalled() const { return updateCalled_; }
    int getLoadCallCount() const { return loadCallCount_; }
    int getUpdateCallCount() const { return updateCallCount_; }
    IGpioProvider* getLastGpio() const { return lastGpio_; }
    IDisplayProvider* getLastDisplay() const { return lastDisplay_; }

    // Test Control Methods
    void setExecuteCallbackImmediately(bool execute) { executeCallbackImmediately_ = execute; }
    void executeStoredCallback() 
    {
        if (callbackFunction_) {
            callbackFunction_();
        }
    }

    void reset() 
    {
        initCalled_ = false;
        loadCalled_ = false;
        updateCalled_ = false;
        loadCallCount_ = 0;
        updateCallCount_ = 0;
        lastGpio_ = nullptr;
        lastDisplay_ = nullptr;
        callbackFunction_ = nullptr;
        executeCallbackImmediately_ = true;
    }

private:
    std::string name_;
    bool initCalled_;
    bool loadCalled_;
    bool updateCalled_;
    int loadCallCount_;
    int updateCallCount_;
    IGpioProvider* lastGpio_;
    IDisplayProvider* lastDisplay_;
    bool executeCallbackImmediately_ = true;
};