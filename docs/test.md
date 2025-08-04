# Clarity Test Suite Documentation

## Criteria:
* All tests must run
* All tests must pass
* Code coverage is more important than number of tests
* Ideally one command to run all tests, but if that is not viable, multiple commands can be included in a test runner
* Preferably build unit tests with Unity and platformIO
* There is no preference between multiple test files, or one consolidated test file, as long as it all runs
* Failing tests are not acceptable
* Commented out tests are not acceptable

## Known Limitations:
* PlatformIO regardless of build filters will attempt to build and consolidate all tests at run time. Test Architecture must take this into account
* PlatformIO and Unity require all files in the root test directory

## Todo:
* Review PlatformIO and Unity test best practices, and recommendations
* Implement unit tests only, to ensure maximum code coverage, and core functionality. But ensure it is done in such a way that test can easily be expanded across all code files