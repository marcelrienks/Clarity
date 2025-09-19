* what is actionCount_ used for in action handler?
* Review void ActionHandler::Process(), this is meant to ProcessButtonEvents regardless of UI State, but only meant to ExecutePendingActions during Idle, according to flow diagrams, but does not look to be set up that way
* Are there any magic string being used, rather then constants?
* continue review fromt trigger manager
* review all log errors, and ensure the log message is short enough to be displayed on the screen
* delete commented out code
* Search for any #ifdef
* Review preference manager logic
* Investigate espressif32 version in ini file