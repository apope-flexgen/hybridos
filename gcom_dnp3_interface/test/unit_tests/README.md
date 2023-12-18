# How to unit test:

## Testing framework used:
- DOCTEST (2.4.8)
- install it by using the command:
    - sudo yum install -y --enablerepo=epel-testing doctest-devel

## Run the test cases:
- Build from the main directory (gcom_interface) using: `make BUILD_MODE=test`
- Run tests using `./build/test/dnp3_test`