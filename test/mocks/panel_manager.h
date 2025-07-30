#pragma once

#include <cstddef>

// Forward declarations
class IPanel;

// Mock PanelManager for testing
class PanelManager {
public:
    PanelManager();
    ~PanelManager();
    
    bool initialize();
    bool registerPanel(IPanel* panel);
    IPanel* createPanel(const char* panelName);
    IPanel* getCurrentPanel();
    size_t getRegisteredPanelCount();
    
private:
    IPanel* currentPanel_;
    size_t registeredPanelCount_;
    bool initialized_;
};

// Mock panels
class IPanel {
public:
    virtual ~IPanel() = default;
    virtual void init() = 0;
    virtual void load() = 0;
    virtual void update() = 0;
};

class KeyPanel : public IPanel {
public:
    void init() override {}
    void load() override {}
    void update() override {}
};

class LockPanel : public IPanel {
public:
    void init() override {}
    void load() override {}
    void update() override {}
};

class OemOilPanel : public IPanel {
public:
    void init() override {}
    void load() override {}
    void update() override {}
};

class SplashPanel : public IPanel {
public:
    void init() override {}
    void load() override {}
    void update() override {}
};