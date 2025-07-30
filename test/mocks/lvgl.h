#pragma once

// Mock LVGL header for unit testing
// This provides minimal LVGL types and functions needed for compilation

#include <stdint.h>
#include <stdbool.h>

// Mock LVGL types
typedef struct _lv_obj_t lv_obj_t;
typedef struct _lv_font_t lv_font_t;
typedef struct _lv_image_dsc_t lv_image_dsc_t;
typedef struct _lv_style_t lv_style_t;
typedef struct _lv_scale_section_t lv_scale_section_t;

typedef int16_t lv_coord_t;
typedef uint8_t lv_align_t;
typedef uint32_t lv_part_t;
typedef uint32_t lv_state_t;
typedef uint32_t lv_scale_mode_t;

// Mock LVGL constants
#define LV_ALIGN_CENTER 9
#define LV_ALIGN_LEFT_MID 5
#define LV_PART_MAIN 0x00000000
#define LV_STATE_DEFAULT 0x00000000

// Mock color type
typedef struct {
    uint32_t full;
} lv_color_t;

// Mock LVGL functions (no-ops for unit testing)
static inline lv_obj_t* lv_label_create(lv_obj_t* parent) { return nullptr; }
static inline lv_obj_t* lv_image_create(lv_obj_t* parent) { return nullptr; }
static inline lv_obj_t* lv_arc_create(lv_obj_t* parent) { return nullptr; }
static inline lv_obj_t* lv_line_create(lv_obj_t* parent) { return nullptr; }
static inline void lv_label_set_text(lv_obj_t* label, const char* text) {}
static inline void lv_image_set_src(lv_obj_t* img, const void* src) {}
static inline void lv_obj_align(lv_obj_t* obj, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs) {}
static inline void lv_obj_add_style(lv_obj_t* obj, lv_style_t* style, lv_part_t part) {}
static inline void lv_obj_set_style_text_font(lv_obj_t* obj, const lv_font_t* font, lv_part_t part) {}

// Mock font
extern const lv_font_t lv_font_montserrat_20;