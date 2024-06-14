# How to unit test:

## Run the test cases:
- Build from the main directory (gcom_modbus_interface) using: `make BUILD_MODE=test`
- Run tests using `./build/test/modbus_test`

## To get test coverage information
- Build and run tests to generate coverage information.
- `cd ./build/test_obj`
- Use `gcov ../../src/<filename>.cpp -o .` to get coverage information about that specific C++ file. I haven't figured out the best way to do this yet, so you might need to search for the specific file of interest in the disgusting mess of output that pops up.
- A full line-by-line report is generated in `build/test_obj/<filename>.cpp.gcov`