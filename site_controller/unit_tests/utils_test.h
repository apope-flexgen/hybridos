#ifndef UTILS_TEST_H_
#define UTILS_TEST_H_

#include "gtest/gtest.h"
#include "Site_Controller_Utils.h"

TEST (UtilsTest, split) {

    //struct that has variables to configure for each test case
    struct test_data
    {
        std::string input_string;
        std::string delimiter;
        std::vector<std::string> output_fragments;
    };

    std::vector<test_data> tests;

    tests.push_back({
        "this/is/a/test",
        "/",
        {"this", "is", "a", "test"}
    });

    tests.push_back({
        "/this/is/a/test",
        "/",
        {"this", "is", "a", "test"}
    });

    tests.push_back({
        "this//is//a//test//",
        "/",
        {"this", "is", "a", "test"}
    });

    for (auto test : tests)
    {
		capture_stdout();
        auto result = split(test.input_string, test.delimiter);
        EXPECT_EQ(result, test.output_fragments);
        release_stdout(result != test.output_fragments);
    }
}

TEST (UtilsTest, near) {
    //struct that has variables to configure for each test case
    struct test_data
    {
        double input;
        double reference;
        double tolerance;
        bool expected_result;
    };

    std::vector<test_data> tests{
        {100.0001,  100.0,      1.0,    true},
        {100.001,   100.0,      1.0,    true},
        {100.01,    100.0,      1.0,    true},
        {100.1,     100.0,      1.0,    true},
        {101.0,     100.0,      1.0,    false},
        {101.0001,  100.0,      1.0,    false},
        {100.0,     100.0001,   1.0,    true},
        {100.0,     100.001,    1.0,    true},
        {100.0,     100.01,     1.0,    true},
        {100.0,     100.1,      1.0,    true},
        {100.0,     101.0,      1.0,    false},
        {100.0,     101.0001,   1.0,    false},
        {100.0001,  100.0,      0.1,    true},
        {100.001,   100.0,      0.1,    true},
        {100.01,    100.0,      0.1,    true},
        {100.1,     100.0,      0.1,    true}, // Compiler specific, rounds < 0.1
        {100.1001,  100.0,      0.1,    false},
        {101.0001,  100.0,      0.1,    false},
        {100.0,     100.0001,   0.1,    true},
        {100.0,     100.001,    0.1,    true},
        {100.0,     100.01,     0.1,    true},
        {100.0,     100.1,      0.1,    true}, // Compiler specific, rounds < 0.1
        {100.0,     100.1001,   0.1,    false},
        {100.0,     101.0001,   0.1,    false},
        {100.0001,  100.0,      0.01,   true},
        {100.001,   100.0,      0.01,   true},
        {100.01,    100.0,      0.01,   false},
        {100.0101,  100.0,      0.01,   false},
        {100.1001,  100.0,      0.01,   false},
        {101.0001,  100.0,      0.01,   false},
        {100.0,     100.0001,   0.01,   true},
        {100.0,     100.001,    0.01,   true},
        {100.0,     100.01,     0.01,   false},
        {100.0,     100.0101,   0.01,   false},
        {100.0,     100.1001,   0.01,   false},
        {100.0,     101.0001,   0.01,   false}
    };

    int test_counter = 1;
    for (auto test : tests)
    {
		capture_stdout();
        bool result = near(test.input, test.reference, test.tolerance);
        std::cout << "near() test " << test_counter << " of " << tests.size() << std::endl;
        EXPECT_EQ(result, test.expected_result);
        release_stdout(result != test.expected_result);
        test_counter++;
    }
}

TEST (UtilsTest, less_or_near) {
    //struct that has variables to configure for each test case
    struct test_data
    {
        double input;
        double reference;
        double tolerance;
        bool expected_result;
    };

    std::vector<test_data> tests{
        {100.0001,  100.0,      1.0,    true},
        {100.001,   100.0,      1.0,    true},
        {100.01,    100.0,      1.0,    true},
        {100.1,     100.0,      1.0,    true},
        {101.0,     100.0,      1.0,    false},
        {101.0001,  100.0,      1.0,    false},
        {100.0,     100.0001,   1.0,    true},
        {100.0,     100.001,    1.0,    true},
        {100.0,     100.01,     1.0,    true},
        {100.0,     100.1,      1.0,    true},
        {100.0,     101.0,      1.0,    true},
        {100.0,     101.0001,   1.0,    true},
        {100.0001,  100.0,      0.1,    true},
        {100.001,   100.0,      0.1,    true},
        {100.01,    100.0,      0.1,    true},
        {100.1,     100.0,      0.1,    true}, // Compiler specific, rounds < 0.1
        {100.1001,  100.0,      0.1,    false},
        {101.0001,  100.0,      0.1,    false},
        {100.0,     100.0001,   0.1,    true},
        {100.0,     100.001,    0.1,    true},
        {100.0,     100.01,     0.1,    true},
        {100.0,     100.1,      0.1,    true}, // Compiler specific, rounds < 0.1
        {100.0,     100.1001,   0.1,    true},
        {100.0,     101.0001,   0.1,    true},
        {100.0001,  100.0,      0.01,   true},
        {100.001,   100.0,      0.01,   true},
        {100.01,    100.0,      0.01,   false},
        {100.0101,  100.0,      0.01,   false},
        {100.1001,  100.0,      0.01,   false},
        {101.0001,  100.0,      0.01,   false},
        {100.0,     100.0001,   0.01,   true},
        {100.0,     100.001,    0.01,   true},
        {100.0,     100.01,     0.01,   true},
        {100.0,     100.0101,   0.01,   true},
        {100.0,     100.1001,   0.01,   true},
        {100.0,     101.0001,   0.01,   true}
    };

    int test_counter = 1;
    for (auto test : tests)
    {
		capture_stdout();
        bool result = less_than_or_near(test.input, test.reference, test.tolerance);
        std::cout << "less_than_or_near() test " << test_counter << " of " << tests.size() << std::endl;
        EXPECT_EQ(result, test.expected_result);
        release_stdout(result != test.expected_result);
        test_counter++;
    }
}

TEST (UtilsTest, greater_or_near) {
    //struct that has variables to configure for each test case
    struct test_data
    {
        double input;
        double reference;
        double tolerance;
        bool expected_result;
    };

    std::vector<test_data> tests{
        {100.0001,  100.0,      1.0,    true},
        {100.001,   100.0,      1.0,    true},
        {100.01,    100.0,      1.0,    true},
        {100.1,     100.0,      1.0,    true},
        {101.0,     100.0,      1.0,    true},
        {101.0001,  100.0,      1.0,    true},
        {100.0,     100.0001,   1.0,    true},
        {100.0,     100.001,    1.0,    true},
        {100.0,     100.01,     1.0,    true},
        {100.0,     100.1,      1.0,    true},
        {100.0,     101.0,      1.0,    false},
        {100.0,     101.0001,   1.0,    false},
        {100.0001,  100.0,      0.1,    true},
        {100.001,   100.0,      0.1,    true},
        {100.01,    100.0,      0.1,    true},
        {100.1,     100.0,      0.1,    true}, // Compiler specific, rounds < 0.1
        {100.1001,  100.0,      0.1,    true},
        {101.0001,  100.0,      0.1,    true},
        {100.0,     100.0001,   0.1,    true},
        {100.0,     100.001,    0.1,    true},
        {100.0,     100.01,     0.1,    true},
        {100.0,     100.1,      0.1,    true}, // Compiler specific, rounds < 0.1
        {100.0,     100.1001,   0.1,    false},
        {100.0,     101.0001,   0.1,    false},
        {100.0001,  100.0,      0.01,   true},
        {100.001,   100.0,      0.01,   true},
        {100.01,    100.0,      0.01,   true},
        {100.0101,  100.0,      0.01,   true},
        {100.1001,  100.0,      0.01,   true},
        {101.0001,  100.0,      0.01,   true},
        {100.0,     100.0001,   0.01,   true},
        {100.0,     100.001,    0.01,   true},
        {100.0,     100.01,     0.01,   false},
        {100.0,     100.0101,   0.01,   false},
        {100.0,     100.1001,   0.01,   false},
        {100.0,     101.0001,   0.01,   false}
    };

    int test_counter = 1;
    for (auto test : tests)
    {
		capture_stdout();
        bool result = greater_than_or_near(test.input, test.reference, test.tolerance);
        std::cout << "greater_than_or_near() test " << test_counter << " of " << tests.size() << std::endl;
        EXPECT_EQ(result, test.expected_result);
        release_stdout(result != test.expected_result);
        test_counter++;
    }
}

#endif /* UTILS_TEST_H_ */
