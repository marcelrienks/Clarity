#include "mock_style_service.h"
#include "mock_utilities.h"

MockStyleService::MockStyleService()
    : currentTheme_(Themes::DAY)
    , initCalled_(false)
    , applyThemeToScreenCalled_(false)
    , setThemeCalled_(false)
    , lastScreenApplied_(nullptr)
    , themeChangeCount_(0)
{
    initializeStyles();
}

void MockStyleService::init(const char* theme)
{
    initCalled_ = true;
    currentTheme_ = theme ? theme : Themes::DAY;
    updateStylesForTheme(currentTheme_.c_str());
}

void MockStyleService::applyThemeToScreen(lv_obj_t* screen)
{
    applyThemeToScreenCalled_ = true;
    lastScreenApplied_ = reinterpret_cast<const char*>(screen);
}

void MockStyleService::setTheme(const char* theme)
{
    setThemeCalled_ = true;
    
    if (theme && currentTheme_ != theme) {
        currentTheme_ = theme;
        themeChangeCount_++;
        updateStylesForTheme(theme);
        
        if (themeChangeCallback_) {
            themeChangeCallback_(theme);
        }
    }
}

lv_style_t& MockStyleService::getBackgroundStyle()
{
    return backgroundStyle_;
}

lv_style_t& MockStyleService::getTextStyle()
{
    return textStyle_;
}

lv_style_t& MockStyleService::getGaugeNormalStyle()
{
    return gaugeNormalStyle_;
}

lv_style_t& MockStyleService::getGaugeWarningStyle()
{
    return gaugeWarningStyle_;
}

lv_style_t& MockStyleService::getGaugeDangerStyle()
{
    return gaugeDangerStyle_;
}

lv_style_t& MockStyleService::getGaugeIndicatorStyle()
{
    return gaugeIndicatorStyle_;
}

lv_style_t& MockStyleService::getGaugeItemsStyle()
{
    return gaugeItemsStyle_;
}

lv_style_t& MockStyleService::getGaugeMainStyle()
{
    return gaugeMainStyle_;
}

lv_style_t& MockStyleService::getGaugeDangerSectionStyle()
{
    return gaugeDangerSectionStyle_;
}

const char* MockStyleService::getCurrentTheme() const
{
    return currentTheme_.c_str();
}

void MockStyleService::reset()
{
    initCalled_ = false;
    applyThemeToScreenCalled_ = false;
    setThemeCalled_ = false;
    lastScreenApplied_ = nullptr;
    themeChangeCount_ = 0;
    currentTheme_ = Themes::DAY;
    themeChangeCallback_ = nullptr;
    initializeStyles();
}

void MockStyleService::setThemeChangeCallback(std::function<void(const char*)> callback)
{
    themeChangeCallback_ = callback;
}

void MockStyleService::initializeStyles()
{
    // Initialize all mock styles
    mock_lv_style_init(&backgroundStyle_);
    mock_lv_style_init(&textStyle_);
    mock_lv_style_init(&gaugeNormalStyle_);
    mock_lv_style_init(&gaugeWarningStyle_);
    mock_lv_style_init(&gaugeDangerStyle_);
    mock_lv_style_init(&gaugeIndicatorStyle_);
    mock_lv_style_init(&gaugeItemsStyle_);
    mock_lv_style_init(&gaugeMainStyle_);
    mock_lv_style_init(&gaugeDangerSectionStyle_);
}

void MockStyleService::updateStylesForTheme(const char* theme)
{
    // Mock theme-specific style updates
    if (std::string(theme) == Themes::NIGHT) {
        // Night theme colors
        mock_lv_style_set_bg_color(&backgroundStyle_, mock_lv_color_hex(0x000000));
        mock_lv_style_set_text_color(&textStyle_, mock_lv_color_hex(0xFFFFFF));
        mock_lv_style_set_line_color(&gaugeNormalStyle_, mock_lv_color_hex(0x00FF00));
        mock_lv_style_set_line_color(&gaugeDangerStyle_, mock_lv_color_hex(0xFF0000));
    } else {
        // Day theme colors (default)
        mock_lv_style_set_bg_color(&backgroundStyle_, mock_lv_color_hex(0xFFFFFF));
        mock_lv_style_set_text_color(&textStyle_, mock_lv_color_hex(0x000000));
        mock_lv_style_set_line_color(&gaugeNormalStyle_, mock_lv_color_hex(0x0000FF));
        mock_lv_style_set_line_color(&gaugeDangerStyle_, mock_lv_color_hex(0xFF0000));
    }
}