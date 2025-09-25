* all null empty checks should have error handling associated, no point in preventing a crash, but then the application is not in a functional state
* throttling of repetative sensor and panel updates
* Are there any magic string being used, rather then constants?
* continue review front trigger manager
* review all log errors, and ensure the log message is short enough to be displayed on the screen
* delete commented out code
* Search for any #ifdef
* Review preference manager logic
* Investigate espressif32 version in ini file