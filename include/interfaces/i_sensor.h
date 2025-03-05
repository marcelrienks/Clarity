#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <utilities/common_types.h>

class ISensor
{
public:    
    virtual void init() = 0;
    virtual Reading get_reading() = 0;
};