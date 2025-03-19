#include "managers/preference_manager.h"

bool PreferenceManager::begin() {
    _is_initialized = _preferences.begin(_namespace, false); // false = read/write mode
    return _is_initialized;
}

void PreferenceManager::end() {
    if (_is_initialized) {
        _preferences.end();
        _is_initialized = false;
    }
}