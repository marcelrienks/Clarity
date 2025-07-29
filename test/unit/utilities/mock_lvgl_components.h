#pragma once

#include "test_utilities.h"

// Mock component types
enum class KeyState {
    Inactive = 0,
    Present = 1,
    NotPresent = 2
};

// Mock functions
extern "C" {
    // Mock object creation functions
    mock_lv_obj_t* mock_lv_label_create(mock_lv_obj_t* parent);
    mock_lv_obj_t* mock_lv_image_create(mock_lv_obj_t* parent);
    mock_lv_obj_t* mock_lv_arc_create(mock_lv_obj_t* parent);
    mock_lv_obj_t* mock_lv_line_create(mock_lv_obj_t* parent);
    
    // Mock object functions
    void mock_lv_label_set_text(mock_lv_obj_t* obj, const char* text);
    void mock_lv_image_set_src(mock_lv_obj_t* obj, const void* src);
    void mock_lv_obj_add_style(mock_lv_obj_t* obj, mock_lv_style_t* style, uint32_t selector);
    void mock_lv_obj_align(mock_lv_obj_t* obj, int32_t align, int32_t x_ofs, int32_t y_ofs);
    void mock_lv_obj_del(mock_lv_obj_t* obj);
    void mock_lv_obj_set_style_text_font(mock_lv_obj_t* obj, const void* font, uint32_t selector);
    void mock_lv_obj_set_style_image_recolor(mock_lv_obj_t* obj, mock_lv_color_t color, uint32_t selector);
    void mock_lv_obj_set_style_image_recolor_opa(mock_lv_obj_t* obj, uint8_t opa, uint32_t selector);

    // Mock icon structures
    struct {
        const char* name;
    } key_solid;
    
    struct {
        const char* name;
    } lock_alt_solid;
    
    struct {
        const char* name;
    } oil_can_regular;
}
