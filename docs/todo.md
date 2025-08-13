* review the intended use of the lib folder and check if there is any source that matches the use case, like ticker.cpp
* I don't like the idea of the factory registration, can this not be absored by the relevant factories?
* Document current arch, and relationships
* What logic is over complicated, or over engineered? Is there any optimization, or simplification that can be done, without loosing any of the current functionality?
* should there be a panel factory?
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