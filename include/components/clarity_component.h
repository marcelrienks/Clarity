#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// Project Includes
#include "interfaces/i_component.h"
#include "interfaces/i_style_service.h"

/**
 * @class ClarityComponent
 * @brief Clarity branding and logo display component
 * 
 * @details This component renders the Clarity brand logo/text for use in
 * splash screens and branding displays. It provides a consistent visual
 * identity across the application.
 * 
 * @view_role Renders static branding content with positioning support
 * @ui_elements Clarity logo/text with theme-aware styling
 * @positioning Supports all ComponentLocation alignment options
 * 
 * @usage_context Primarily used in SplashPanel for startup branding
 * @render_strategy Static content with one-time load rendering
 * @memory_usage Minimal - static content only
 * 
 * @context This component displays the Clarity brand identity.
 * It's used in the splash screen and could be reused for other branding
 * purposes throughout the application.
 */
class ClarityComponent : public IComponent
{
public:
    // Constructors and Destructors
    explicit ClarityComponent(IStyleService *styleService);
    virtual ~ClarityComponent() = default;

    // Core Functionality Methods
    void render(lv_obj_t *screen, const ComponentLocation& location, IDisplayProvider *display) override;

private:
    IStyleService *styleService_;
};