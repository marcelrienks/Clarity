#pragma once

/**
 * @interface IPanelSwitchService
 * @brief Interface for requesting panel switches
 * 
 * @details This interface allows panels to request switching to other panels
 * without having direct dependencies on PanelManager. It provides a clean
 * way to decouple panel switching logic from the panels themselves.
 */
class IPanelSwitchService
{
public:
    virtual ~IPanelSwitchService() = default;

    /**
     * @brief Request to switch to a different panel
     * @param targetPanel Name of the panel to switch to
     */
    virtual void RequestPanelSwitch(const char* targetPanel) = 0;
};