#pragma once

#ifdef UNIT_TESTING

// Forward declarations
class MockDisplayProvider;
class MockGpioProvider;
class MockStyleService;

// Centralized mock service declarations to prevent redefinition conflicts
// These will be defined once in mock_implementations.cpp
extern MockDisplayProvider* g_mockDisplay;
extern MockGpioProvider* g_mockGpio;
extern MockStyleService* g_mockStyle;

// Utility functions for mock management
void initGlobalMocks();
void cleanupGlobalMocks();

#endif // UNIT_TESTING