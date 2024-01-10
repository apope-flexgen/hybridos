# How to unit test:

## Testing framework used:
- DOCTEST (2.4.8)
- install it by using the command:
    - sudo yum install -y --enablerepo=epel-testing doctest-devel

## Run the script:
- run:
    - sh scripts/build_and_run_unit_tests.sh

## requirements:
- fims secret/v2.0.0 branch
    - run: make clean all && sudo make install
    - requires c++17
    - will get you "libfims_special"
- source scripts/benv.sh will set your environment up for compiling everything