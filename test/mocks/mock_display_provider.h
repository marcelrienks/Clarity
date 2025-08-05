#pragma once

#include "interfaces/i_display_provider.h"
#include <vector>
#include <map>

/**
 * @class MockDisplayProvider  
 * @brief Mock implementation of IDisplayProvider for unit testing
 * 
 * @details This mock tracks LVGL object creation and method calls
 * without actual rendering. Used primarily for component testing
 * in later phases.
 * 
 * @note Phase 1 focuses on sensors, so this mock is minimal
 */
class MockDisplayProvider : public IDisplayProvider
{
public:
    MockDisplayProvider() = default;
    virtual ~MockDisplayProvider() = default;

    // IDisplayProvider interface (minimal implementation for Phase 1)
    void init() override {}
    void update() override {}
    void* createObject(const char* type) override { 
        objectCreationCounts_[type]++;
        return nullptr; 
    }
    
    // Test verification methods
    int getObjectCreationCount(const char* type) const {
        auto it = objectCreationCounts_.find(type);
        return (it != objectCreationCounts_.end()) ? it->second : 0;
    }
    
    void reset() {
        objectCreationCounts_.clear();
    }

private:
    std::map<std::string, int> objectCreationCounts_;
};