#!/bin/python3

import os
import shutil
import sys
from subprocess import DEVNULL, run
import time

cfg_path = "/usr/local/etc/config/"
test_cfg_path = os.path.expanduser("~") + "/test_config/"
vsaap_network = "vsaap_network"
initial_test_delay = 5
additional_container_delay = 0.25


class Run_Cmd:
    def __init__(self, name, config, ip, ports, image, detached, delay=0):
        self.name = name
        self.volume_mounts = [config+':/home/config/', config+':/usr/local/etc/config/', '/usr/local/bin/:/home/bin']
        self.ip = ip
        self.ports = ports
        self.image = image
        if detached:
            self.detached = 'd'
        else:
            self.detached = ''
        # Prepend any custom commands received from the caller
        self.delay = delay
        self.update_cmd()

    def update_cmd(self):
        self.cmd = ['sudo', 'docker', 'run', '-' + self.detached + 'it', '--privileged']
        self.cmd.extend(['--name', self.name])
        for mount in self.volume_mounts:
            self.cmd.extend(['-v', mount])
        self.cmd.extend(['--net', vsaap_network, '--ip', self.ip])
        for port in self.ports:
            self.cmd.extend(['-p', port])
        self.cmd.append(self.image)

    def execute(self):
        # Wait for site container pytests to modify twins config before starting twins container
        if self.delay > 0:
            pid = os.fork()
            if pid == 0:
                time.sleep(self.delay)
                run(self.cmd, stdout=DEVNULL)
                sys.exit()
        else:
            run(self.cmd, stdout=DEVNULL)

    def print_cmd(self):
        print(' '.join(self.cmd))


class Twins:
    def __init__(self, site_num, version_num, image_suffix, test_mode, detached=True, delay=0):
        self.name = 'twins_{i:d}'.format(i=site_num)
        self.config = test_cfg_path if test_mode else cfg_path  # Point config at shared volume if test
        self.config += "test_sites/site_{i:d}/twins/".format(i=site_num)
        self.ip = '172.3.27.{o:d}'.format(o=200+site_num)
        self.ports = []
        self.image = f'flexgen/twins{image_suffix}:{version_num}'
        self.run_cmd = Run_Cmd(self.name, self.config, self.ip, self.ports, self.image, detached, delay=delay)

    def launch(self):
        self.run_cmd.print_cmd()
        self.run_cmd.execute()


class Site:
    def __init__(self, site_num, version_num, image_suffix, test_mode, detached=True):
        self.name = 'site_{i:d}'.format(i=site_num)
        self.config = test_cfg_path if test_mode else cfg_path  # Point config at shared volume if test
        self.config += 'test_sites/site_{i:d}/'.format(i=site_num)
        self.ip = '172.3.27.{o:d}'.format(o=100+site_num)
        self.ports = []
        if not test_mode:
            self.ports = ['{ip:d}:443'.format(ip=10000+site_num)]
        self.image = f'flexgen/site_controller{image_suffix}:{version_num}'
        self.run_cmd = Run_Cmd(self.name, self.config, self.ip, self.ports, self.image, detached)

    def launch(self):
        self.run_cmd.print_cmd()
        self.run_cmd.execute()


class Vsaap:
    def __init__(self, site_num, image_tag, image_suffix, test_mode, site_detached=True):
        self.twins = Twins(site_num, image_tag, image_suffix, test_mode, delay=initial_test_delay+additional_container_delay*site_num) if test_mode\
            else Twins(site_num, image_tag, image_suffix, test_mode)
        self.site = Site(site_num, image_tag, image_suffix, test_mode, site_detached)
        self.test_mode = test_mode

    def launch(self):
        self.twins.launch()
        if not self.test_mode:
            run(['cp', '-rf', self.site.config+'web_server/permissions_copy.json', self.site.config+'web_server/permissions.json'])
        self.site.launch()


def launch_sites(num_sites, image_tag, image_suffix, test_mode=False, site_detached=True):
    run(['sudo', 'docker', 'network', 'create', '--subnet=172.3.27.0/24', vsaap_network], stdout=DEVNULL)
    # Test mode needs to share data between the containers, but not outside them
    if test_mode:
        # Create a test directory with config to be removed on termination
        shutil.copytree(cfg_path, test_cfg_path)
    for i in range(1, num_sites+1):
        site_controller = Vsaap(i, image_tag, image_suffix, test_mode, site_detached)
        site_controller.launch()


# define function for reading a repo's tag from repo.txt
# TO DO: put this in a utils file so the same function can be called from both
# vsaap.py and hybridos_run.py
def read_repo_tag(repo_name):
    repo_txt = cfg_path + "/repo.txt"
    repo_txt_file = open(repo_txt, "r")
    repo_txt_contents = repo_txt_file.read()
    repo_txt_file.close()
    # extract repo line from repo.txt
    repo_line = "not found"
    line_search = repo_name
    line_search += "|"
    line_list = repo_txt_contents.split("\n")
    for line in line_list:
        if line_search in line:
            repo_line = line
    if repo_line == "not found":
        print("<<<<< DID NOT FIND " + repo_name + " LINE IN REPO.TXT >>>>>")
        sys.exit()
    # extract tag from repo line
    line_frags = repo_line.split("|")
    if len(line_frags) != 2:
        print("<<<<< FOUND " + repo_name + "LINE IN REPO.TXT BUT AFTER SPLITTING ON | FOUND ", line_frags, " FRAGS NOT 2")
        sys.exit()
    repo_tag = line_frags[1]
    return repo_tag


def main():
    hybridos_image_tag = read_repo_tag("hybridos")[1:].replace("-", ".")
    # Determine if snapshot image based on whether the tag containers any of the following
    if any(e in hybridos_image_tag for e in ["rc", "alpha", "beta"]):
        hybridos_image_suffix = "-snapshot"
    launch_sites(1, hybridos_image_tag, hybridos_image_suffix)


if __name__ == "__main__":
    main()
