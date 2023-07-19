# Unit Testing
Author: Stephanie Reynolds

Updated: 4/20/2023

## How to run
Unit testing is built into Golang. To run the already written unit tests, navigate to `/go_metrics/src` and run the following command:
```
go test -coverprofile=test.txt
```

In addition to giving a quick `PASS/FAIL` response, this will yield an output file `test.txt` that details coverage information on each statement within the package. You can also optionally enable "verbose" output using the `-v` flag, which gives a reading of how long each of the test cases for `functions.go` takes to run. Verbose information has not been built into any of the other functions.

Once `test.txt` has been generated, you can view a function-by-function breakdown of the unit test coverage by running:
```
go tool cover -func=test.txt
```