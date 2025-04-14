// include/managers/panel_manager.h
#pragma once

#include "interfaces/i_panel.h"
#include "interfaces/i_device.h"
#include "handlers/preference_manager.h"
#include "handlers/panel_factory.h"
#include "panels/splash_panel.h"
#include "panels/demo_panel.h"
#include "panels/oil_panel.h"

#include <string>
#include <functional>
#include <memory>
#include <list>
#include <vector>

#define PANEL_DISPLAY_TIME 3000
#define SPLASH_PANEL "SplashPanel"
#define DEMO_PANEL "DemoPanel"
#define OIL_PANEL "OilPanel"


class PanelManager {
public:
    PanelManager();
    ~PanelManager();

    void init(IDevice *device);
    void show_all_panels();
    void show_panel(IPanel *panel, std::function<void()> completion_callback = nullptr);
    void update_current_panel();

protected:
    IDevice *_device;

private:
    std::list<std::shared_ptr<IPanel>> _panels_ptr;
    std::list<std::shared_ptr<IPanel>>::iterator _panels_ptr_it;
    IPanel *_current_panel;
    bool _is_show_all_locked = false;   // this allows the panel manager to be locked during a cycle of recursion from show_all_panels()
    bool _is_panel_locked = false; // this allows the panel to be locked during loading from show_panel() or change from update_current_panel()

    static void display_timer_callback(lv_timer_t *timer);

    void read_panels_from_preferences();
    void register_panel_types();
    void register_panel(std::shared_ptr<IPanel> panel_ptr);
    void show_panel_completion_callback();
    void update_current_panel_completion_callback();
};