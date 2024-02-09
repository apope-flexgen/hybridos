# Sequences tests
from pytest_cases import parametrize, fixture
from ..assertion_framework import Assertion_Type, Flex_Assertion
from ..pytest_steps import Setup, Steps

"""
This test relies on a specific branch on config found here. (https://github.com/flexgen-power/config/tree/AGT_11.3)
It assesses the following:
    - Do new ranged templates behave as expected?
    - ESS is a ranged template with 2 templates.
        - [1,3,4] are template 1
        - [2,5] are template 2
"""
@ fixture
@ parametrize("test", [
    Setup(
        "templates_ranged",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 5, wait_secs=10),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 5),
        ]
    ),
    # pub a fault to ess_psm_01 and make sure ess_01 picks it up
    Steps(
        [
            # setup (of which there are none)
            {
            },
            # pubs
            {
                "/components/ess_psm_01": {"faults": [{"string": "Something alarming is happening", "value": 1}]}
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/faults", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 4),
        ]
    ),
    # clear the fault
    Steps(
        [
            {
                "/site/operation/clear_faults_flag": True
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_01/faults", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 5),
        ]
    ),
    # pub a fault to ess_psm_02 and make sure ess_02 picks it up
    Steps(
        [
            # setup (of which there are none)
            {
            },
            # pubs
            {
                "/components/ess_psm_02": {"faults": [{"string": "Something alarming is happening", "value": 1}]}
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/faults", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 4),
        ]
    ),
    # clear the fault
    Steps(
        [
            {
                "/site/operation/clear_faults_flag": True
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_02/faults", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 5),
        ]
    ),
    # pub a fault to ess_psm_03 and make sure ess_03 picks it up
    Steps(
        [
            # setup (of which there are none)
            {
            },
            # pubs
            {
                "/components/ess_psm_03": {"faults": [{"string": "Something alarming is happening", "value": 1}]}
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_03/faults", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 4),
        ]
    ),
    # clear the fault
    Steps(
        [
            {
                "/site/operation/clear_faults_flag": True
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_03/faults", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 5),
        ]
    ),
    # pub a fault to ess_psm_04 and make sure ess_04 picks it up
    Steps(
        [
            # setup (of which there are none)
            {
            },
            # pubs
            {
                "/components/ess_psm_04": {"faults": [{"string": "Something alarming is happening", "value": 1}]}
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_04/faults", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 4),
        ]
    ),
    # clear the faults
    Steps(
        [
            {
                "/site/operation/clear_faults_flag": True
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_04/faults", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 5),
        ]
    ),
    # pub a fault to ess_psm_05 and make sure ess_05 picks it up
    Steps(
        [
            # setup (of which there are none)
            {
            },
            # pubs
            {
                "/components/ess_psm_05": {"faults": [{"string": "Something alarming is happening", "value": 1}]}
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_05/faults", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 4),
        ]
    ),
    # clear the faults
    Steps(
        [
            {
                "/site/operation/clear_faults_flag": True
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_05/faults", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 5),
        ]
    ),
])
def test_ranged(test):
    return test

"""
This test relies on a specific branch on config found here. (https://github.com/flexgen-power/config/tree/AGT_11.3)
It assesses the following:
    - Do traditional templates still function correctly?
    - Solar is a template with number_of_instances = 5
"""
@ fixture
@ parametrize("test", [
    Setup(
        "templates_traditional",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_running", 5, wait_secs=10),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_available", 5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/summary/num_ess_controllable", 5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_running", 5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 5),
        ]
    ),
    # pub a fault to pv_01 and make sure solar_01 picks it up
    Steps(
        [
            # setup (of which there are none)
            {
            },
            # pubs
            {
                "/components/pv_01": {"faults": [{"string": "Something alarming is happening", "value": 1}]}
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_01/faults", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 4),
        ]
    ),
    # clear the fault
    Steps(
        [
            {
                "/site/operation/clear_faults_flag": True
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_01/faults", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 5),
        ]
    ),
    # pub a fault to pv_02 and make sure solar_02 picks it up
    Steps(
        [
            # setup (of which there are none)
            {
            },
            # pubs
            {
                "/components/pv_02": {"faults": [{"string": "Something alarming is happening", "value": 1}]}
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_02/faults", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 4),
        ]
    ),
    # clear the fault
    Steps(
        [
            {
                "/site/operation/clear_faults_flag": True
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_02/faults", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 5),
        ]
    ),
    # pub a fault to pv_03 and make sure solar_03 picks it up
    Steps(
        [
            # setup (of which there are none)
            {
            },
            # pubs
            {
                "/components/pv_03": {"faults": [{"string": "Something alarming is happening", "value": 1}]}
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_03/faults", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 4),
        ]
    ),
    # clear the fault
    Steps(
        [
            {
                "/site/operation/clear_faults_flag": True
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_03/faults", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 5),
        ]
    ),
    # pub a fault to pv_04 and make sure solar_04 picks it up
    Steps(
        [
            # setup (of which there are none)
            {
            },
            # pubs
            {
                "/components/pv_04": {"faults": [{"string": "Something alarming is happening", "value": 1}]}
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_04/faults", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 4),
        ]
    ),
    # clear the faults
    Steps(
        [
            {
                "/site/operation/clear_faults_flag": True
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_04/faults", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 5),
        ]
    ),
    # pub a fault to pv_05 and make sure solar_05 picks it up
    Steps(
        [
            # setup (of which there are none)
            {
            },
            # pubs
            {
                "/components/pv_05": {"faults": [{"string": "Something alarming is happening", "value": 1}]}
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_05/faults", 2),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 4),
        ]
    ),
    # clear the faults
    Steps(
        [
            {
                "/site/operation/clear_faults_flag": True
            }
        ],
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_05/faults", 0),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/summary/num_solar_available", 5),
        ]
    ),
])
def test_traditional(test):
    return test

