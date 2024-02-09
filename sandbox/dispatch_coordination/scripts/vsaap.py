#!/bin/python3

from distutils.command.config import config
import os
import shutil
import sys
from subprocess import DEVNULL, run
import time

cfg_path = "/usr/local/etc/config/"
vsaap_network = "vsaap_network"

class Run_Cmd:
    def __init__(self, name, config, ip, ports, image, detached):
        self.name = name
        self.volume_mounts = [ config+':/home/config' ]
        self.ip = ip
        self.ports = ports
        self.image = image
        if detached:
            self.detached = 'd'
        else:
            self.detached = ''
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
        run(self.cmd, stdout=DEVNULL)

    def print_cmd(self):
        print(' '.join(self.cmd))


class Psm:
    def __init__(self, site_num, version_num, image_suffix, base_cfg=cfg_path, site_name="", detached=True):
        self.name = f"{site_name}_psm"
        self.config = os.path.join(base_cfg, "psm/config")
        self.ip = '172.3.27.{o:d}'.format(o=200+site_num)
        self.ports = []
        self.image = f'flexgen/psm{image_suffix}:{version_num}'
        self.run_cmd = Run_Cmd(self.name, self.config, self.ip, self.ports, self.image, detached)

    def launch(self):
        self.run_cmd.print_cmd()
        self.run_cmd.execute()


class Site:
    def __init__(self, site_num, version_num, image_suffix, base_cfg=cfg_path, site_name="", detached=True):
        self.name = f"{site_name}_site"
        self.config = os.path.join(base_cfg, "site-controller/config")
        self.ip = '172.3.27.{o:d}'.format(o=100+site_num)
        self.ports = ['{ip:d}:443'.format(ip=10000+site_num)]
        self.image = f'flexgen/site_controller{image_suffix}:{version_num}'
        self.run_cmd = Run_Cmd(self.name, self.config, self.ip, self.ports, self.image, detached)

    def launch(self):
        self.run_cmd.print_cmd()
        self.run_cmd.execute()


class Vsaap:
    def __init__(self, site_num, image_tag, image_suffix, site_detached=True, base_cfg=cfg_path, site_name=""):
        self.psm = Psm(site_num, image_tag, image_suffix, base_cfg, site_name)
        self.site = Site(site_num, image_tag, image_suffix, base_cfg, site_name, site_detached)

    def launch(self):
        self.psm.launch()
        run(['cp', '-rf', os.path.join(self.site.config, 'web_server/permissions_copy.json'), os.path.join(self.site.config, 'web_server/permissions.json')])
        self.site.launch()


def launch_sites(num_sites, base_cfg, image_tag, image_suffix, site_detached=True):
    run(['sudo', 'docker', 'network', 'create', '--subnet=172.3.27.0/24', vsaap_network], stdout=DEVNULL)
    # TODO: this is a quick workaround that reads any directory not called "fleet-manager" as a separate site
    # decide whether to switch entirely to docker compose, or cleanup this approach as needed
    site_names = []
    for entry in os.scandir(base_cfg):
        if entry.is_dir() and entry != "fleet-manager":
            site_names.append(entry.name)
    for i in range(1, num_sites+1):
        site_cfg_path = os.path.join(base_cfg, site_names[i-1])
        site_controller = Vsaap(i, image_tag, image_suffix, site_detached, site_cfg_path, site_names[i-1])
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
