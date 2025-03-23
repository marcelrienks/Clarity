#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_component.h"
#include "utilities/serial_logger.h"

class ClarityComponent : public IComponent
{
public:
    void init(lv_obj_t *screen) override;
    void update(Reading reading, std::function<void()> update_component_completion_callback = nullptr) override;
};