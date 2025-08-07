* Create config panel  
Create a dummy config panel, that displays a screen that looks similar to error component, but with grey colours to allow for single button input implementation.
    * Input:
    what should happen if the input button is pressed
            * Short press = cycle through options
            Hard code this temporarily to option 1, option 2, exit
            * Long press = edit option
            Temporarily this will do nothing
* Implement single button functionality  
this button will have custom functionality depending on which panel is visible at the time
    * Oil Panel: quick press = nothing, long press = load config panel
    * Splash Panel: quick press = stop animation and load default pannel immediately, long press = load config panel
    * Config Panel: quick press = cycle through various options, long press = set highlighted option
    * Error Panel: quick press = cycle through each error, long press = clear all errors
* Implement config panel
    * does it rely on triggers, because it will be accessed by long pressing the input button?
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