# Test Scenarios

## Major Integration Test

Full system test covering all triggers and transitions:

1. **Startup**: Splash → Oil panel (day theme)
2. **Night Mode**: Light sensor high → Theme changes to night (red)
3. **Lock Active**: Lock sensor high → Lock panel loads
4. **Key States**: 
   - Key not present → Key panel (red icon)
   - Key present → Key panel (green icon)
5. **State Resolution**: Clear triggers → Return to oil panel
6. **Day Mode**: Light sensor low → Theme changes to day (white)
7. **Configuration**: Oil Panel Long press → Config Panel
8. **Navigate Config**: Config panel → short press until theme setting
9. **Set Config**: Config Panel change theme → Night
10. **Exit Config**: Navigate to exit → Long Press

## Individual Scenarios

### Startup
- Default: Splash (day theme) → Oil panel
- With sensors: Needles animate to sensor values
- With triggers: Skip oil panel, load triggered panel
- With triggers: Set theme before loading any panels

### Key Triggers
- **Present**: Green icon, priority over oil panel
- **Not Present**: Red icon, priority over oil panel

### Lock Trigger
- Takes priority over oil panel
- Lower priority than key triggers

### Theme Switching
- Light sensor controls day/night theme
- No panel reload on theme change
- Persists across panel switches

## Testing Notes
- Panel priorities: Key > Lock > Oil
- Theme changes are instant (no reload)
- Invalid states handled gracefully