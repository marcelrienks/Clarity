#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <esp32-hal-log.h>
#include <utilities/types.h>

class ISensor
{
public:    
    virtual void init() = 0;
    virtual Reading get_reading() = 0;
    
    // Delta-based update support
    virtual bool has_value_changed() { return true; } // Default: always update
};