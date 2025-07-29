#include "mock_lvgl_components.h"

extern "C" {

// Mock constants
const uint32_t LV_PART_MAIN = 0x01;
const uint32_t LV_STATE_DEFAULT = 0x02;
const uint32_t MAIN_DEFAULT = LV_PART_MAIN | LV_STATE_DEFAULT;
const uint8_t LV_OPA_COVER = 255;
const int32_t LV_ALIGN_CENTER = 0;

// Mock font
struct {
    int size;
} lv_font_montserrat_20 = {20};

// Mock icon data
struct {
    const char* name;
} key_solid = {"key_solid_icon"};

struct {
    const char* name;
} lock_alt_solid = {"lock_alt_solid_icon"};

struct {
    const char* name;
} oil_can_regular = {"oil_can_regular_icon"};

mock_lv_obj_t* mock_lv_label_create(mock_lv_obj_t* parent) {
    static mock_lv_obj_t obj = create_mock_lv_obj();
    obj.created = true;
    obj.deleted = false;
    return &obj;
}

mock_lv_obj_t* mock_lv_image_create(mock_lv_obj_t* parent) {
    static mock_lv_obj_t obj = create_mock_lv_obj();
    obj.created = true;
    obj.deleted = false;
    return &obj;
}

mock_lv_obj_t* mock_lv_arc_create(mock_lv_obj_t* parent) {
    static mock_lv_obj_t obj = create_mock_lv_obj();
    obj.created = true;
    obj.deleted = false;
    return &obj;
}

mock_lv_obj_t* mock_lv_line_create(mock_lv_obj_t* parent) {
    static mock_lv_obj_t obj = create_mock_lv_obj();
    obj.created = true;
    obj.deleted = false;
    return &obj;
}

void mock_lv_label_set_text(mock_lv_obj_t* obj, const char* text) {
    obj->text_set = true;
    obj->text_content = text;
}

void mock_lv_image_set_src(mock_lv_obj_t* obj, const void* src) {
    obj->image_set = true;
    obj->image_src = src;
}

void mock_lv_obj_add_style(mock_lv_obj_t* obj, mock_lv_style_t* style, uint32_t selector) {
    obj->styles_applied = true;
    obj->style = style;
}

void mock_lv_obj_align(mock_lv_obj_t* obj, int32_t align, int32_t x_ofs, int32_t y_ofs) {
    obj->aligned = true;
    obj->align_type = align;
    obj->x_offset = x_ofs;
    obj->y_offset = y_ofs;
}

void mock_lv_obj_set_style_text_font(mock_lv_obj_t* obj, const void* font, uint32_t selector) {
    // Font setting - no need to store since we don't test font rendering
}

void mock_lv_obj_set_style_image_recolor(mock_lv_obj_t* obj, mock_lv_color_t color, uint32_t selector) {
    obj->color_value = color.hex_value;
}

void mock_lv_obj_set_style_image_recolor_opa(mock_lv_obj_t* obj, uint8_t opa, uint32_t selector) {
    obj->recolor_opa = opa;
}

void mock_lv_obj_del(mock_lv_obj_t* obj) {
    if (obj) {
        obj->deleted = true;
        obj->created = false;
    }
}

}
