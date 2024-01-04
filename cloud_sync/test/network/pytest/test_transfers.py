'''
CloudSync transfer pytests
'''


import requests
import time
import subprocess
import enum
import pytest


ARCHIVE_PERIOD_SEC = 24
'Period with which archives are created'

ARCHIVES_PER_PERIOD = 5
'Number of archives created per archive period'

CONSUMER_DELAY_SEC = 0.5
'Upper bound on archive processing delay caused by archive consumer polling (equal to consumer polling period)'

RECONNECT_ATTEMPT_PERIOD_SEC = 4
'Period with which reconnects are attempted when connection is bad'

CONNECTION_START_DELAY_SEC = 1
'Upper bound on time it takes for a container to establish a connection'

TRANSFER_DELAY_SEC = 1
'Upper bound on time it takes to transfer a period worth of archives'


class Container(enum.Enum):
    CLIENT = 1
    ROUTER = 2
    SERVER = 3


def get_archive_count(container: Container):
    '''Get the archive count from the given container'''
    time.sleep(CONSUMER_DELAY_SEC) # Wait some time for the consumer to process any currently received archives
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


def disconnect_via_latency(container: Container):
    '''Disconnect the given container via increasing latency (Note: only works for client and router)'''
    script = '../test_scripts/'
    if container == Container.CLIENT:
        script += 'change_client_latency.sh'
    elif container == Container.ROUTER:
        script += 'change_router_latency.sh'
    else:
        raise(Exception("Invalid container argument"))
    result = subprocess.run([script, '5s'])
    print(result)
    return result


def reset_latency(container: Container):
    '''Resets the latency on the given container (Note: only works for client and router)'''
    normal_latency = '10ms'
    script = '../test_scripts/'
    if container == Container.CLIENT:
        script += 'change_client_latency.sh'
    elif container == Container.ROUTER:
        script += 'change_router_latency.sh'
    else:
        raise(Exception("Invalid container argument"))
    result = subprocess.run([script, normal_latency])
    print(result)
    return result


# General notes on test functions:
# Because disconnecting and reconnecting from the docker network resets latency and causes other weird behavior,
# instead simulate disconnects by dramatically slowing the connection down.
# Because the archive consumer http port is being accessed via network, it is also impacted by network changes.
# So, do not disconnect the container whose consumer you want to check, instead disconnect its destination.


def test_normal_client_local_transfers():
    '''Test that in normal conditions, client transfers files locally'''
    old_archive_count = get_archive_count(Container.CLIENT)
    time.sleep(ARCHIVE_PERIOD_SEC)
    new_archive_count = get_archive_count(Container.CLIENT)
    assert(new_archive_count >= old_archive_count + ARCHIVES_PER_PERIOD)


def test_normal_router_transfers():
    '''Test that in normal conditions, router gets transfers'''
    old_archive_count = get_archive_count(Container.ROUTER)
    time.sleep(ARCHIVE_PERIOD_SEC)
    new_archive_count = get_archive_count(Container.ROUTER)
    assert(new_archive_count >= old_archive_count + ARCHIVES_PER_PERIOD)


def test_disconnected_client_local_transfers():
    '''Test that while disconnected, client transfers files locally'''
    disconnect_via_latency(Container.ROUTER)
    old_archive_count = get_archive_count(Container.CLIENT)
    time.sleep(ARCHIVE_PERIOD_SEC)
    new_archive_count = get_archive_count(Container.CLIENT)
    reset_latency(Container.ROUTER)
    assert(new_archive_count >= old_archive_count + ARCHIVES_PER_PERIOD)


def test_disconnected_startup_client_local_transfers():
    '''Test that when disconnected at cloud_sync startup, client still transfers files locally'''
    disconnect_via_latency(Container.ROUTER)
    subprocess.run('../test_scripts/kill_client_cloud_sync.sh')
    subprocess.run('../test_scripts/run_client_cloud_sync.sh')
    old_archive_count = get_archive_count(Container.CLIENT)
    time.sleep(ARCHIVE_PERIOD_SEC)
    new_archive_count = get_archive_count(Container.CLIENT)
    reset_latency(Container.ROUTER)
    assert(new_archive_count >= old_archive_count + ARCHIVES_PER_PERIOD)


def test_disconnected_no_router_transfers():
    '''Sanity check that router does not get archives when it is disconnected'''
    disconnect_via_latency(Container.CLIENT)
    old_archive_count = get_archive_count(Container.ROUTER)
    time.sleep(ARCHIVE_PERIOD_SEC)
    new_archive_count = get_archive_count(Container.ROUTER)
    reset_latency(Container.CLIENT)
    assert(new_archive_count == old_archive_count)


def test_reconnect_router_immediate_transfers():
    '''Test that after a reconnect, router gets transfers immediately'''
    disconnect_via_latency(Container.CLIENT)
    old_archive_count = get_archive_count(Container.ROUTER)
    time.sleep(ARCHIVE_PERIOD_SEC)
    reset_latency(Container.CLIENT)
    time.sleep(RECONNECT_ATTEMPT_PERIOD_SEC)
    time.sleep(CONNECTION_START_DELAY_SEC)
    new_archive_count = get_archive_count(Container.ROUTER)
    assert(new_archive_count > old_archive_count)


def test_reconnected_startup_router_immediate_transfers():
    '''Test that client retries sending previously failed files to router immediately at startup'''
    disconnect_via_latency(Container.CLIENT)
    old_archive_count = get_archive_count(Container.ROUTER)
    time.sleep(ARCHIVE_PERIOD_SEC)
    subprocess.run('../test_scripts/kill_client_cloud_sync.sh')
    reset_latency(Container.CLIENT)
    subprocess.run('../test_scripts/run_client_cloud_sync.sh')
    time.sleep(CONNECTION_START_DELAY_SEC)
    new_archive_count = get_archive_count(Container.ROUTER)
    assert(new_archive_count > old_archive_count)
