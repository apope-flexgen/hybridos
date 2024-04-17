# Behave Testing

## Basic Setup
In your development environment, install `behave`:
```
python3 -m pip install behave
```

## Running All Tests

To run all tests simply call the `behave` command from the `go_metrics/test` directory:
```
behave
```

Because this might be too verbose, you can also run behave with the progress2 format, which shows dotted progress for each executed step:
```
behave -f progress2
```

## Running Specific Tests
If you have a specific set of tests that you want to run, you should identify what tag pertains to those tests and then use the following command to run the tests from the `go_metrics/test` directory:
```
behave --tags=<tag>
```
For example, all tests in the `features/math` folder should be tagged with `@math`. Thus, to run all math tests, you would run:
```
behave --tags=math
```

## Writing New Tests
To add a new test, you will need to develop a new `*.feature` file. See the [behave documentation](https://behave.readthedocs.io/en/stable/tutorial.html) for information about how to develop a feature file.

For the most part, you shouldn't need to change the `steps.py` file to add new tests. Most steps relevant to `go_metrics` have already been implemented.