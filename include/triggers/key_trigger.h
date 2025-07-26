#pragma once

#include "interfaces/i_trigger.h"
#include "utilities/types.h"

/**
 * @class KeyTrigger
 * @brief Trigger for key presence detection (pin HIGH = key present)
 * 
 * @details Shows key panel with "key present" styling when activated
 */
class KeyTrigger : public AlertTrigger
{
public:
    KeyTrigger();
    void init() override;
    
    TriggerActionRequest GetActionRequest() override;
    TriggerActionRequest GetRestoreRequest() override;
};

/**
 * @class KeyNotPresentTrigger  
 * @brief Trigger for key absence detection (pin HIGH = key not present)
 * 
 * @details Shows key panel with "key not present" styling when activated
 */
class KeyNotPresentTrigger : public AlertTrigger
{
public:
    KeyNotPresentTrigger();
    void init() override;
    
    TriggerActionRequest GetActionRequest() override;
    TriggerActionRequest GetRestoreRequest() override;
};