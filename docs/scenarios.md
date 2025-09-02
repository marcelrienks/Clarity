# Test Scenarios

## Full Integration Test

Comprehensive system test covering all interrupts, effects, and panel transitions:

### 1. Startup and Initial Animation
- **Start** → Splash panel loads with day theme
- **Splash** → Oil panel loads automatically after animation
- **Oil Panel**: Animate pressure & animate temperature gauges to show sensor data

### 2. Theme Change (Non-Blocking)
- **Activate lights interrupt** → Theme changes to night (red theme)
- **Oil Panel**: Continues displaying with night theme
- **Lights interrupt**: SET_THEME effect, always maintains (doesn't block restoration)

### 3. Panel Loading Interrupts (Priority-Based)
- **Activate lock** → Lock panel loads (IMPORTANT priority)
- **Lights interrupt**: Still active but doesn't block
- **Lock Panel**: Displays with night theme

### 4. Higher Priority Key Interrupts
- **Activate key not present** → Key panel with red key loads (CRITICAL priority)
- **Key interrupt**: Higher priority than lock, overrides lock panel
- **Key Panel**: Red key icon with night theme

### 5. Same Priority Key State Change
- **Activate key present** → Key panel with green key (CRITICAL priority)
- **Key present**: Same priority as key not present, but newer so takes precedence
- **Key Panel**: Green key icon with night theme

### 6. Key State Transitions
- **Deactivate key present** → Key panel with red key
- **Key not present**: Still active, so key panel remains
- **Key Panel**: Back to red key icon (key not present state)

### 7. Priority-Based Restoration
- **Deactivate key not present** → Lock panel loads
- **Lock interrupt**: Next highest priority (IMPORTANT), loads lock panel
- **Lights interrupt**: Still active but maintains (doesn't participate in blocking)
- **Lock Panel**: Displays with night theme

### 8. User Panel Restoration
- **Deactivate lock** → Restore user-driven panel (Oil Panel)
- **No blocking interrupts**: All panel-loading interrupts inactive
- **Lights interrupt**: Still active but allows restoration
- **Oil Panel**: Restored with night theme, animate pressure & temperature

### 9. Theme Restoration
- **Deactivate lights** → Theme changes to day
- **Oil Panel**: Continues with day theme (white theme)
- **Oil Panel**: Animate pressure & temperature with day theme

### 10. Button Actions - Config Access
- **Oil panel long press** → Config panel loads
- **Long Press**: Executes Oil panel's long press function (LoadConfigPanel)
- **Config Panel**: Main menu with day theme

### 11. Config Navigation
- **Short press** → Navigate configs repeatedly until theme setting selected
- **Short Press**: Executes Config panel's short press function (CycleOptions)
- **Config Panel**: Highlight theme setting option

### 12. Theme Configuration
- **Long press** → Enter theme settings
- **Long Press**: Executes Config panel's long press function (SelectOption)
- **Config Panel**: Theme submenu

### 13. Theme Selection
- **Short press** → Navigate to night theme option
- **Short Press**: Cycle through theme options (Day → Night)
- **Config Panel**: Night theme option highlighted

### 14. Theme Application
- **Long press** → Set theme to night and exit to main menu
- **Long Press**: Apply night theme setting, return to main config menu
- **Config Panel**: Main menu with night theme applied

### 15. Config Exit
- **Short press** → Navigate configs repeatedly until exit setting selected
- **Short Press**: Cycle through main menu options to Exit
- **Config Panel**: Exit option highlighted

### 16. Return to Oil Panel
- **Long press** → Oil panel with night theme
- **Long Press**: Execute exit function, load Oil panel
- **Oil Panel**: Displays with night theme, animate pressure & temperature

### 17. Error System Integration
- **Activate debug error** → Error panel loads (CRITICAL priority)
- **Error interrupt**: Trigger interrupt with highest priority
- **Error Panel**: Displays error list with night theme

### 18. Error Navigation
- **Short press** → Cycle through errors repeatedly
- **Short Press**: Executes Error panel's short press function (CycleErrors)
- **Error Panel**: Navigate through all error messages

### 19. Error Resolution and Restoration
- **All error messages viewed** → Oil panel restored
- **Error acknowledgment**: Clears error queue, deactivates error interrupt
- **Oil Panel**: Restored with night theme, animate pressure & temperature

## Individual Scenarios

### Startup
- Default: Splash (day theme) → Oil panel
- With sensors: Needles animate to sensor values
- With interrupts: Skip oil panel, load interrupt-driven panel
- With interrupts: Set theme before loading any panels

### Key Interrupts
- **Present**: Green icon, CRITICAL priority over oil panel
- **Not Present**: Red icon, CRITICAL priority over oil panel

### Lock Interrupt
- Takes IMPORTANT priority over oil panel
- Lower priority than key interrupts

### Theme Switching
- Light sensor controls day/night theme
- No panel reload on theme change
- Persists across panel switches
- Always maintains (never blocks restoration)

## Testing Notes
- **Panel priorities**: Key (CRITICAL) > Lock (IMPORTANT) > Oil (user-driven)
- **Theme changes**: Instant (no reload), always allow restoration
- **Button interrupts**: Universal functions work with all panels
- **Error handling**: CRITICAL priority, overrides all other panels
- **Restoration logic**: Smart restoration with priority-based override
- **Invalid states**: Handled gracefully with logging