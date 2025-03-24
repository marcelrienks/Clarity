#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_component.h"
#include "utilities/serial_logger.h"

class ClarityComponent : public IComponent
{
public:
    void init(lv_obj_t *screen) override;
    void render_reading(Reading reading, std::function<void()> render_completion_callback = nullptr) override;
};