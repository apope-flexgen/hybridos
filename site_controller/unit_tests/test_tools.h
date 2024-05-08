/**
 * test_tools.h
 *
 * Helper data structures for writing unit tests.
 *
 */
#ifndef TEST_TOOLS_H_
#define TEST_TOOLS_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <ostream>
#include <string>
#include <vector>
#include <math.h>
#include <Site_Controller_Utils.h>
#include "site_manager_mock.h"

typedef struct float_result {
    float expected;
    float actual;
    std::string id;
} float_result;

typedef struct range_result {
    float expected;
    float tolerance;
    float actual;
    std::string id;
    // Use tolerance as absolute if expected is 0
    float get_lower_limit() { return expected - float(fabs(tolerance * (expected == 0.0f ? 1.0f : expected))); };
    float get_upper_limit() { return expected + float(fabs(tolerance * (expected == 0.0f ? 1.0f : expected))); };
} range_result;

typedef struct bool_result {
    bool expected;
    bool actual;
    std::string id;
} bool_result;

typedef struct int_result {
    int expected;
    int actual;
    std::string id;
} int_result;

typedef struct string_result {
    std::string expected;
    std::string actual;
    std::string id;
} string_result;

typedef struct test_logger {
    std::stringstream ss;
    std::vector<float_result> float_results;
    std::vector<range_result> range_results;
    std::vector<bool_result> bool_results;
    std::vector<int_result> int_results;
    std::vector<string_result> string_results;
    test_logger(const char* tn, int ctn, int tnt);
    void check_solution();
} test_logger;

test_logger::test_logger(const char* tn, int ctn, int tnt) {
    ss << tn << " test " << ctn << " of " << tnt << std::endl;
    capture_stdout();
    std::cout << "stdout captured during test execution:" << std::endl;
}

void test_logger::check_solution(void) {
    bool failure = false;
    // load all expected values into stringstream
    ss << "expected values:" << std::endl;
    for (auto& result : int_results) {
        ss << result.id.data() << " = " << result.expected << std::endl;
    }
    for (auto& result : bool_results) {
        ss << result.id.data() << " = " << (result.expected ? "true" : "false") << std::endl;
    }
    for (auto& result : float_results) {
        ss << result.id.data() << " = " << result.expected << std::endl;
    }
    for (auto& result : string_results) {
        ss << result.id.data() << " = " << result.expected << std::endl;
    }
    for (auto& result : range_results) {
        ss << result.id.data() << " between " << result.get_lower_limit() << " and " << result.get_upper_limit() << std::endl;
    }
    // load all actual values into stringstream.
    // also check for failures
    ss << "actual values:" << std::endl;
    for (auto& result : int_results) {
        ss << result.id.data() << " = " << result.actual << std::endl;
        if (result.expected != result.actual) {
            failure = true;
        }
    }
    for (auto& result : bool_results) {
        ss << result.id.data() << " = " << (result.actual ? "true" : "false") << std::endl;
        if (result.expected != result.actual) {
            failure = true;
        }
    }
    for (auto& result : float_results) {
        ss << result.id.data() << " = " << result.actual << std::endl;
        if (result.expected != result.actual) {
            failure = true;
        }
    }
    for (auto& result : string_results) {
        ss << result.id.data() << " = " << result.actual << std::endl;
        if (result.expected != result.actual) {
            failure = true;
        }
    }
    for (auto& result : range_results) {
        ss << result.id.data() << " = " << result.actual << std::endl;
        if (result.actual < result.get_lower_limit() || result.actual > result.get_upper_limit()) {
            failure = true;
        }
    }

    release_stdout(failure);

    // invoke gtest checkers
    for (auto& result : int_results) {
        EXPECT_EQ(result.actual, result.expected);
    }
    for (auto& result : bool_results) {
        EXPECT_EQ(result.actual, result.expected);
    }
    for (auto& result : float_results) {
        EXPECT_FLOAT_EQ(result.actual, result.expected);
    }
    for (auto& result : string_results) {
        EXPECT_EQ(result.actual, result.expected);
    }
    for (auto& result : range_results) {
        EXPECT_NEAR(result.actual, result.expected, fabs(result.tolerance * (result.expected == 0.0f ? 1.0f : result.expected)));
    }
    // print stringstream if test failed
    if (failure) {
        // print test logs
        std::cout << ss.str() << std::endl;
        std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
    } else {
        // if test passed, clear out the log for the next test
        ss.str(std::string());
    }
    // clear out results in case instance of logger is to be reused
    int_results.clear();
    float_results.clear();
    bool_results.clear();
    range_results.clear();
}

// Can be called when a generic defaults object is needed for testing.
void fill_in_defaults(Fims_Object& defaults) {
    Site_Manager_Mock temp_site_man;
    std::string raw_defaults_json = R"(
        {
            "name": "Default Variable Settings",
            "unit": "",
            "ui_type": "status",
            "type": "number",
            "var_type": "Float",
            "value": 0.0,
            "scaler": 1,
            "ui_enabled": true,
            "multiple_inputs": false,
            "options": []
        }
    )";
    cJSON* defaults_json = cJSON_Parse(raw_defaults_json.c_str());
    temp_site_man.call_parse_default_vals(defaults_json, defaults);
}

#endif /* TEST_TOOLS_H_ */
