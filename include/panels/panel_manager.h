#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "interfaces/i_device.h"
#include "utilities/serial_logger.h"

#include <vector>
#include <map>
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

    void register_panel(std::shared_ptr<IPanel> panel);
    void show_panel(std::shared_ptr<IPanel> panel, std::function<void()> completion_callback = nullptr);
    void show_all_panels();
    void update_current_panel();

private:
    IDevice *_device; // TODO: is this required, is it being used?
    std::list<std::shared_ptr<IPanel>> _panels;
    std::list<std::shared_ptr<IPanel>>::iterator _panel_iterator;
    IPanel *_current_panel;
    bool _is_recursion_locked = false; // this allows the panel manager to be locked during a cycle of recursion
    bool _is_panel_locked = false; // this allows the panel to be locked during loading (animation, etc.)

    void show_panel_from_iterator();

    static void show_panel_completion_callback();
    static void display_timer_callback(lv_timer_t *timer);
};

extern PanelManager *g_panel_manager_instance;
