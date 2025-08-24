## MVP:

## State:
Each individual panel is responsible for setting the state to BUSY or IDLE due to the panels being closer to the actual LVGL or Timer logic that consumes processing.

## "Early Return":
Ensure early validation and return or error handling

## Logging:
  1. Verbose logs (log_v) - Only for method entry (indicating the method has been called)
  2. Debug logs (log_d) - Only within method bodies for complex activities
  3. Info logs (log_i) - Only for major functional/logical parts of the system