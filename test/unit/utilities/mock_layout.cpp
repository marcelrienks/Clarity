#include "test_utilities.h"

void lv_obj_set_align(mock_lv_obj_t* obj, int32_t align_type) {
    obj->align_type = align_type;
    obj->aligned = true;
}

void lv_obj_set_pos(mock_lv_obj_t* obj, int32_t x, int32_t y) {
    obj->x_offset = x;
    obj->y_offset = y;
}

void mock_lv_obj_align(mock_lv_obj_t* obj, int32_t align, int32_t x_offset, int32_t y_offset) {
    obj->align_type = align;
    obj->x_offset = x_offset;
    obj->y_offset = y_offset;
    obj->aligned = true;
}

mock_lv_obj_t* mock_lv_label_create(mock_lv_obj_t* screen) {
    static mock_lv_obj_t obj = create_mock_lv_obj();
    obj.created = true;
    obj.deleted = false;
    return &obj;
}

void mock_lv_label_set_text(mock_lv_obj_t* obj, const char* text) {
    obj->text_set = true;
    obj->text_content = text;
}

mock_lv_obj_t* mock_lv_image_create(mock_lv_obj_t* screen) {
    static mock_lv_obj_t obj = create_mock_lv_obj();
    obj.created = true;
    obj.deleted = false;
    return &obj;
}

void mock_lv_image_set_src(mock_lv_obj_t* obj, const void* src) {
    obj->image_set = true;
    obj->image_src = src;
}

void mock_lv_obj_set_style_text_font(mock_lv_obj_t* obj, const lv_font_t* font, uint32_t selector) {
    // No need to store the font since we're not testing font rendering
}

void mock_lv_obj_set_style_image_recolor(mock_lv_obj_t* obj, local_style_lv_color_t color, uint32_t selector) {
    obj->color_value = color.hex_value;
}

void mock_lv_obj_set_style_image_recolor_opa(mock_lv_obj_t* obj, uint8_t opa, uint32_t selector) {
    obj->recolor_opa = opa;
}

void mock_lv_obj_del(mock_lv_obj_t* obj) {
    if (obj) {
        obj->deleted = true;
    }
}

mock_lv_obj_t* mock_lv_arc_create(mock_lv_obj_t* screen) {
    static mock_lv_obj_t obj = create_mock_lv_obj();
    obj.created = true;
    obj.deleted = false;
    return &obj;
}

mock_lv_obj_t* mock_lv_line_create(mock_lv_obj_t* screen) {
    static mock_lv_obj_t obj = create_mock_lv_obj();
    obj.created = true;
    obj.deleted = false;
    return &obj;
}
