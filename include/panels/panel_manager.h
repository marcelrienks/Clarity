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
private:
    IDevice *_device; // TODO: is this required, is it being used?
    std::list<std::shared_ptr<IPanel>> _panels;
    std::list<std::shared_ptr<IPanel>>::iterator _panel_iterator;
    IPanel *_current_panel;
    int32_t _recursion_depth = 0;
    bool _recursion_locked = false;
    bool _panel_locked = false;

    static void show_panel_timer_completion_callback(lv_timer_t *timer);

public:
    PanelManager(IDevice *device);
    ~PanelManager();

    void register_panel(std::shared_ptr<IPanel> panel);
    void show_panel(std::shared_ptr<IPanel> panel, std::function<void()> completion_callback = nullptr);
    void show_panels_recursively();
    void update_current_panel();
};
