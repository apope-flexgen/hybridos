# race condition tests which depend on the timing of certain processes:
# set/get a document immediately upon dbi startup
# set/get a document immediately upon mongod and dbi startup
# set/get a document immediately upon mongod restart

import pytest
import subprocess
from conftest import *
import time

# The time allowed for DBI to startup before we expect it to be responsive
# Adjust this value to make the DBI startup tests more or less strict
dbi_startup_leniency_seconds = 0.1

# The time allowed for Mongod to startup before we expect it to be responsive
# Adjust this value to make the Mongod startup tests more or less strict
mongod_startup_leniency_seconds = 0.1


# Test that DBI requests are handled as expected when sent immediately after DBI startup
class TestReqsAtStartup():

    @pytest.mark.parametrize('count', range(20))
    @pytest.mark.parametrize('proc', ['dbi', 'mongod and dbi', 'mongod'])
    def test_get(self, proc, count):
        uri = "/dbi/test_col/test_doc"
        body = '{"value": "Hi"}'
        set_response = send("set", uri, body, False)
        self.restart_proc(proc)
        get_response = send("get", uri)

        assert "not found" not in get_response
        assert "" != get_response
        assert get_response == set_response
        pass

    @pytest.mark.parametrize('count', range(20))
    @pytest.mark.parametrize('proc', ['dbi', 'mongod and dbi', 'mongod'])
    def test_set(self, proc, count):
        uri = "/dbi/test_col/test_doc"
        body = '{"value": "Hi"}'
        self.restart_proc(proc)
        set_response = send("set", uri, body, False)
        get_response = send("get", uri)

        assert "not found" not in get_response
        assert "" != get_response
        assert get_response == set_response
        pass

    # Restarts processes as specified by the proc string
    def restart_proc(self, proc):
        if proc == 'dbi':
            subprocess.run(['pkill', 'dbi'])
            subprocess.Popen(['dbi', '--c', 'test_dbi_config.json'])
            time.sleep(dbi_startup_leniency_seconds)
        elif proc == 'mongod and dbi':
            subprocess.run(['pkill', 'dbi'])
            subprocess.run(['pkill', 'mongod'])
            subprocess.Popen(['sudo', 'mongod', '--config', '/etc/mongod.conf'])
            subprocess.Popen(['dbi', '--c', 'test_dbi_config.json'])
            time.sleep(max(mongod_startup_leniency_seconds, dbi_startup_leniency_seconds))
        elif proc == 'mongod':
            subprocess.run(['pkill', 'mongod'])
            subprocess.Popen(['sudo', 'mongod', '--config', '/etc/mongod.conf'])
            time.sleep(mongod_startup_leniency_seconds)
