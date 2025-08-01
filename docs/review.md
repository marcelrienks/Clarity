* is there any specific init/integration testing value in the separation of the Entry Point & Bootstrap?  
Could and should main, clarity_bootstrap, clarity_application be merged for simplicity, and removing of layers just for layer sake?
* Are all the functions in esp32_gpio_provider actually used, where is the write used?  
Also this needs to be renamed simply to gpio_provider.cpp (including the header)
* Confirm the purpose of the providers, is it simply to allow for unit/integration testing
* The idea that managers implement services seems a little flawed?  
Should IServices be renamed to IManagers, or should managers be renamed to services
* There seems to be inconsistency around service_container and component_registry, do they not serve similar functions, if so should they not have a more consistent naming?  
What is the difference between a container, and a registry?