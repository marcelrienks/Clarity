#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "interfaces/i_device.h"
#include "utilities/serial_logger.h"
#include "splash_panel.h"

#include <string>
#include <functional>
#include <memory>
#include <list>

#define PANEL_DISPLAY_TIME 3000

class PanelManager
{
public:
    PanelManager(IDevice *device);
    ~PanelManager();

    void register_panel(IPanel *panel);
    void show_panel(IPanel *panel, std::function<void()> completion_callback = nullptr);
    void show_all_panels_recursively();
    void update_current_panel();

protected:
    IDevice *_device;

private:
    std::list<IPanel*> _panels;
    std::list<IPanel*>::iterator _panel_iterator;
    IPanel *_current_panel;
    bool _is_recursion_locked = false; // this allows the panel manager to be locked during a cycle of recursion from show_all_panels()
    bool _is_panel_locked = false;     // this allows the panel to be locked during loading from show_panel()
    bool is_splash_locked = false;     // this allows the splash panel to be locked after having been shown, or if disabled in config

    void show_panel_from_iterator();

    void show_panel_completion_callback();
    static void display_timer_callback(lv_timer_t *timer);
};