* because of error panels, ensure that there are proper null and empty checks throughout the code
* review logging, and the level between debug and info
    * every method (within reason) should have a debug log
    * every major concept (managers, panels, components etc.) should have an info log at key points
* Ensure that all function comments have the same param name as the actual argument in code
* Document current arch, and relationships
* What logic is over complicated, or over engineered? Is there any optimization, or simplification that can be done, without loosing any of the current functionality?
* Ensure that all tests now inject using constructor, and not methods
* Rewrite architecture document by summarising the current architecture, and highlight the MVP pattern, Trigger Interrupts, and Input Event handling.  
Also highlight the normal flow of panel loading, and the ability to show error messages, change configs, and handle button inputs which force an action for each panel
* Complete unity and wokwi tests for error panel
* create unity and wokwi tests for input
    * this includes all panels
    * required tests for config panels
* Replace any GetInstance() with Instance()
* Ensure that any pointer variables/arguments have a name which indicates that it's a pointer
* Review the concept behind services, what is the difference between IPanel, and IPanelService?
    * Whats the difference between managers, handlers, and services