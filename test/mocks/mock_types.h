#pragma once
#include <cstdint>

// Mock types that don't conflict with actual types.h
// This file only contains mock-specific types for testing

#ifdef NATIVE_TESTING
#include "mock_colors.h"

// MOCK: LVGL Types and Constants for testing only
// Note: lv_obj_t is defined in lvgl.h mock
typedef mock_lv_style_t mock_lv_style_t_alias;
typedef mock_lv_color_t mock_lv_color_t_alias;

#endif // NATIVE_TESTING
