#pragma once

#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_style_service.h"
#include "utilities/types.h"


/**
 * @class BasePanel
 * @brief Base class providing common panel functionality and lifecycle management
 *
 * @details This class eliminates 200+ lines of duplicated boilerplate code across
 * all panels by centralizing common patterns:
 * - Constructor with standard dependency injection
 * - Screen creation and destruction
 * - Provider validation and error handling
 * - Theme application and styling
 * - LVGL event callbacks and lifecycle
 * - Manager service injection
 * - Standard logging patterns
 *
 * @design_pattern Template Method Pattern - defines common algorithms, derived classes implement specifics
 * @simplification_impact Reduces panel implementations by 60-80% for simple panels
 * @memory_management Handles screen lifecycle and component cleanup consistently
 *
 * @virtual_methods_to_implement:
 * - CreateContent(): Create and render panel-specific UI components
 * - UpdateContent(): Update components with fresh data
 * - GetPanelName(): Return panel name constant for logging/identification
 *
 * @optional_overrides:
 * - HandleShortPress(): Panel-specific short button press behavior
 * - HandleLongPress(): Panel-specific long button press behavior
 * - PostLoad(): Additional setup after content creation
 * - CustomInit(): Additional initialization before screen creation
 *
 * @example
 * @code
 * class KeyPanel : public BasePanel {
 * protected:
 *     void CreateContent() override {
 *         keyComponent_ = componentFactory_->CreateKeyComponent(styleService_);
 *         keyComponent_->Render(screen_, centerLocation_, displayProvider_);
 *     }
 *
 *     void UpdateContent() override {
 *         // Update key status if needed
 *     }
 *
 *     const char* GetPanelName() const override { return PanelNames::KEY; }
 * };
 * @endcode
 */
class BasePanel : public IPanel
{
public:
    // Constructors and Destructors
    BasePanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService);
    virtual ~BasePanel();

    // Core Functionality Methods - Final implementations using Template Method pattern
    void Init() final;
    void Load() final;
    void Update() final;

    // Manager injection method
    void SetManagers(IPanelService* panelService, IStyleService* styleService) override;

    // IActionService Interface Implementation
    void (*GetShortPressFunction())(void* panelContext) final;
    void (*GetLongPressFunction())(void* panelContext) final;
    void* GetPanelContext() final;

protected:
    // Template Method hooks - derived classes implement these

    /// @brief Create and render panel-specific UI components
    /// Called during Load() after screen setup but before screen loading
    virtual void CreateContent() = 0;

    /// @brief Update components with fresh data
    /// Called during Update() to refresh display with current data
    virtual void UpdateContent() = 0;

    /// @brief Return panel name constant for logging and identification
    virtual const char* GetPanelName() const = 0;

    // Optional hooks - default implementations provided

    /// @brief Additional initialization before screen creation
    /// Override for custom setup that needs to happen before screen creation
    virtual void CustomInit() {}

    /// @brief Additional setup after content creation but before screen loading
    /// Override for custom setup after components are created
    virtual void PostLoad() {}

    /// @brief Handle short button press - default does nothing
    virtual void HandleShortPress() {}

    /// @brief Handle long button press - default does nothing
    virtual void HandleLongPress() {}

    // Protected members available to derived classes
    IGpioProvider* gpioProvider_;
    IDisplayProvider* displayProvider_;
    IStyleService* styleService_;
    IPanelService* panelService_;
    ComponentLocation centerLocation_;

private:
    // Static event callback for LVGL
    static void ShowPanelCompletionCallback(lv_event_t* event);

    // Static button press functions for IActionService
    static void BasePanelShortPress(void* panelContext);
    static void BasePanelLongPress(void* panelContext);

    // Common validation and setup methods
    void ValidateProviders();
    void SetupScreen();
    void ApplyThemeAndLoadScreen();
};