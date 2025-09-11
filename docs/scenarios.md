# Test Scenarios

**Related Documentation:**
- **[Requirements](requirements.md)** - Complete trigger scenarios and panel behaviors
- **[Architecture](architecture.md)** - System components and interaction patterns
- **[Interrupt Architecture](interrupt-architecture.md)** - Trigger priority and processing logic

## Primary Integration Test Scenario

This scenario demonstrates the complete system functionality following a realistic user interaction flow:

### 1. System Startup
- **System Start** → Splash panel loads and animates with day theme
- **Splash Animation Completes** → Oil panel loads automatically
- **Oil Panel Initial State**: Pressure and Temperature scales animate to preset start values (demonstrating gauge functionality)

### 2. Real-time Sensor Interaction
- **Adjust Pressure Input** → Pressure gauge animates smoothly to reflect new value
- **Adjust Temperature Input** → Temperature gauge animates smoothly to reflect new value
- **Oil Panel**: Both gauges respond dynamically to sensor changes with continuous animation

### 3. Theme Change (Non-Blocking Style Trigger)
- **Lights Trigger Activates** → Night theme applied instantly to Oil Panel
- **Oil Panel**: Continues displaying with night theme without panel reload
- **Lights Trigger**: STYLE type trigger, always maintains and doesn't block restoration

### 4. Panel Loading Trigger (IMPORTANT Priority)
- **Lock Trigger Activates** → Lock panel loads (IMPORTANT priority)
- **Lights Trigger**: Still active, night theme maintained
- **Lock Panel**: Displays lock status with night theme

### 5. Higher Priority Key Trigger Override
- **Key Not Present Trigger Activates** → Key panel loads with red key icon (CRITICAL priority)
- **Key Trigger**: Higher priority than lock, overrides lock panel immediately
- **Key Panel**: Red key icon displayed with night theme

### 6. Same Priority Key State Transition
- **Key Present Trigger Activates** → Key panel reloads with green key icon (CRITICAL priority)
- **Key Panel Update**: Panel reloads to show green key icon (key present state)
- **Key Not Present Trigger**: Automatically deactivates when key present activates

### 7. Key State Reversal and Panel Persistence
- **Key Not Present Trigger Deactivates** → Key panel maintains green key icon
- **Key Present Trigger**: Still active, so Key panel continues showing green icon
- **Key Panel**: Remains displayed with green key icon (no state change)

### 8. Priority-Based Restoration Chain
- **Key Present Trigger Deactivates** → Lock panel loads (restoration to next highest priority)
- **Lock Trigger**: Still active (IMPORTANT priority), becomes highest active trigger
- **Lock Panel**: Loads and displays with night theme

### 9. Error System Integration with Multiple Errors
- **Debug Error Trigger Activates** → Error panel loads with 3 reported errors (CRITICAL priority)
- **Error Trigger**: Highest priority, overrides all other panels
- **Error Panel**: Displays error list with night theme, showing all 3 error messages

### 10. Error Navigation and Interaction
- **Short Press Action Button** → Cycle through one error message
- **Error Panel**: Advances to next error in queue, marking current as viewed
- **Error Navigation**: User can cycle through all 3 errors for review

### 11. Error Resolution and Priority Restoration
- **Long Press Action Button** → Clear all errors and exit Error panel
- **Error Trigger**: Deactivates after clearing error queue
- **Lock Panel**: Loads (next highest priority active trigger - IMPORTANT)

### 12. Complete Trigger Chain Restoration
- **Lock Trigger Deactivates** → Oil panel loads with night theme
- **No Active Panel Triggers**: All PANEL-type triggers inactive, restore to user panel
- **Oil Panel**: Displays with night theme, Pressure and Temperature scales animate to preset values

### 13. Theme Restoration to Day Mode
- **Lights Trigger Deactivates** → Day theme applied to Oil Panel
- **Oil Panel**: Continues displaying with day theme without panel reload
- **Theme Change**: Instant theme transition, no interruption to panel operation

### 14. Configuration Access via Button Action
- **Long Press Action Button** → Config panel loads with main preference settings
- **Config Panel**: Main menu displayed with day theme, showing available options
- **Panel Transition**: Oil panel → Config panel via universal long press function

### 15. Configuration Navigation to Theme Settings
- **Short Press Repeatedly** → Navigate through options until theme settings reached
- **Config Panel**: Cycles through main menu options: Default Panel, Theme Settings, Exit
- **Theme Settings**: Option highlighted when navigation reaches it

### 16. Theme Submenu Access
- **Long Press Action Button** → Theme option settings displayed
- **Config Panel**: Enters theme submenu, showing Day/Night theme options
- **Submenu State**: Theme configuration options now available for selection

### 17. Theme Selection Navigation
- **Short Press Navigation** → Cycle through theme options until Night selected
- **Config Panel**: Cycles between Day Theme and Night Theme options
- **Night Option**: Highlighted when navigation reaches it

### 18. Theme Setting Application
- **Long Press Action Button** → Apply night theme and return to main preference settings
- **Config Panel**: Applies night theme setting, returns to main config menu
- **Theme Applied**: Night theme now active across all panels, main menu displayed

### 19. Configuration Exit Navigation
- **Short Press Repeatedly** → Navigate through options until Exit reached
- **Config Panel**: Cycles through main menu options until Exit option highlighted
- **Exit Option**: Final option in main configuration menu

### 20. Return to Oil Panel with Applied Settings
- **Long Press Action Button** → Oil panel loads with night theme
- **Config Exit**: Executes exit function, loads Oil panel as default user panel
- **Oil Panel Final State**: Displays with applied night theme, Pressure and Temperature scales animate to preset values

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