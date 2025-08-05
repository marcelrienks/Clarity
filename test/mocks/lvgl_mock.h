#pragma once

#include <cstdint>

/**
 * @file lvgl_mock.h
 * @brief Mock LVGL types and constants for native testing
 * 
 * @details This file provides mock implementations of LVGL types and constants
 * that are needed by the sensor code but not available in native testing environment.
 * Only the minimal subset needed for sensor testing is provided.
 */

// Mock LVGL coordinate type
typedef int16_t lv_coord_t;

// Mock LVGL color type
typedef struct {
    uint16_t full;
} lv_color_t;

// Mock LVGL alignment constants
typedef enum {
    LV_ALIGN_CENTER = 0,
    LV_ALIGN_LEFT_MID = 1,
    LV_ALIGN_RIGHT_MID = 2,
    LV_ALIGN_TOP_MID = 3,
    LV_ALIGN_BOTTOM_MID = 4
} lv_align_t;

// Mock size constants
#define LV_SIZE_CONTENT -1

// Mock color creation function
#define lv_color_hex(c) {c}
#define lv_color_white() {0xFFFF}
#define lv_color_black() {0x0000}
#define lv_color_red() {0xF800}