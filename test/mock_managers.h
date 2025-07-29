#pragma once
#include "mock_types.h"

class TriggerManager {
public:
    static TriggerManager& GetInstance();
    void init();
    void ProcessTriggerEvents();
private:
    TriggerManager() = default;
};

class PanelManager {
public:
    static PanelManager& GetInstance();
    void init();
    void loadPanel(const char* panelName);
    const char* getCurrentPanelName() const;
private:
    PanelManager() = default;
};

class StyleManager {
public:
    static StyleManager& GetInstance();
    void init();
    void setTheme(const char* theme);
private:
    StyleManager() = default;
};
