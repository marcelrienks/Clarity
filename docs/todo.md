* update the long press options, there feels like no difference between 1500 and 2500
* review constants, config constants are also located there?
* Why is there a config manager and a preference manager
* review what RegisterLiveUpdateCallbacks() is for
* what is the COMPONENT_PATTERN variable for?
* Review void ActionHandler::Process(), this is meant to ProcessButtonEvents regardless of UI State, but only meant to ExecutePendingActions during Idle, according to flow diagrams, but does not look to be set up that way
* Are there any magic string being used, rather then constants?
* continue review front trigger manager
* review all log errors, and ensure the log message is short enough to be displayed on the screen
* delete commented out code
* Search for any #ifdef
* Review preference manager logic
* Investigate espressif32 version in ini file