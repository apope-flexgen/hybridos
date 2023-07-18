import os

# Test report formatted as markdown table
logfile = None


# Setup a directory for reports
def setup_report_dir():
    if not os.path.exists('reports'):
        os.makedirs('reports')


# Setup an individual report (test table)
def setup_report(name: str):
    name = name.lower().replace(" ", "_")
    global logfile
    logfile = open(f"reports/{name}.log", "w")
    logfile.write(f"### {name}\n")
    logfile.write("|ID|Steps|Expected|Actual|\n|-|-|-|-")


# Close the report
def close_report():
    global logfile
    logfile.write("|\n")
    logfile.close()


# Report the id of the test
def report_id(id):
    # Pytest will generate ids with both the test name (function name) and unique test object appended
    # For example "reactive_power_poi_limits-test1"
    # The report should already have a header with the name, so remove it from the id
    id = id[id.find("-")+1:]
    global logfile
    # Close out the previous line before reporting this line
    # Resolves the case where a test failed and couldn't close its line
    logfile.write(f"|\n|{id}")


# Write a key value pair to the report
def report_pair(key, value):
    global logfile
    logfile.write(f"{key}: {value} ")


# Report a list of fims uri value commands
# Used by steps, expected value pairs, and actual value pairs
def report_fims_cmds(cmds):
    global logfile
    logfile.write("|")
    if isinstance(cmds, dict):
        for uri, value in cmds.items():
            report_pair(uri, value)
    else:
        for uri, value in cmds:
            report_pair(uri, value)


# Report the steps of the test
def report_steps(steps):
    report_fims_cmds(steps)


# Report the expected values of the test
def report_expected(expected):
    expected_values = []
    if isinstance(expected, list):
        # Use a list of tuples so that duplicate keys with different values can be used
        # TODO: steps are still stored in dictionary which would disallow this. I don't think a use case
        # exists where duplicate steps would be useful, but reevalute if that becomes the case
        for assertion in expected:
            expected_values.append((assertion.uri, assertion.report()))
    else:
        expected_values = {expected.uri: expected.report()}
    report_fims_cmds(expected_values)
    # Close out the expected cell to start the actual cell
    global logfile
    logfile.write("|")


# Report the actual result of the test (single result)
# This is different from the other calls as the actual values must be queried individually
# at specific times rather than all together
def report_actual(uri, value):
    report_pair(uri, value)
