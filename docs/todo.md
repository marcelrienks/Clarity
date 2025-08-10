* Consolidate panel switching mechanism between PanelManager, and InputManager  
If possible they should use a similar mechanism in the form of a simplePanelSwitchAction
* Expand on the idea of interrupts, where we have two types, triggers and input
    * I think I forgot that triggers do not use queues any more, they just check current state. If so that means input will not be able to work like that
        * If so think through input, and how best it can be built with the idea of letting main loop check for input during idle, and actioning
* Ensure that in between animation of splash and oil panel needle, that interrupts are checked
* remove all comments based on changes made, so remove temp, test simplified
* Ensure that all tests now inject using constructor, and not methods
* Rewrite architecture document by summarising the current architecture, and highlight the MVP pattern, Trigger Interupts, and Input Event handling.  
Also highlight the normal flow of panel loading, and the ability to show error messages, change configs, and handle button inputs which force an action for each panel
* because of error panels, ensure that there are proper null and empty checks throughout the code
* Complete unity and wokwi tests for error panel
* create unity and wokwi tests for input
    * this includes all panels
    * required tests for config panels
* Replace any GetInstance() with Instance()
* Ensure that any pointer variables/arguments have a name which indicates that it's a pointer
* Review the concept behind services, what is the difference between IPanel, and IPanelService?
    * Whats the difference between managers, handlers, and services