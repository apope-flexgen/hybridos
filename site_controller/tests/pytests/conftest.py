from .pytest_report import setup_report_dir
from .pytest_framework import Site_Controller_Instance

'''
conftest.py

This file is used by pytest to specify hooks to be run.
'''
def pytest_configure(config):
    '''
    "Run before all tests" hook https://docs.pytest.org/en/stable/reference/reference.html#pytest.hookspec.pytest_configure
`   '''
    print("\n-- Running pytest_configure")
    setup_report_dir()

def pytest_sessionfinish(session, exitstatus):
    '''
    "Run at the end of all tests" hook https://docs.pytest.org/en/stable/reference/reference.html#pytest.hookspec.pytest_sessionfinish
`   '''
    print("-- Running pytest_sessionfinish")
    sc = Site_Controller_Instance.get_instance()
    sc.teardown()
