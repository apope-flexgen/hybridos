'''
CloudSync transfer pytests
'''

import requests
import time
import subprocess
import enum
import pytest

ARCHIVE_PERIOD_SEC = 12
CONSUMER_DELAY_SEC = 1

class Container(enum.Enum):
    CLIENT = 1
    ROUTER = 2
    SERVER = 3

def get_archive_count(container: Container):
    '''Get the archive count from the given container'''
    url = ''
    if container == Container.CLIENT:
        url = 'http://localhost:8081'
    elif container == Container.ROUTER:
        url = 'http://localhost:8082'
    elif container == Container.SERVER:
        url = 'http://localhost:8083'
    else:
        raise(Exception("Invalid container argument"))
    url += '/archiveCount'
    response = requests.get(url).content
    return int(response)

@pytest.fixture(autouse=True)
def change_test_dir(request, monkeypatch):
    '''
    pytest fixture which sets the working directory to the directory containing this python file. See the following link:
    https://stackoverflow.com/questions/62044541/change-pytest-working-directory-to-test-case-directory
    '''
    monkeypatch.chdir(request.fspath.dirname)


def test_normal_client_local_transfers():
    '''Test that in normal conditions, client transfers files locally'''
    old_archive_count = get_archive_count(Container.CLIENT)
    time.sleep(ARCHIVE_PERIOD_SEC)
    new_archive_count = get_archive_count(Container.CLIENT)
    assert(new_archive_count > old_archive_count)

def test_normal_router_transfers():
    '''Test that in normal conditions, router gets transfers'''
    old_archive_count = get_archive_count(Container.ROUTER)
    time.sleep(ARCHIVE_PERIOD_SEC)
    new_archive_count = get_archive_count(Container.ROUTER)
    assert(new_archive_count > old_archive_count)

def test_disconnected_client_local_transfers():
    '''Test that while disconnected, client transfers files locally'''
    subprocess.run('../test_scripts/disconnect_router.sh', shell=True) # Note: must disconnect destination server instead because otherwise consumer http port is unreachable
    time.sleep(CONSUMER_DELAY_SEC)
    old_archive_count = get_archive_count(Container.CLIENT)
    time.sleep(ARCHIVE_PERIOD_SEC)
    new_archive_count = get_archive_count(Container.CLIENT)
    subprocess.run('../test_scripts/reconnect_router.sh', shell=True)
    time.sleep(CONSUMER_DELAY_SEC)
    assert(new_archive_count > old_archive_count)

def test_disconnected_startup_client_local_transfers():
    '''Test that when disconnected at cloud_sync startup, client still transfers files locally'''
    subprocess.run('../test_scripts/disconnect_router.sh', shell=True)
    time.sleep(CONSUMER_DELAY_SEC)
    subprocess.run('../test_scripts/kill_client_cloud_sync.sh', shell=True)
    subprocess.run('../test_scripts/run_client_cloud_sync.sh', shell=True)
    old_archive_count = get_archive_count(Container.CLIENT)
    time.sleep(ARCHIVE_PERIOD_SEC)
    new_archive_count = get_archive_count(Container.CLIENT)
    subprocess.run('../test_scripts/reconnect_router.sh', shell=True)
    time.sleep(CONSUMER_DELAY_SEC)
    assert(new_archive_count > old_archive_count)

def test_disconnected_no_router_transfers():
    '''Sanity check that router does not get archives when it is disconnected'''
    subprocess.run('../test_scripts/disconnect_client.sh', shell=True)
    time.sleep(CONSUMER_DELAY_SEC)
    old_archive_count = get_archive_count(Container.ROUTER)
    time.sleep(ARCHIVE_PERIOD_SEC)
    new_archive_count = get_archive_count(Container.ROUTER)
    subprocess.run('../test_scripts/reconnect_client.sh', shell=True)
    time.sleep(CONSUMER_DELAY_SEC)
    assert(new_archive_count == old_archive_count)

# TODO: uncomment this pytest once cloud_sync is changed to do retries immediately after reconnect
# def test_reconnect_router_immediate_transfers():
#     '''Test that after a reconnect, router gets transfers immediately'''
#     subprocess.run('../test_scripts/disconnect_client.sh', shell=True)
#     time.sleep(CONSUMER_DELAY_SEC)
#     old_archive_count = get_archive_count(Container.ROUTER)
#     time.sleep(ARCHIVE_PERIOD_SEC)
#     subprocess.run('../test_scripts/reconnect_client.sh', shell=True)
#     time.sleep(CONSUMER_DELAY_SEC)
#     new_archive_count = get_archive_count(Container.ROUTER)
#     assert(new_archive_count > old_archive_count)
