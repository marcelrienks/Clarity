#pragma once
#include <cstdint>

// Type declarations for LVGL color mocking
typedef struct {
    uint32_t hex_value;
} mock_lv_color_t;

// LVGL style mocking structure
typedef struct {
    bool initialized;
    mock_lv_color_t bg_color;
    mock_lv_color_t text_color;
    mock_lv_color_t line_color;
    uint8_t bg_opa;
    uint8_t text_opa;
    uint16_t length;
    uint16_t line_width;
    uint16_t arc_width;
    bool reset_called;
} mock_lv_style_t;

// LVGL object mocking structure
typedef struct {
    bool styles_applied;
    bool invalidated;
} mock_lv_obj_t;

// Theme colors structure for different display themes
struct MockThemeColors {
    mock_lv_color_t background;
    mock_lv_color_t text;
    mock_lv_color_t primary;
    mock_lv_color_t gaugeNormal;
    mock_lv_color_t gaugeWarning;
    mock_lv_color_t gaugeDanger;
    mock_lv_color_t gaugeTicks;
    mock_lv_color_t needleNormal;
    mock_lv_color_t needleDanger;
    mock_lv_color_t keyPresent;
    mock_lv_color_t keyNotPresent;

    MockThemeColors() = default;
    MockThemeColors(
        uint32_t bg, uint32_t txt, uint32_t pri,
        uint32_t gn, uint32_t gw, uint32_t gd,
        uint32_t gt, uint32_t nn, uint32_t nd,
        uint32_t kp, uint32_t knp)
        : background{mock_lv_color_hex(bg)}
        , text{mock_lv_color_hex(txt)}
        , primary{mock_lv_color_hex(pri)}
        , gaugeNormal{mock_lv_color_hex(gn)}
        , gaugeWarning{mock_lv_color_hex(gw)}
        , gaugeDanger{mock_lv_color_hex(gd)}
        , gaugeTicks{mock_lv_color_hex(gt)}
        , needleNormal{mock_lv_color_hex(nn)}
        , needleDanger{mock_lv_color_hex(nd)}
        , keyPresent{mock_lv_color_hex(kp)}
        , keyNotPresent{mock_lv_color_hex(knp)}
    {}
};

// LVGL style mock functions
inline void mock_lv_style_init(mock_lv_style_t* style) {
    style->initialized = true;
    style->reset_called = false;
}

inline void mock_lv_style_reset(mock_lv_style_t* style) {
    style->reset_called = true;
    style->initialized = false;
}

inline void mock_lv_style_set_bg_color(mock_lv_style_t* style, mock_lv_color_t color) {
    style->bg_color = color;
}

inline void mock_lv_style_set_bg_opa(mock_lv_style_t* style, uint8_t opa) {
    style->bg_opa = opa;
}

inline void mock_lv_style_set_text_color(mock_lv_style_t* style, mock_lv_color_t color) {
    style->text_color = color;
}

inline void mock_lv_style_set_text_opa(mock_lv_style_t* style, uint8_t opa) {
    style->text_opa = opa;
}

inline void mock_lv_style_set_line_color(mock_lv_style_t* style, mock_lv_color_t color) {
    style->line_color = color;
}

inline void mock_lv_style_set_length(mock_lv_style_t* style, uint16_t length) {
    style->length = length;
}

inline void mock_lv_style_set_line_width(mock_lv_style_t* style, uint16_t width) {
    style->line_width = width;
}

inline void mock_lv_style_set_arc_width(mock_lv_style_t* style, uint16_t width) {
    style->arc_width = width;
}

// LVGL object mock functions
inline void mock_lv_obj_add_style(mock_lv_obj_t* obj, mock_lv_style_t* style, uint32_t selector) {
    obj->styles_applied = true;
}

inline void mock_lv_obj_invalidate(mock_lv_obj_t* obj) {
    obj->invalidated = true;
}

// Mock color conversion
inline mock_lv_color_t mock_lv_color_hex(uint32_t hex) {
    return mock_lv_color_t{hex};
}

// Constants
constexpr uint8_t LV_OPA_COVER = 255;
constexpr uint32_t MAIN_DEFAULT = 0x01;
