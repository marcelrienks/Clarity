# TODO:
## 1. Architecture Documentation

Create an architecture md document in the docs directory and link it in this README under the existing **Architecture** section.

### Include:
- **Patterns**
  - MVP Relationships
  - Layer Structure
- **Roles and Responsibilities**
- **Logic Flow**
- **Dynamic Panel Switching**
  - Based on sensor input
- **Trigger System Interrupts**
- **Panels**
- **Components**
- **Hardware Configuration**
  - GPIO Pins
  - Display/LCD Interface (SPI)
  - Sensor Inputs (Digital + Analog)

---

## 2. Scenario Documentation

Create a scenario md document in the docs directory and link it in this README under the existing **Scenario** section.

### Major Scenario (Full System Test)

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

### Individual Test Scenarios

- App starts → Splash animates with day theme (white text)
- App starts with pressure and temperature values set to halfway  
  → Splash animates with day theme (white text)  
  → Oil panel loads with day theme (white scale ticks and icon)  
  → Oil panel needles animate
- App starts  
  → Splash animates with day theme (white text)  
  → Oil panel loads with day theme  
  → Key present trigger high → Key panel loads (green icon)  
  → Key present trigger low → Oil panel loads
- App starts with key present trigger high  
  → Splash animates  
  → Oil panel does NOT load  
  → Key panel loads (green icon)  
  → Key present trigger low → Oil panel loads
- App starts  
  → Splash animates with day theme  
  → Oil panel loads  
  → Key not present trigger high → Key panel loads (red icon)  
  → Key not present trigger low → Oil panel loads
- App starts with key not present trigger high  
  → Splash animates  
  → Oil panel does NOT load  
  → Key panel loads (red icon)  
  → Key not present trigger low → Oil panel loads
- App starts  
  → Splash animates  
  → Oil panel loads  
  → Lock trigger high → Lock panel loads  
  → Lock trigger low → Oil panel loads
- App starts with lock trigger high  
  → Splash animates  
  → Oil panel does NOT load  
  → Lock panel loads  
  → Lock trigger low → Oil panel loads
- App starts  
  → Splash animates  
  → Oil panel loads  
  → Lights trigger high → Theme changes to night (no reload)  
  → Lights trigger low → Theme changes to day (no reload)
- App starts with lights trigger high  
  → Splash animates with night theme (red text)  
  → Oil panel loads with night theme  
  → Lights trigger low → Theme changes to day (no reload)

---

## 3. Unit Tests

Create unit tests for core logic and components.

---

## 4. Integration Tests

Create integration tests to validate full scenario flows.  
> Is it possible to do so to prove scenarios?

---

## 5. Wokwi Tests

Create Wokwi-based simulations for the documented scenarios.
