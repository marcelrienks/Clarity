#pragma once

#include "interfaces/i_input_service.h"
#include "interfaces/i_input_action.h"
#include "actions/input_actions.h"
#include <memory>

/**
 * @class LegacyInputAdapter
 * @brief Temporary adapter to bridge old panel interface with new action system
 * 
 * @details This adapter allows panels that still use the old IInputService interface
 * (with OnShortPress/OnLongPress methods) to work with the new action-based InputManager.
 * It wraps the old panel methods in action objects.
 * 
 * @temporary This adapter should be removed once all panels are updated to return
 * action objects directly from GetShortPressAction/GetLongPressAction.
 * 
 * @usage Create an adapter instance that wraps an old-style panel:
 * ```cpp
 * auto adapter = std::make_unique<LegacyInputAdapter>(oldPanel);
 * inputManager->SetInputService(adapter.get(), panelName);
 * ```
 */
class LegacyInputAdapter : public IInputService
{
public:
    // Forward declarations for legacy interface
    class LegacyInputService {
    public:
        virtual ~LegacyInputService() = default;
        virtual void OnShortPress() = 0;
        virtual void OnLongPress() = 0;
        virtual bool CanProcessInput() const = 0;
    };

    // Constructor
    LegacyInputAdapter(LegacyInputService* legacyService);
    ~LegacyInputAdapter() = default;

    // IInputService Interface Implementation
    std::unique_ptr<IInputAction> GetShortPressAction() override;
    std::unique_ptr<IInputAction> GetLongPressAction() override;
    bool CanProcessInput() const override;

private:
    LegacyInputService* legacyService_;

    // Internal action classes that wrap legacy method calls
    class LegacyShortPressAction : public IInputAction {
    public:
        LegacyShortPressAction(LegacyInputService* service) : service_(service) {}
        void Execute() override { if (service_) service_->OnShortPress(); }
        const char* GetDescription() const override { return "Legacy short press action"; }
        bool CanExecute() const override { return service_ != nullptr; }
    private:
        LegacyInputService* service_;
    };

    class LegacyLongPressAction : public IInputAction {
    public:
        LegacyLongPressAction(LegacyInputService* service) : service_(service) {}
        void Execute() override { if (service_) service_->OnLongPress(); }
        const char* GetDescription() const override { return "Legacy long press action"; }
        bool CanExecute() const override { return service_ != nullptr; }
    private:
        LegacyInputService* service_;
    };
};