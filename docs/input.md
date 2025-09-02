# Input System Architecture

## Overview
The Clarity digital gauge system uses a single button input connected to GPIO 32 that provides different behaviors based on the current panel and press duration.

## Hardware Configuration

### Button Setup
* Push button wired to GPIO 32 (3.3V connection)
* Rising edge detection (trigger only when pin changes to HIGH)
* Hardware debouncing to prevent false triggers

## Input System Design

### Architecture
* Input system integrated with ActionHandler for centralized button handling
* Panels implement IActionService interface for universal button functions
* ActionHandler manages timing logic to distinguish short press (50ms-2000ms) vs long press (2000ms-5000ms)
* Button functions injected into Action interrupts for universal panel compatibility

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

## Config Panel Design

### Visual Design
* ConfigPanel with styling similar to error component but with grey colors
* Simple menu structure suitable for single button navigation

### Menu Structure
* Multi-level navigation with clear visual feedback
* Editing states for modifying option values
* Back navigation support

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
* Settings survive device reboots
* Settings validation on startup

## Technical Considerations

### Button Timing
* Short press: 50ms - 2000ms (with debouncing)
* Long press: 2000ms - 5000ms
* Maximum press time: 5000ms (auto-release)
* ActionHandler manages press duration detection and event queuing

### State Management
* Track current menu level and selected option
* Handle navigation state transitions
* Provide visual feedback for all interactions

### Error Handling
* Graceful fallback if invalid states are reached
* Timeout mechanisms for unresponsive interactions
* Reset to safe state on errors