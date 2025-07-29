#!/bin/bash

# Quick test runner for development
echo "Running quick trigger system tests..."

# Simple compilation for rapid feedback
mkdir -p ../build
g++ -std=c++17 -I. -I../include -DUNIT_TESTING \
    test_utilities.cpp \
    test_trigger_system.cpp \
    test_main.cpp \
    -DQUICK_TESTS_ONLY \
    -lunity -o ../build/quick_tests && \
    ../build/quick_tests