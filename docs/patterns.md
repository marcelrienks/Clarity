# Development Patterns

## MVP Architecture
Model-View-Presenter pattern with clear separation of concerns.

## State Management
Each panel manages BUSY or IDLE state due to proximity to LVGL and Timer logic.

## Error Handling
Early validation and return patterns with comprehensive error handling.

## Logging Standards
- **Verbose logs (log_v)**: Method entry indication
- **Debug logs (log_d)**: Complex activities within method bodies
- **Info logs (log_i)**: Major functional and logical system operations