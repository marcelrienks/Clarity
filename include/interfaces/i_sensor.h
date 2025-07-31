#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// Project Includes
#include "utilities/types.h"

class ISensor
{
public:
    // Core Interface Methods
    virtual void init() = 0;
    virtual Reading getReading() = 0;
};