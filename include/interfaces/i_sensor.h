#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <utilities/types.h>

/// @brief Data sources
class ISensor
{
public:    
    virtual void init() = 0;
    virtual Reading get_reading() = 0;
};