# Input System Implementation Plan

## Overview
This document outlines the implementation plan for adding single button input functionality to the Clarity digital gauge system. The button will be connected to GPIO 32 and provide different behaviors based on the current panel and press duration.

## Phase 1: Hardware & Core Input System

### Hardware Setup
* Add push button to wokwi diagram wired to GPIO 32 (3.3V connection)
* Configure rising edge detection (trigger only when pin changes to HIGH)
* Implement proper debouncing to prevent false triggers

### Input System Architecture
* Create separate input system (isolated from existing trigger functionality)
* Design InputManager class for centralized button handling
* Implement IInputHandler interface for panels to implement
* Add timing logic to distinguish short press (<500ms) vs long press (>500ms)

## Phase 2: Panel Input Integration

### Per-Panel Input Behaviors
* **Oil Panel**: 
  - Short press: No action
  - Long press: Load config panel
* **Splash Panel**: 
  - Short press: Stop animation and load default panel immediately
  - Long press: Load config panel
* **Error Panel**: 
  - Short press: Cycle through each error
  - Long press: Clear all errors
* **Config Panel**: 
  - Short press: Cycle through options
  - Long press: Set/edit highlighted option

## Phase 3: Basic Config Panel (Dummy Implementation)

### Visual Design
* Create ConfigPanel with styling similar to error component but with grey colors
* Display simple menu structure suitable for single button navigation

### Basic Navigation
* Hardcoded options: "Option 1", "Option 2", "Exit"
* Short press: Cycle through options with visual highlighting
* Long press: Placeholder functionality (no action initially)

## Phase 4: Full Config Panel Implementation

### Menu Structure
* Multi-level navigation with clear visual feedback
* Editing states for modifying option values
* Breadcrumb or back navigation support

### Configuration Options

#### Default Panel Selection
* Display list of all registered panels
* Short press: Cycle through available panels
* Long press: Set highlighted panel as default

#### Splash Screen Toggle
* Enable/disable splash screen on startup
* Short press: Cycle between "Enabled" and "Disabled"
* Long press: Apply selected setting

#### Exit Option
* Return to default panel
* Short press: Highlight exit option
* Long press: Execute exit and return to default panel

### Persistence Integration
* Integrate with PreferenceManager for persistent storage of all configuration settings
* Ensure settings survive device reboots
* Validate settings on startup

## Technical Considerations

### Button Timing
* Short press: 50ms - 500ms (with debouncing)
* Long press: >500ms
* Maximum press time: 3000ms (auto-release)

### State Management
* Track current menu level and selected option
* Handle navigation state transitions
* Provide visual feedback for all interactions

### Error Handling
* Graceful fallback if invalid states are reached
* Timeout mechanisms for unresponsive interactions
* Reset to safe state on errors