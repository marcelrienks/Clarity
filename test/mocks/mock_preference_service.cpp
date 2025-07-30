#include "mock_preference_service.h"
#include "mock_types.h"

MockPreferenceService::MockPreferenceService()
    : hasStoredConfig_(false)
    , initCalled_(false)
    , saveConfigCalled_(false)
    , loadConfigCalled_(false)
    , createDefaultConfigCalled_(false)
    , saveCount_(0)
    , loadCount_(0)
    , simulateLoadFailure_(false)
    , simulateSaveFailure_(false)
{
    createDefaults();
}

void MockPreferenceService::init()
{
    initCalled_ = true;
    
    if (initCallback_) {
        initCallback_();
    }
    
    // Simulate initialization behavior - load config if available
    if (hasStoredConfig_ && !simulateLoadFailure_) {
        loadConfig();
    } else {
        createDefaultConfig();
    }
}

void MockPreferenceService::saveConfig()
{
    saveConfigCalled_ = true;
    saveCount_++;
    
    if (simulateSaveFailure_) {
        // Simulate save failure - don't actually save
        return;
    }
    
    // Save current config
    lastSavedConfig_ = currentConfig_;
    hasStoredConfig_ = true;
    
    if (saveCallback_) {
        saveCallback_(currentConfig_);
    }
}

void MockPreferenceService::loadConfig()
{
    loadConfigCalled_ = true;
    loadCount_++;
    
    if (simulateLoadFailure_ || !hasStoredConfig_) {
        // Simulate load failure or no stored config - use defaults
        createDefaultConfig();
        return;
    }
    
    // Load the last saved config
    currentConfig_ = lastSavedConfig_;
    
    if (loadCallback_) {
        loadCallback_();
    }
}

void MockPreferenceService::createDefaultConfig()
{
    createDefaultConfigCalled_ = true;
    createDefaults();
}

Configs& MockPreferenceService::getConfig()
{
    return currentConfig_;
}

const Configs& MockPreferenceService::getConfig() const
{
    return currentConfig_;
}

void MockPreferenceService::setConfig(const Configs& config)
{
    currentConfig_ = config;
}

void MockPreferenceService::reset()
{
    initCalled_ = false;
    saveConfigCalled_ = false;
    loadConfigCalled_ = false;
    createDefaultConfigCalled_ = false;
    saveCount_ = 0;
    loadCount_ = 0;
    simulateLoadFailure_ = false;
    simulateSaveFailure_ = false;
    hasStoredConfig_ = false;
    initCallback_ = nullptr;
    saveCallback_ = nullptr;
    loadCallback_ = nullptr;
    createDefaults();
}

void MockPreferenceService::createDefaults()
{
    currentConfig_.panelName = PanelNames::OIL;
    lastSavedConfig_ = currentConfig_;
}