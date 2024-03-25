'''
FTD archive pytests
'''


import pytest
import os
import subprocess
import time


OUTPUT_PATH_1 = "/home/hybridos/ftd_pytest/cold"
OUTPUT_PATH_2 = "/home/hybridos/ftd_pytest/warm"

ARCHIVE_PERIOD_SECONDS = 1
ARCHIVE_CREATION_DELAY_SECONDS = 0.5

TEST_MESSAGE_URI = "/ftd/test"
TEST_MESSAGE_BODY = '{"value": 1}'


@pytest.fixture
def clear_directories():
    '''Clears test directories before and after test functions'''
    dirs = [OUTPUT_PATH_1, OUTPUT_PATH_2]
    for dir in dirs:
        for file_name in os.listdir(dir):
            file_path = os.path.join(dir, file_name)
            subprocess.run(["sudo", "rm", file_path])
    yield
    for dir in dirs:
        for file_name in os.listdir(dir):
            file_path = os.path.join(dir, file_name)
            subprocess.run(["sudo", "rm", file_path])


def test_archives_created_after_pub(clear_directories):
    '''Tests that archives are created after a message is published'''
    subprocess.run(["fims_send", "-m", "pub", "-u", TEST_MESSAGE_URI, TEST_MESSAGE_BODY])
    time.sleep(ARCHIVE_PERIOD_SECONDS)
    time.sleep(ARCHIVE_CREATION_DELAY_SECONDS)
    assert len(os.listdir(OUTPUT_PATH_1)) == 1
    assert len(os.listdir(OUTPUT_PATH_2)) == 1

def test_archives_created_after_set(clear_directories):
    '''Tests that archives are created after a set message is sent'''
    subprocess.run(["fims_send", "-m", "set", "-u", TEST_MESSAGE_URI, TEST_MESSAGE_BODY])
    time.sleep(ARCHIVE_PERIOD_SECONDS)
    time.sleep(ARCHIVE_CREATION_DELAY_SECONDS)
    assert len(os.listdir(OUTPUT_PATH_1)) == 1
    assert len(os.listdir(OUTPUT_PATH_2)) == 1

def test_archives_not_created_after_inactivity(clear_directories):
    '''Tests that archives are not created after a period of inactivity'''
    time.sleep(ARCHIVE_PERIOD_SECONDS)
    time.sleep(ARCHIVE_CREATION_DELAY_SECONDS)
    assert len(os.listdir(OUTPUT_PATH_1)) == 0
    assert len(os.listdir(OUTPUT_PATH_2)) == 0
