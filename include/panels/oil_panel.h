#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "components/demo_component.h"
#include "sensors/demo_sensor.h"

class OilPanel : public IPanel
{
public:
    OilPanel(IDevice *device, PanelIteration panel_iteration);
    ~OilPanel();
};