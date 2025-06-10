#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <esp32-hal-log.h>
#include <utilities/types.h>

template<typename T>
class ISensor
{
public:    
    virtual void init() = 0;
    virtual Reading get_reading(std::optional<T> type = std::nullopt) = 0;
};