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