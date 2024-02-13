# `/scripts`

Scripts to test go_metrics functionality

## Running tests
In `scripts/test_funcs.sh` change 'dir' (Line 21) to the directory that your test script resides in. Then, from the hybridos/go_metrics directory, run `sh scripts/<dir>/<script> start`

## Running all tests in a folder
In `scripts/test_funcs.sh` change 'dir' (Line 21) to the directory that your test scripts resides in. Then, from the hybridos/go_metrics directory, run
```
for f in scripts/<dir>/*.sh; do
  bash "$f" start
done
```