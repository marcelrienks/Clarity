#ifndef I_SENSOR_H
#define I_SENSOR_H

#include <string>

class ISensor
{
public:    
    virtual void init();
    virtual std::string get_reading();
};

#endif // I_SENSOR_H