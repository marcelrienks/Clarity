* Add a push button to the wokwi diagram, wired up to 3.3v in and out to GPIO 34.
    * This button should trigger on rising edge, so only when pin 33 changes to high
* Implement single button functionality  
this button will have to leverage the trigger functionality, but will have custom functionality depending on which panel is visible at the time. So can this leverage the existing trigger functionality, or should we create a new input tupe functionality that mirrors triggers? I feel like a seperate input functionality would be better, which functions similary to triggers but is isolated
    * Oil Panel: quick press = nothing, long press = load config panel
    * Splash Panel: quick press = stop animation and load default pannel immediately, long press = load config panel
    * Config Panel: quick press = cycle through various options, long press = set highlighted option
    * Error Panel: quick press = cycle through each error, long press = clear all errors
* Create config panel  
Create a dummy config panel, that displays a screen that looks similar to error component, but with grey colours to allow for single button input implementation.
    * Input:
    what should happen if the input button is pressed
            * Short press = cycle through options
            Hard code this temporarily to option 1, option 2, exit
            * Long press = edit option
            Temporarily this will do nothing
* Implement config panel
    * Input:
    what should happen if the input button is pressed
            * Short press = cycle through options
            * Long press = edit option
    * options:
        * Default Panel:  
        When selected it should show a list of all panels that are registered
            * Input:
            what should happen if the input button is pressed
                * Short press = cycle through list
                * Long press = Set Default panel to the panel highlighted
        * Splash Screen:
        When selected it should give an enable/disable option
            * Input:
            what should happen if the input button is pressed
                * Short press = cycle through options
                * Long press = set the option highlighted
        * Exit:  
            * Input:
            what should happen if the input button is pressed
                * Short press = exit and load default panel
                * Long press = nothing