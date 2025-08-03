#pragma once

// Unity Configuration for Clarity Tests

// Note: UNITY_INCLUDE_DOUBLE is defined in platformio.ini build_flags

// Enable memory comparison tests  
#define UNITY_SUPPORT_TEST_CASES

// Buffer sizes
#define UNITY_INT_WIDTH 32
#define UNITY_FLOAT_TYPE double
#define UNITY_FLOAT_PRECISION 0.00001f