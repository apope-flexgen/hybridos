'''
Archive Cocoon convert pytests
'''


import pytest
import pytest_cases
import os
import shutil
import subprocess
import time
import pandas
import pyarrow # pyarrow is indirectly used by pandas parquet reader, imported so it appears in requirements.txt


INPUT_PATH = "/home/hybridos/archive_cocoon_pytest/input"
OUTPUT_PATH = "/home/hybridos/archive_cocoon_pytest/output"
FORWARD_PATH = "/home/hybridos/archive_cocoon_pytest/forward"
FAILURE_PATH = "/home/hybridos/archive_cocoon_pytest/failed_convert"
SAMPLE_VALID_ARCHIVES_DIR_PATH = os.path.join(os.path.dirname(__file__), "../sample_archives/valid")
SAMPLE_INVALID_ARCHIVES_DIR_PATH = os.path.join(os.path.dirname(__file__), "../sample_archives/invalid")

CONVERSION_MAXIMUM_DELAY_SECONDS = 0.3


@pytest.fixture
def clear_directories():
    '''Clears test directories before and after test functions'''
    dirs = [INPUT_PATH, OUTPUT_PATH, FORWARD_PATH, FAILURE_PATH]
    for dir in dirs:
        for file_name in os.listdir(dir):
            file_path = os.path.join(dir, file_name)
            subprocess.run(["sudo", "rm", file_path])
    yield
    for dir in dirs:
        for file_name in os.listdir(dir):
            file_path = os.path.join(dir, file_name)
            subprocess.run(["sudo", "rm", file_path])


@pytest_cases.parametrize("sample_archive_name", os.listdir(SAMPLE_VALID_ARCHIVES_DIR_PATH))
def test_convert_sample_valid_archive(clear_directories, sample_archive_name):
    '''Test that sample valid archives are successfully converted'''
    sample_archive_path = os.path.join(SAMPLE_VALID_ARCHIVES_DIR_PATH, sample_archive_name)
    shutil.copy(sample_archive_path, INPUT_PATH)
    time.sleep(CONVERSION_MAXIMUM_DELAY_SECONDS)

    inputs = os.listdir(INPUT_PATH)
    outputs = os.listdir(OUTPUT_PATH)
    forwards = os.listdir(FORWARD_PATH)
    failures = os.listdir(FAILURE_PATH)
    assert(len(inputs) == 0)
    assert(len(outputs) == 1)
    assert(len(forwards) == 1)
    assert(len(failures) == 0)

    # check that file is valid parquet, an error will be thrown otherwise
    output_parquet_path = os.path.join(OUTPUT_PATH, outputs[0])
    pandas.read_parquet(output_parquet_path, engine='pyarrow')

@pytest_cases.parametrize("sample_archive_name", os.listdir(SAMPLE_INVALID_ARCHIVES_DIR_PATH))
def test_convert_sample_invalid_archive(clear_directories, sample_archive_name):
    '''Test that sample invalid archives are properly handled'''
    sample_archive_path = os.path.join(SAMPLE_INVALID_ARCHIVES_DIR_PATH, sample_archive_name)
    shutil.copy(sample_archive_path, INPUT_PATH)
    time.sleep(CONVERSION_MAXIMUM_DELAY_SECONDS)

    inputs = os.listdir(INPUT_PATH)
    outputs = os.listdir(OUTPUT_PATH)
    forwards = os.listdir(FORWARD_PATH)
    failures = os.listdir(FAILURE_PATH)
    assert(len(inputs) == 0)
    assert(len(outputs) == 0)
    assert(len(forwards) == 0)
    assert(len(failures) == 1)