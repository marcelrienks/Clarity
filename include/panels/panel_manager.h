#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "interfaces/i_device.h"
#include "utilities/serial_logger.h"
#include "splash_panel.h"
#include "demo_panel.h"

#include <string>
#include <functional>
#include <memory>
#include <list>

#define PANEL_DISPLAY_TIME 3000

//TODO: Implement preferences that will store a list of configured panel names
//TODO: Implement a default function in PanelManager that creates a list of all possible panels
//      If reading the config settings fails, use this default to save a new config
//TODO: Set up mechanism to disable splash after first showing

class PanelManager
{
public:
    PanelManager(IDevice *device);
    ~PanelManager();

    void init();
    void show_panel(IPanel *panel, std::function<void()> completion_callback = nullptr);
    void show_all_panels();
    void update_current_panel();

protected:
    IDevice *_device;

private:
    std::list<std::shared_ptr<IPanel>> _panel_ptrs;
    std::list<std::shared_ptr<IPanel>>::iterator _panels_iterator;
    IPanel *_current_panel;
    bool _is_show_all_locked = false; // this allows the panel manager to be locked during a cycle of recursion from show_all_panels()
    bool _is_panel_locked = false;     // this allows the panel to be locked during loading from show_panel()

    static void display_timer_callback(lv_timer_t *timer);

    void init_default_panels();
    void register_panel(std::shared_ptr<IPanel> panel_ptr);
    void show_next_panel();
    void show_panel_completion_callback();
};