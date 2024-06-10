'''
DTS archive removal pytests
'''


import time
import subprocess
import pytest


DTS_CONTAINER_NAME = "docker-dts_test_dts-1"
'Name of container DTS is running in'

DTS_INPUT_DIR = "/home/data/"
'Directory in the DTS container where DTS gets its input'

TMP_DIR = "/tmp/pytest/"
'Directory in the DTS container used for temporary files'

ARCHIVE_PERIOD_SECONDS = 10
'Period with which archives are created'

ARCHIVE_PROCESSING_DELAY_SECONDS = 0.1
'Time to wait after an archive is created before expecting it to be done processing'


def run_in_dts_container(cmd, args):
    '''
    Runs the given command in the DTS container and returns the result
    '''
    return subprocess.run(['docker', 'exec', DTS_CONTAINER_NAME, cmd] + args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)


def check_files_removed_from_dts_input():
    '''
    Returns true if and only if we successfully determine that there are no files in the dts input dir
    '''
    # check if there are any archives in the input dir
    result = run_in_dts_container('ls', [DTS_INPUT_DIR])
    print(result)
    if result.returncode == 0 and len(result.stdout) == 0:
        return True
    # if there are, wait long enough for them to be processed and check again
    time.sleep(ARCHIVE_PROCESSING_DELAY_SECONDS)
    result = run_in_dts_container('ls', [DTS_INPUT_DIR])
    print(result)
    return result.returncode == 0 and len(result.stdout) == 0


@pytest.fixture
def clear_tmp_dir():
    '''
    Clears the tmp dir in the dts container
    '''
    run_in_dts_container('rm', [TMP_DIR + '*'])
    run_in_dts_container('mkdir', [TMP_DIR])


def test_successful_archives_removed():
    '''
    Test that archives are removed during normal operation
    '''
    time.sleep(ARCHIVE_PERIOD_SECONDS)
    assert(check_files_removed_from_dts_input())


def test_empty_archives_removed():
    '''
    Test that empty archives are removed
    '''
    run_in_dts_container('touch', [DTS_INPUT_DIR + 'empty.tar.gz'])
    assert(check_files_removed_from_dts_input())
    run_in_dts_container('touch', [DTS_INPUT_DIR + 'empty.batchpqtgz'])
    assert(check_files_removed_from_dts_input())


def test_batched_archive_empty_datafiles_removed(clear_tmp_dir):
    '''
    Test that a batched archive of empty datafiles is removed
    '''
    run_in_dts_container('touch', [TMP_DIR + 'empty.parquet.gz'])
    run_in_dts_container('tar', ['-czvf', DTS_INPUT_DIR + 'empty_datafiles.batchpqtgz', TMP_DIR + 'empty.parquet.gz'])
    assert(check_files_removed_from_dts_input())


def test_archives_write_type_conflict_removed():
    '''
    Test that archives which are rejected due to a type conflict are removed
    '''
    # Cause a type conflict by sending a value which gets written to a previously written value
    time.sleep(ARCHIVE_PERIOD_SECONDS)
    run_in_dts_container('fims_send', ['-m', 'pub', '-u', '/test_string', '{"random_walk":"test string"}'])
    time.sleep(ARCHIVE_PERIOD_SECONDS)
    assert(check_files_removed_from_dts_input())