# Scenario Documentation

This document outlines the major test scenarios for the Clarity digital gauge system, covering the complete system behavior under various trigger conditions and state transitions.

## Major Scenario (Full System Test)

This comprehensive scenario tests all major system components and their interactions:

> App starts with pressure and temperature values set to halfway  
> → Splash animates with day theme (white text)  
> → Oil panel loads with day theme (white scale ticks and icon)  
> → Oil panel needles animate  
> → Lights trigger high  
> → Oil panel does NOT reload, theme changes to night (red scale ticks and icon)  
> → Lock trigger high  
> → Lock panel loads  
> → Key not present trigger high  
> → Key panel loads (present = false → red icon)  
> → Key present trigger high  
> → Lock panel loads (both key present and key not present true = invalid state)  
> → Key not present trigger low  
> → Key panel loads (present = true → green icon, key present trigger still high)  
> → Key present trigger low  
> → Lock panel loads (lock trigger still high)  
> → Lock trigger low  
> → Oil panel loads with night theme (red scale ticks and icon, lights trigger still active)  
> → Oil panel needles animate  
> → Lights trigger low  
> → Oil panel does NOT reload, theme changes to day (white scale ticks and icon)

## Individual Test Scenarios

### Basic Startup Scenarios

**Default Startup**
- App starts → Splash animates with day theme (white text)

**Startup with Oil Data**
- App starts with pressure and temperature values set to halfway  
  → Splash animates with day theme (white text)  
  → Oil panel loads with day theme (white scale ticks and icon)  
  → Oil panel needles animate

### Key Present Scenarios

**Key Present During Runtime**
- App starts  
  → Splash animates with day theme (white text)  
  → Oil panel loads with day theme  
  → Key present trigger high → Key panel loads (green icon)  
  → Key present trigger low → Oil panel loads

**Key Present at Startup**
- App starts with key present trigger high  
  → Splash animates  
  → Oil panel does NOT load  
  → Key panel loads (green icon)  
  → Key present trigger low → Oil panel loads

### Key Not Present Scenarios

**Key Not Present During Runtime**
- App starts  
  → Splash animates with day theme  
  → Oil panel loads  
  → Key not present trigger high → Key panel loads (red icon)  
  → Key not present trigger low → Oil panel loads

**Key Not Present at Startup**
- App starts with key not present trigger high  
  → Splash animates  
  → Oil panel does NOT load  
  → Key panel loads (red icon)  
  → Key not present trigger low → Oil panel loads

### Lock Scenarios

**Lock During Runtime**
- App starts  
  → Splash animates  
  → Oil panel loads  
  → Lock trigger high → Lock panel loads  
  → Lock trigger low → Oil panel loads

**Lock at Startup**
- App starts with lock trigger high  
  → Splash animates  
  → Oil panel does NOT load  
  → Lock panel loads  
  → Lock trigger low → Oil panel loads

### Light/Theme Scenarios

**Theme Change During Runtime**
- App starts  
  → Splash animates  
  → Oil panel loads  
  → Lights trigger high → Theme changes to night (no reload)  
  → Lights trigger low → Theme changes to day (no reload)

**Night Theme at Startup**
- App starts with lights trigger high  
  → Splash animates with night theme (red text)  
  → Oil panel loads with night theme  
  → Lights trigger low → Theme changes to day (no reload)

## Scenario Testing Notes

- **Theme Changes**: When lights trigger state changes, panels should update their theme without reloading
- **Panel Priority**: Key and lock triggers take priority over oil panel display
- **Invalid States**: System should handle conflicting trigger states gracefully
- **Animation**: Oil panel needles should animate when loading with sensor data
- **Persistence**: Theme state should persist across panel switches