#include "test_utilities.h"

// Mock LVGL function implementations
void mock_lv_style_init(mock_lv_style_t* style) {
    style->initialized = true;
    style->reset_called = false;
}

void mock_lv_style_reset(mock_lv_style_t* style) {
    style->reset_called = true;
    style->initialized = false;
}

void mock_lv_style_set_bg_color(mock_lv_style_t* style, mock_lv_color_t color) {
    style->bg_color = color;
}

void mock_lv_style_set_text_color(mock_lv_style_t* style, mock_lv_color_t color) {
    style->text_color = color;
}

void mock_lv_style_set_line_color(mock_lv_style_t* style, mock_lv_color_t color) {
    style->line_color = color;
}

void mock_lv_style_set_bg_opa(mock_lv_style_t* style, uint8_t opa) {
    style->bg_opa = opa;
}

void mock_lv_style_set_text_opa(mock_lv_style_t* style, uint8_t opa) {
    style->text_opa = opa;
}

void mock_lv_style_set_length(mock_lv_style_t* style, uint16_t length) {
    style->length = length;
}

void mock_lv_style_set_line_width(mock_lv_style_t* style, uint16_t width) {
    style->line_width = width;
}

void mock_lv_style_set_arc_width(mock_lv_style_t* style, uint16_t width) {
    style->arc_width = width;
}

void mock_lv_obj_add_style(mock_lv_obj_t* obj, mock_lv_style_t* style, uint32_t selector) {
    obj->styles_applied = true;
}

void mock_lv_obj_invalidate(mock_lv_obj_t* obj) {
    obj->invalidated = true;
}

mock_lv_color_t mock_lv_color_hex(uint32_t hex) {
    return mock_lv_color_t{hex};
}
