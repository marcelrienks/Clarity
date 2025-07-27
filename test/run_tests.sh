#!/bin/bash

# Clarity ESP32 Test Runner Script
# Compiles and runs the comprehensive unit test suite

echo "==================================================="
echo "    CLARITY ESP32 UNIT TEST COMPILATION & RUN"
echo "==================================================="

# Set compiler and flags
CXX="g++"
CXXFLAGS="-std=c++17 -I. -I../include -DUNIT_TESTING -DUNITY_INCLUDE_DOUBLE"
UNITY_LIB="unity"

# Create build directory
mkdir -p ../build/test

echo "Compiling test files..."

# Compile Unity framework (if not system-installed)
if [ ! -f "/usr/lib/libunity.a" ] && [ ! -f "/usr/local/lib/libunity.a" ]; then
    echo "Unity not found in system. You may need to install it or compile manually."
    echo "On Ubuntu/Debian: sudo apt-get install libunity-dev"
    echo "Or download from: https://github.com/ThrowTheSwitch/Unity"
fi

# Compile test files
echo "Compiling test utilities..."
$CXX $CXXFLAGS -c test_utilities.cpp -o ../build/test/test_utilities.o

echo "Compiling trigger system tests..."
$CXX $CXXFLAGS -c test_trigger_system.cpp -o ../build/test/test_trigger_system.o

echo "Compiling panel manager tests..."
$CXX $CXXFLAGS -c test_panel_manager.cpp -o ../build/test/test_panel_manager.o

echo "Compiling sensor tests..."
$CXX $CXXFLAGS -c test_sensors.cpp -o ../build/test/test_sensors.o

echo "Compiling integration tests..."
$CXX $CXXFLAGS -c test_scenarios_integration.cpp -o ../build/test/test_scenarios_integration.o

echo "Compiling test main..."
$CXX $CXXFLAGS -c test_main.cpp -o ../build/test/test_main.o

# Link executable
echo "Linking test executable..."
$CXX $CXXFLAGS ../build/test/*.o -l$UNITY_LIB -o ../build/test/clarity_tests

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    echo ""
    echo "==================================================="
    echo "           RUNNING COMPREHENSIVE TEST SUITE"
    echo "==================================================="
    
    # Run the tests
    ../build/test/clarity_tests
    
    echo ""
    echo "==================================================="
    echo "                TESTS COMPLETED"
    echo "==================================================="
else
    echo "Compilation failed!"
    exit 1
fi