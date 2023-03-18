/**
 * test_tools.h
 * 
 * Helper data structures for writing unit tests.
 * 
*/

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <ostream>
#include <string>
#include <vector>
#include <math.h>

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
    float get_lower_limit() { return expected - float(fabs(tolerance*expected)); };
    float get_upper_limit() { return expected + float(fabs(tolerance*expected)); };
} range_result;

typedef struct bool_result {
    bool expected;
    bool actual;
    std::string id;
} bool_result;

typedef struct test_logger {
    std::stringstream ss;
    std::vector<float_result> float_results;
    std::vector<range_result> range_results;
    std::vector<bool_result> bool_results;
    test_logger(const char* tn, int ctn, int tnt) { ss << tn << " test " << ctn << " of " << tnt << std::endl; };
    void check_solution();
} test_logger;

void test_logger::check_solution(void) {
    bool failure = false;
    // load all expected values into stringstream
    ss << "expected values:" << std::endl;
    for(auto& result : bool_results) {
        ss << result.id.data() << " = " << (result.expected ? "true":"false") << std::endl;
    }
    for(auto& result : float_results) {
        ss << result.id.data() << " = " << result.expected << std::endl;
    }
    for(auto& result : range_results) {
        ss << result.id.data() << " between " << result.get_lower_limit() << " and " << result.get_upper_limit() << std::endl;
    }
    // load all actual values into stringstream.
    // also check for failures
    ss << "actual values:" << std::endl;
    for(auto& result : bool_results) {
        ss << result.id.data() << " = " << (result.actual ? "true":"false") << std::endl;
        if (result.expected != result.actual) {
            failure = true;
        }
    }
    for(auto& result : float_results) {
        ss << result.id.data() << " = " << result.actual << std::endl;
        if (result.expected != result.actual) {
            failure = true;
        }
    }
    for(auto& result : range_results) {
        ss << result.id.data() << " = " << result.actual << std::endl;
        if (result.expected < result.get_lower_limit() || result.expected > result.get_upper_limit()) {
            failure = true;
        }
    }
    // invoke gtest checkers
    for(auto& result : bool_results) {
        EXPECT_EQ(result.actual, result.expected);
    }
    for(auto& result : float_results) {
        EXPECT_FLOAT_EQ(result.actual, result.expected);
    }
    for(auto& result : range_results) {
        EXPECT_NEAR(result.actual, result.expected, fabs(result.tolerance*result.expected));
    }
    // print stringstream if test failed
    if (failure) {
        // if test failed, print diagnostics
        std::cout << ss.str() << std::endl;
        std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
    } else {
        // if test passed, clear out the log for the next test
        ss.str(std::string());
    }
    // clear out results in case instance of logger is to be reused
    float_results.clear();
    bool_results.clear();
    range_results.clear();
}
