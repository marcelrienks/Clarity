#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <Preferences.h>
#include <Arduino.h>

class PreferenceManager
{
private:
    Preferences _preferences;
    const char *_namespace;
    bool _is_initialized;

public:
    PreferenceManager(const char *name = "preferences") : _namespace(name), _is_initialized(false) {}
    bool begin();
    void end();
};