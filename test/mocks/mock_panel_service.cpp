#include "mock_panel_service.h"
#include "mock_types.h"
#include <chrono>
#include <thread>

MockPanelService::MockPanelService()
    : currentPanel_(PanelNames::OIL)
    , restorationPanel_(PanelNames::OIL)
    , currentUiState_(UIState::IDLE)
    , mockGpioProvider_(nullptr)
    , mockDisplayProvider_(nullptr)
    , initCalled_(false)
    , initWithProvidersCalled_(false)
    , createAndLoadPanelCalled_(false)
    , createAndLoadPanelWithSplashCalled_(false)
    , updatePanelCalled_(false)
    , triggerPanelSwitchCallbackCalled_(false)
    , updatePanelCallCount_(0)
    , panelLoadCount_(0)
    , simulateLoadFailure_(false)
    , loadDelay_(0)
{
}

void MockPanelService::init()
{
    initCalled_ = true;
    
    if (initCallback_) {
        initCallback_();
    }
}

void MockPanelService::init(IGpioProvider* gpio, IDisplayProvider* display)
{
    initWithProvidersCalled_ = true;
    mockGpioProvider_ = gpio;
    mockDisplayProvider_ = display;
    
    // Also call the regular init
    init();
}

void MockPanelService::createAndLoadPanel(const char* panelName, 
                                         std::function<void()> completionCallback,
                                         bool isTriggerDriven)
{
    createAndLoadPanelCalled_ = true;
    panelLoadCount_++;
    lastCompletionCallback_ = completionCallback;
    
    if (simulateLoadFailure_) {
        // Don't change panel or execute callback on failure
        return;
    }
    
    recordPanelLoad(panelName, isTriggerDriven, false);
    
    if (panelName) {
        currentPanel_ = panelName;
    }
    
    if (loadCallback_) {
        loadCallback_(panelName);
    }
    
    // Simulate load delay if specified
    if (loadDelay_ > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(loadDelay_));
    }
    
    executeCompletionCallback();
}

void MockPanelService::createAndLoadPanelWithSplash(const char* panelName)
{
    createAndLoadPanelWithSplashCalled_ = true;
    panelLoadCount_++;
    
    if (simulateLoadFailure_) {
        return;
    }
    
    recordPanelLoad(panelName, false, true);
    
    // Simulate splash sequence: splash -> target panel
    currentPanel_ = "SplashPanel";
    if (loadCallback_) {
        loadCallback_("SplashPanel");
    }
    
    // Simulate splash duration
    if (loadDelay_ > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(loadDelay_));
    }
    
    // Then load target panel
    if (panelName) {
        currentPanel_ = panelName;
        if (loadCallback_) {
            loadCallback_(panelName);
        }
    }
}

void MockPanelService::updatePanel()
{
    updatePanelCalled_ = true;
    updatePanelCallCount_++;
    
    if (updateCallback_) {
        updateCallback_();
    }
}

void MockPanelService::setUiState(UIState state)
{
    currentUiState_ = state;
}

const char* MockPanelService::getCurrentPanel() const
{
    return currentPanel_.c_str();
}

const char* MockPanelService::getRestorationPanel() const
{
    return restorationPanel_.c_str();
}

void MockPanelService::triggerPanelSwitchCallback(const char* triggerId)
{
    triggerPanelSwitchCallbackCalled_ = true;
    
    if (triggerId) {
        triggerCallbackHistory_.emplace_back(triggerId);
    }
    
    if (triggerCallback_) {
        triggerCallback_(triggerId);
    }
}

void MockPanelService::reset()
{
    currentPanel_ = PanelNames::OIL;
    restorationPanel_ = PanelNames::OIL;
    currentUiState_ = UIState::IDLE;
    mockGpioProvider_ = nullptr;
    mockDisplayProvider_ = nullptr;
    
    initCalled_ = false;
    initWithProvidersCalled_ = false;
    createAndLoadPanelCalled_ = false;
    createAndLoadPanelWithSplashCalled_ = false;
    updatePanelCalled_ = false;
    triggerPanelSwitchCallbackCalled_ = false;
    
    updatePanelCallCount_ = 0;
    panelLoadCount_ = 0;
    simulateLoadFailure_ = false;
    loadDelay_ = 0;
    
    panelLoadHistory_.clear();
    triggerCallbackHistory_.clear();
    lastCompletionCallback_ = nullptr;
    
    initCallback_ = nullptr;
    loadCallback_ = nullptr;
    updateCallback_ = nullptr;
    triggerCallback_ = nullptr;
}

void MockPanelService::setCurrentPanel(const char* panelName)
{
    currentPanel_ = panelName ? panelName : "";
}

void MockPanelService::setRestorationPanel(const char* panelName)
{
    restorationPanel_ = panelName ? panelName : "";
}

void MockPanelService::recordPanelLoad(const char* panelName, bool isTriggerDriven, bool withSplash)
{
    PanelLoadEvent event;
    event.panelName = panelName ? panelName : "";
    event.isTriggerDriven = isTriggerDriven;
    event.withSplash = withSplash;
    event.timestamp = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    
    panelLoadHistory_.push_back(event);
}

void MockPanelService::executeCompletionCallback()
{
    if (lastCompletionCallback_) {
        auto callback = lastCompletionCallback_;
        lastCompletionCallback_ = nullptr; // Clear before calling to avoid re-entry
        callback();
    }
}