#include "mock_style_service.h"
#include "mock_utilities.h"
#include "utilities/types.h"

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
    // Initialize all mock styles - using no-op for testing
    // In actual implementation, these would initialize LVGL styles
}

void MockStyleService::updateStylesForTheme(const char* theme)
{
    // Mock theme-specific style updates - no-op for testing
    // In actual implementation, these would set LVGL style properties
}