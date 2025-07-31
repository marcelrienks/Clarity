#pragma once

// System/Library Includes
#include <string>
#include <map>
#include <functional>

// Project Includes  
#include "interfaces/i_style_service.h"
#include "mock_types.h"
#include "mock_colors.h"

/**
 * @class MockStyleService
 * @brief Mock implementation of IStyleService for testing
 * 
 * @details Provides a testable implementation of the style service interface
 * with controllable behavior for unit tests. All LVGL style objects are
 * mocked using mock_lv_style_t to avoid LVGL dependencies in tests.
 * 
 * @testability Allows verification of theme switching and style application
 * @dependency_free No actual LVGL dependency - uses mock types
 */
class MockStyleService : public IStyleService
{
public:
    MockStyleService();
    virtual ~MockStyleService() = default;

    // Core Functionality Methods
    void init(const char* theme) override;
    void applyThemeToScreen(lv_obj_t* screen) override;
    void setTheme(const char* theme) override;

    // Style Accessor Methods
    lv_style_t& getBackgroundStyle() override;
    lv_style_t& getTextStyle() override;
    lv_style_t& getGaugeNormalStyle() override;
    lv_style_t& getGaugeWarningStyle() override;
    lv_style_t& getGaugeDangerStyle() override;
    lv_style_t& getGaugeIndicatorStyle() override;
    lv_style_t& getGaugeItemsStyle() override;
    lv_style_t& getGaugeMainStyle() override;
    lv_style_t& getGaugeDangerSectionStyle() override;

    // Theme Information Methods
    const char* getCurrentTheme() const override;
    const ThemeColors& getThemeColors() const override;

    // Test Helper Methods
    bool wasInitCalled() const { return initCalled_; }
    bool wasApplyThemeToScreenCalled() const { return applyThemeToScreenCalled_; }
    bool wasSetThemeCalled() const { return setThemeCalled_; }
    const char* getLastAppliedScreen() const { return lastScreenApplied_; }
    int getThemeChangeCount() const { return themeChangeCount_; }

    // Test Control Methods
    void reset();
    void setThemeChangeCallback(std::function<void(const char*)> callback);

private:
    // Mock Styles using LVGL mock types for compatibility
    lv_style_t backgroundStyle_;
    lv_style_t textStyle_;
    lv_style_t gaugeNormalStyle_;
    lv_style_t gaugeWarningStyle_;
    lv_style_t gaugeDangerStyle_;
    lv_style_t gaugeIndicatorStyle_;
    lv_style_t gaugeItemsStyle_;
    lv_style_t gaugeMainStyle_;
    lv_style_t gaugeDangerSectionStyle_;

    // State Tracking
    std::string currentTheme_;
    bool initCalled_;
    bool applyThemeToScreenCalled_;
    bool setThemeCalled_;
    const char* lastScreenApplied_;
    int themeChangeCount_;
    ThemeColors themeColors_;

    // Test Callbacks
    std::function<void(const char*)> themeChangeCallback_;

    void initializeStyles();
    void updateStylesForTheme(const char* theme);
};