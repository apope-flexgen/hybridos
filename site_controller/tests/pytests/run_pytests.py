#!/bin/python3

from subprocess import run
import time

import pytest
from test_framework import launch_sites
import os
import sys


bin_dir = "/usr/local/bin"
config_dir = "/usr/local/etc/config"

run("/home/vagrant/git/scripts/hybridos_stop.sh", shell=True)

num_sites = 6
# Optionally provided the number of sites to run with -n flag e.g. -n 3
if "-n" in sys.argv:
    num_sites = int(sys.argv[sys.argv.index("-n") + 1])
print("Starting {n:d} Site Controller test container(s).".format(n=num_sites))
launch_sites(num_sites, "pytest", "10.2.0", test_mode=True)

# Start pytest
args = ["pytest", "-s", "test_site_manager.py"]
# TODO 3rd argument filters the tests collected
# Some way to set the working directory to filter instead?
args.extend(sys.argv[1:])
args.extend([
    "--dist=load",
    "--tx", "socket=172.3.27.101:8888",             # Container IPs
    "--tx", "socket=172.3.27.102:8888",
    "--tx", "socket=172.3.27.103:8888",
    "--tx", "socket=172.3.27.104:8888",
    "--tx", "socket=172.3.27.105:8888",
    "--tx", "socket=172.3.27.106:8888",
    "--rsyncdir", ".",                              # Environments must be in sync for
    "--rsyncignore", "__pycache__",                 # pytests to run successfully
    "--rsyncignore", ".pytest_cache",               # Any one-off directories that
    "--rsyncdir", "/home/vagrant/git/fims/pyfims",  # do not need to be volume mounted
    "--rsyncdir", "/home/vagrant/git/scripts"       # are listed here. TODO: cleanup?
])
pytest.main(args)
