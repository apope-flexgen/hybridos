

from pytest_cases import parametrize, fixture

from ..pytest_framework import Site_Controller_Instance
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_steps import Setup, Teardown, Steps

# ldss testing
# I've made the assumption throughout this suite that the SoC should be between 30 and 70


@ fixture
@ parametrize("test", [
    # Preconditions - enable ldss
    Setup(
        "test_ldss",
        {
            "/features/standalone_power/ldss_enable_flag": True,
            "/features/standalone_power/ldss_start_gen_time": 1,
            "/features/standalone_power/ldss_stop_gen_time": 1,
        },
        [
            *[
                Flex_Assertion(
                    Assertion_Type.approx_eq,
                    f"/features/standalone_power/{name}",
                    expected,
                    wait_secs=.1
                ) for (name, expected) in [
                    ("ldss_enable_flag", True),
                    ("ldss_enable_soc_threshold", True),
                    ("ldss_max_load_threshold_percent", 90),
                    ("ldss_max_soc_threshold_percent", 90),
                    ("ldss_min_load_threshold_percent", 50),
                    ("ldss_min_soc_threshold_percent", 10),
                ]
            ],
        ]
    ),

    # SoC is too high, generator should turn off
    Steps(
        {
            "/features/standalone_power/ldss_max_soc_threshold_percent": 20
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/num_gen_running", 0, wait_secs=5),
        ]
    ),

    # SoC is too low, generator should turn back on
    Steps(
        {
            "/features/standalone_power/ldss_max_soc_threshold_percent": 90,
            "/features/standalone_power/ldss_min_soc_threshold_percent": 80
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/num_gen_running", 1, wait_secs=5),
        ]
    ),

    Teardown(
        {
            "/features/standalone_power/ldss_enable_flag": False,
            "/features/standalone_power/ldss_min_soc_threshold_percent": 10,
            "/features/standalone_power/ldss_max_soc_threshold_percent": 90,
            "/features/standalone_power/ldss_start_gen_time": 30,
            "/features/standalone_power/ldss_stop_gen_time": 30,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/features/standalone_power/ldss_enable_flag", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/summary/num_gen_running", 1, wait_secs=5),
        ]
    )
])
def test_ldss(test):
    return test
