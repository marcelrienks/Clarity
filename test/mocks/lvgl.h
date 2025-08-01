#pragma once

// Mock LVGL definitions for unit testing
#ifdef UNIT_TESTING

#include <cstdint>

// Basic LVGL types
typedef struct _lv_obj_t lv_obj_t;
typedef int16_t lv_coord_t;
typedef uint8_t lv_align_t;
typedef struct _lv_style_t { int dummy; } lv_style_t;

// Mock color type
typedef struct {
    uint16_t full;
} lv_color_t;

// LVGL constants
#define LV_ALIGN_CENTER 0

// Mock LVGL functions
static inline void lv_tick_inc(uint32_t tick_period) {}
static inline uint32_t lv_timer_handler() { return 1; }
static inline lv_color_t lv_color_hex(uint32_t c) { lv_color_t color = {0}; return color; }
static inline lv_color_t lv_color_white() { lv_color_t color = {0xFFFF}; return color; }
static inline lv_color_t lv_color_black() { lv_color_t color = {0}; return color; }

#endif // UNIT_TESTING