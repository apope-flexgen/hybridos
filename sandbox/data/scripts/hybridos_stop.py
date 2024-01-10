#!/bin/python3

import os
import time
from termcolor import cprint
import subprocess
from subprocess import run


memcheck_log='~/git/hybridos/site_controller/valgrind.log'


def rm_docker_containers(keyword):
    '''
    Removes docker containers whose names contain the given keyword
    '''
    docker_ps_output = run(['docker', 'ps', '-a'], stdout=subprocess.PIPE, universal_newlines=True).stdout
    # output is a table of attributes for running containers, where the first row is column labels
    # and columns 0 and 1 are ids and names respectively (it is assumed that the name has no whitespace)
    attributes = [line.split() for line in docker_ps_output.split('\n')[1:]]
    ids = [ row[0] for row in attributes if len(row) >= 2 and row[1].find(keyword) != -1 ]
    if len(ids) > 0:
        print(f'Stopping containers matching {keyword}: {ids}')
        run(['docker', 'container', 'rm', '-f'] + ids, stdout=subprocess.DEVNULL)
    else:
        print(f'No containers matching {keyword}')


def has_detected_memory_leak():
    '''
    Checks the valgrind log for a detected memory leak and returns true if a memory leak was detected
    '''
    log_file_path = os.path.expanduser(memcheck_log)

    if os.path.exists(log_file_path):
        log_file_stats = os.stat(log_file_path)
        seconds_since_last_mod = time.time() - log_file_stats.st_mtime
        
        if seconds_since_last_mod < 6:
            log_file = open(log_file_path, 'r')
            for line in log_file.readlines():

                if 'definitely lost' in line:
                    return True
    return False


def pkill(keyword: str, sudo=False):
    '''
    Kills processes using the linux pkill command.
    '''
    if sudo:
        run(['sudo', 'pkill', keyword, '-e'])
    else:
        run(['pkill', keyword, '-e'])


def stop_hybridos():
    print('##### HYBRIDOS STOP #####')

    print('Stopping Docker containers')
    rm_docker_containers('site')
    rm_docker_containers('twins')
    run(['docker', 'network', 'prune', '--force'])
    print('Stopped all Docker containers')

    # stop all services
    pkill('MCP', sudo=True)

    pkill('cops', sudo=True)

    pkill('fleet_manager', sudo=True)
    pkill('dnp3', sudo=True)
    time.sleep(1) # no clue why, but this `sleep 1` makes the `sudo pkill dnp3` succeed
    pkill('washer', sudo=True)
    pkill('dts', sudo=True)
    pkill('ftd', sudo=True)
    pkill('cloud_sync', sudo=True)

    pkill('modbus_client')
    pkill('site_controller')

    pkill('metrics')
    pkill('events')
    pkill('dbi')
    pkill('scheduler')

    pkill('influxd', sudo=True)
    pkill('mongod', sudo=True)

    pkill('modbus_server', sudo=True)
    pkill('web_server', sudo=True)

    pkill('fims')

    # check for memory leak logged by valgrind
    time.sleep(1)
    if has_detected_memory_leak():
        cprint('\n$### Memory Leak detected ###$\n', color='red')


if __name__ == "__main__":
    stop_hybridos()