#!/bin/python3

# Requires pyyaml:
# sudo pip3 install pyyaml
import yaml

import time
import os
import sys
import grp
import pwd
import getpass
from subprocess import Popen, run, DEVNULL, PIPE
from textwrap import dedent
from typing import Dict
from argparse import ArgumentParser, RawTextHelpFormatter
from vsaap import launch_sites

from hybridos_stop import stop_hybridos
from hybridos_stop import stop_systemd_hybridos


pid = os.getpid()
sleeptime = 1
bin_dir = "/usr/local/bin"
config_usecase_dir = ""
machine_config_dir_symlink = "/usr/local/etc/config"
username = getpass.getuser()
site_product = "site_controller"
fleet_product = "fleet_man"
product = site_product
hybridos_image_tag = ""
hybridos_image_suffix = ""
reset_setpoints = False
launch_powercloud = False
check_memory = False
disable_comms = False
service_to_debug = None
# List of FPS-owned services
hybridos_services = ["fims", "dbi", "metrics", "go_metrics", "events", "scheduler",
                     "site_controller", "modbus_interface", "overwatch",
                     "ftd", "cloud_sync", "dts", "cops", "web_ui", "fleet_manager", "dnp3_interface", "echo"]

fps_services = ["site_controller", "cops", "scheduler", "ftd", "cloud_sync", "dts"]
site_to_machine_to_config_dir = dict()
'''
Nested dictionary of site config dir paths, replicates the structure of the config directory.
Keys are site names and values are dictionaries of machine type to dir path.
'''
site_to_run = ""


def error_exit(msg):
    print(msg, file=sys.stderr)
    stop_hybridos()
    sys.exit()


# Parsing command line arguments
def parse_args():
    args = ArgumentParser(description="HybridOS run script", formatter_class=RawTextHelpFormatter)
    args.add_argument("--ref", type=str, required=False, default=None, help="Specify the name of a reference design config use-case to run")
    args.add_argument("--sandbox", type=str, required=False, default=None, help="Specify the name of a sandbox config use-case to run")
    args.add_argument("--init", action="store_true", help="Clear runtime configurations")
    args.add_argument("--update", action="store_true", help="Update systemctl service files")
    args.add_argument("--systemd", action="store_true", help="Execute enviroment using systemctl instead of MCP.")
    args.add_argument("--powercloud", action="store_true", help="Launch powercloud processes")
    args.add_argument("--memcheck", action="store_true", help="Check site controller for memory leaks")
    args.add_argument("--disable_comms", action="store_true", help="Disable modbus/dnp3 components for testing")
    args.add_argument("--debug", nargs=1, type=str.lower, required=False, default=None, choices=fps_services,
                      help="Disables a specific process so that it can be called\nmanually with the desired tool\n" + dedent('''\
                            Supported processes are:\n\
                            \t- site_controller\n\
                            \t- cops\n\
                            \t- scheduler\n\
                            \t- ftd\n\
                            \t- cloud_sync\n\
                            \t- dts\n\
                        ''')
                      )
    parsed_args = args.parse_args()

    # Extract arguments into variables. This allows us to use arguments that may not be defined without getting an AttributeError
    global reset_setpoints
    reset_setpoints = getattr(parsed_args, "init", False)
    global update_services
    update_services = getattr(parsed_args, "update", False)
    global systemd
    systemd = getattr(parsed_args, "systemd", False)
    global launch_powercloud
    launch_powercloud = getattr(parsed_args, "powercloud", False)
    global check_memory
    check_memory = getattr(parsed_args, "memcheck", False)
    global disable_comms
    disable_comms = getattr(parsed_args, "disable_comms", False)
    # This is a flag followed by a value rather than just the flag e.g. --debug site_controller
    global service_to_debug
    service_to_debug = getattr(parsed_args, "debug", None)
    if service_to_debug is not None:
        service_to_debug = service_to_debug[0]
    # Determine what config use-case to use
    global config_usecase_dir
    ref_arg = getattr(parsed_args, "ref", None)
    sandbox_arg = getattr(parsed_args, "sandbox", None)
    if ref_arg == None and sandbox_arg == None and update_services == None:
        error_exit("You must specify a config use-case to use.")
    elif ref_arg != None and sandbox_arg == None:
        config_usecase_dir = os.path.join(os.path.expanduser("~/git/hybridos/config/"), ref_arg, "config/")
    elif ref_arg == None and sandbox_arg != None:
        config_usecase_dir = os.path.join(os.path.expanduser("~/git/hybridos/sandbox/dispatch_coordination/config/"), sandbox_arg, "config/")
    elif ref_arg != None and sandbox_arg != None:
        error_exit("Specify either a sandbox use-case or a reference design use-case, not both.")


def scan_config_directory():
    '''Scans config directory to find relevant file paths and store them in a navigable structure'''
    global product, site_to_machine_to_config_dir
    for site_entry in os.scandir(config_usecase_dir):
        if site_entry.is_dir() and site_entry.name != 'fleet-manager':
            site_name = site_entry.name
            site_dir = os.path.join(config_usecase_dir, site_name)
            site_to_machine_to_config_dir[site_name] = {}
            for machine_entry in os.scandir(site_dir):
                if machine_entry.is_dir():
                    machine_type = machine_entry.name
                    site_to_machine_to_config_dir[site_name][machine_type] = os.path.join(machine_entry.path, 'config')
        elif site_entry.name == 'fleet-manager':
            product = fleet_product
            site_to_machine_to_config_dir['fleet-manager'] = {}
            site_to_machine_to_config_dir['fleet-manager']['fleet-manager'] = os.path.join(site_entry.path, 'config')

    if product == site_product:
        if len(site_to_machine_to_config_dir) == 1:
            global site_to_run
            site_to_run = next(iter(site_to_machine_to_config_dir))
            site_controller_to_run_config_dir = site_to_machine_to_config_dir[site_to_run]['site-controller']
            symlink_machine_config_dir(site_controller_to_run_config_dir)
        else:
            error_exit(f'There are {len(site_to_machine_to_config_dir)} sites in your chosen config, but exactly 1 is expected.')
    elif product == fleet_product:
        fleet_manager_to_run_config_dir = site_to_machine_to_config_dir['fleet-manager']['fleet-manager']
        symlink_machine_config_dir(fleet_manager_to_run_config_dir)


def symlink_machine_config_dir(src_dir):
    '''
    Creates a symlink between /usr/local/etc/config and the desired machine config dir.
    If a symlink already exists, it gets removed and recreated.
    '''
    if os.path.exists(machine_config_dir_symlink):
        run(['/usr/bin/sudo', 'unlink', machine_config_dir_symlink])
    run(['/usr/bin/sudo', 'ln', '-sf', src_dir, machine_config_dir_symlink])


def check_environment():
    if bin_dir in os.getenv('PATH') == False:
        error_exit("/usr/local/bin not found in PATH environmental variable. Please add it for convenience.")

    if not os.path.isdir(machine_config_dir_symlink):
        print("config directory: ", machine_config_dir_symlink)
        error_exit("config directory not found.")

    print("##### HYBRIDOS RUN #####")
    print("config directory: " + os.readlink(machine_config_dir_symlink))
    print("starting system...")

    check_dir_user_permissions("/var/log/flexgen")

    global hybridos_image_tag
    global hybridos_image_suffix
    if product == site_product:
        hybridos_image_tag = read_override_version(os.path.normpath(os.path.join(site_to_machine_to_config_dir[site_to_run]['psm'], '../psm-01.yml')))
    elif product == fleet_product:
        # for simplicity, assume the same tag is used across everything
        hybridos_image_tag = read_override_version(os.path.normpath(os.path.join(
            site_to_machine_to_config_dir['fleet-manager']['fleet-manager'], '../fleet-manager-01.yml')))
    # Determine if snapshot image based on whether the tag containers any of the following
    if any(e in hybridos_image_tag for e in ["rc", "alpha", "beta"]):
        hybridos_image_suffix = "-snapshot"


# Assign the current user to be the user owner and group owner of the file/folder located at the given path.
def give_user_permissions(path):
    # stat gives a numeric ID for the user and the group that owners the file/folder.
    # convert these to their string equivalents
    st = os.stat(path)
    user_owner_of_path = pwd.getpwuid(st.st_uid)[0]  # convert user owner numeric ID to user name
    group_owner_of_path = grp.getgrgid(st.st_gid)[0]  # convert group owner numeric ID to group name

    # if user/group owner of the file/path do not already match username of current user, update the ownership
    if user_owner_of_path != username:
        run(f"sudo chown -R {username} {path}", shell=True)
    if group_owner_of_path != username:
        run(f"sudo chgrp -R {username} {path}", shell=True)


# Makes sure logging directory exists and permissions will let user log there.
def check_dir_user_permissions(path):
    if not os.path.isdir(path):
        cmd = "sudo mkdir " + path
        run(cmd, shell=True)
    give_user_permissions(path)


def load_dbi(process_name: str, config_src, delete_existing_data=True, file_document_renames: Dict[str, str] = {}) -> None:
    """
    Deletes the process's existing DBI data with a DEL to /dbi/process_name unless delete_existing_data is false.
    Then, identifies any files in the process's configuration directory that end in .json and adds their data to
    DBI with SETs to /dbi/process_name/file_name (one per file).

    Args:
        process_name (str): The name of the process which should match the configuration directory's name and the DBI collection's name.

        config_src (str, optional): filepath source of hybridos configs. Defaults to the config_dir but can be overwritten with directories like config_powercloud

        delete_existing_data (bool, optional): If true, will start by sending a DEL to the collection URI. Defaults to true.

        file_document_renames (Dict[str, str], optional): Indicates what data should be stored under a document with a different name than the file name. Ex: {'CA_configs.json': 'assets'}
    """
    if delete_existing_data:
        print(f"Resetting {process_name} configurations")
        run(['fims_send', '-m', 'del', '-u', f'/dbi/{process_name}', '-r', f'/{pid}'],
            stdout=DEVNULL)  # reply-to used to keep FIMS from getting clogged
    process_config_directory = os.path.join(config_src, process_name)
    file_names = os.listdir(process_config_directory)
    for file_name in file_names:
        file_path = os.path.join(process_config_directory, file_name)
        if os.path.isdir(file_path):
            sub_file_names = os.listdir(file_path)
            for sub_file_name in sub_file_names:
                document_name = file_document_renames[sub_file_name] if sub_file_name in file_document_renames else os.path.splitext(sub_file_name)[0]
                dbi_uri = f'/dbi/{process_name}/{file_name}/{document_name}'
                run(['fims_send', '-m', 'set', '-u', dbi_uri, '-f', os.path.join(file_path, sub_file_name), '-r', f'/{pid}'], stdout=DEVNULL)
        elif not file_name.endswith('.json'):
            continue
        document_name = file_document_renames[file_name] if file_name in file_document_renames else os.path.splitext(file_name)[0]
        dbi_uri = f'/dbi/{process_name}/{document_name}'
        # reply-to used to keep FIMS from getting clogged
        run(['fims_send', '-m', 'set', '-u', dbi_uri, '-f', file_path, '-r', f'/{pid}'], stdout=DEVNULL)


# Check if folder exists, if not make it.
# Check if log file exists, if not make it.
# Make sure permissions allow user access.
def prep_for_logging(path, logfile):
    fullpath = path + "/" + logfile

    # checking folder
    if not os.path.isdir(path):
        os.mkdir(path)
    give_user_permissions(path)

    # checking file beneath folder
    if not os.path.exists(fullpath):
        cmd = "sudo touch " + fullpath
        run(cmd, shell=True)  # running a shell command.
    give_user_permissions(fullpath)


# Start an FPS-owned service
# name: the name of the service
# config_src: config directory path, either to config or config_powercloud
# root: whether elevated privileges are required
def start_fps_service(name: str, config_src, root=False, file_document_renames: Dict[str, str] = {}):
    # If this is the service that is being debugged launch using the provided tool
    if service_to_debug == name:
        print(f"Skipping {name} for debugging")
        return

    # Otherwise try to launch using MCP
    mcp_config = os.path.join(config_src, f"mcp/mcp_{name}.json")
    if not os.path.isfile(mcp_config):
        print(f"Failed to start {name} with mcp config {mcp_config}: missing config or debug parameter", file=sys.stderr)
        return

    # Clear runtime settings based on command line argument
    load_dbi(name, config_src, delete_existing_data=reset_setpoints, file_document_renames=file_document_renames)

    # Setup logging config source
    prep_for_logging(f"/var/log/flexgen/{name}", f"{name}.log")

    print(f"starting {name}")
    executable = os.path.join(bin_dir, "FlexGenMCP")
    # Try to run through memory checking config if specified. This should invoke valgrind to check for memory leaks
    if check_memory:
        memcheck_config = os.path.join(config_src, f"mcp/mcp_{name}_memcheck.json")
        if not os.path.isfile(memcheck_config):
            print(f"Memory checking not available for {name}")
        else:
            mcp_config = memcheck_config
    if systemd:
        # Run via systemd
        Popen(["sudo", "systemctl", "start", name])
    elif root:
        # Run with root privileges
        Popen(["sudo", executable, mcp_config])
    else:
        Popen([executable, mcp_config])


# LEVEL 1: start influxdb, mongodb, fims_server, check for /var/log/flexgen logging access
def start_influx():
    if (launch_powercloud == True):
        print("starting influx")
        redirDevnull = open(os.devnull, 'w')
        if systemd:
            Popen(["sudo", "systemctl", "start", "influxd"], stderr=redirDevnull)
        else:
            Popen(['sudo', 'influxd'], stderr=redirDevnull)


def start_mongo():
    print("starting mongo")
    # stop mongod if it was enabled at startup
    # Popen(["sudo", "systemctl", "stop", "mongod"])
    # reset mongod every time, avoid failed states on VM restarts
    cmd = "sudo chown mongod:mongod /home/mongodb/* -R"
    run(cmd, shell=True)  # running a shell command.
    # Run mongod one time to initialize socket files
   # Popen(["sudo", "systemctl", "start", "mongod"])
    time.sleep(sleeptime)
    # Popen(["sudo", "systemctl", "stop", "mongod"])
    time.sleep(sleeptime)
    if systemd:
        Popen(["sudo", "systemctl", "restart", "mongod"])
    else:
        Popen(["sudo", "systemctl", "restart", "mongod"])
        # Popen(['sudo', 'mongod', '--config', '/etc/mongod.conf'])


def start_fims():
    print("starting fims")
    if systemd:
        Popen(["sudo", "systemctl", "start", "fims"])
    else:
        Popen([bin_dir + '/fims_server'])
    time.sleep(sleeptime)


# LEVEL 2: dbi, events, start metrics, go_metrics, scheduler
def start_dbi():
    if os.path.isdir(machine_config_dir_symlink + "/dbi"):
        print("starting dbi")

        if systemd:
            Popen(["sudo", "systemctl", "start", "dbi"])
        else:
            Popen([bin_dir + '/FlexGenMCP', machine_config_dir_symlink + '/mcp/mcp_dbi.json'])
        time.sleep(sleeptime)


def start_events():
    if os.path.isdir(machine_config_dir_symlink + "/events"):
        print("starting events")
        if reset_setpoints:
            print("Resetting events alerting configurations")
            command = "cd /home/hybridos/git/hybridos/events; npm run clearData"
            # Clear out alerting data from mongo
            Popen(command, shell=True)
            time.sleep(0.1)
        if systemd:
            Popen(["sudo", "systemctl", "start", "events"])
        else:
            Popen([bin_dir + '/FlexGenMCP', machine_config_dir_symlink + '/mcp/mcp_events.json'])


def start_metrics():
    if os.path.isdir(machine_config_dir_symlink + "/metrics"):
        print("starting metrics")
        if systemd:
            Popen(["sudo", "systemctl", "start", "metrics@metrics.json"])
        else:
            Popen([bin_dir + '/FlexGenMCP', machine_config_dir_symlink + '/mcp/mcp_metrics.json'])


def start_go_metrics():
    if os.path.isdir(machine_config_dir_symlink + "/go_metrics"):
        print("starting go_metrics")
        # Load config
        load_dbi("go_metrics", machine_config_dir_symlink, delete_existing_data=reset_setpoints)
        if reset_setpoints:
            # Manually reset alerting config. It doesn't not utilize config files so load_dbi() is not appropriate
            run(['fims_send', '-m', 'del', '-u', f'/dbi/go_metrics_alerting', '-r', f'/{pid}'],
                stdout=DEVNULL)  # reply-to used to keep FIMS from getting clogged
        if systemd:
            Popen(["sudo", "systemctl", "start", "go_metrics@configuration.json"])
        else:
            Popen([bin_dir + '/FlexGenMCP', machine_config_dir_symlink + '/mcp/mcp_go_metrics.json'])
        # demo, start another go_metrics
        Popen([bin_dir + '/FlexGenMCP', machine_config_dir_symlink + '/mcp/mcp_go_metrics_example.json'])


def start_scheduler():
    start_fps_service("scheduler", machine_config_dir_symlink)


# LEVEL 3: start core product processes
def read_override_version(override_file_path):
    '''Reads the version given in a virtual_site override YAML file'''
    with open(override_file_path, "r") as stream:
        contents = yaml.safe_load(stream)
        return contents['version']


# start dnp3 clients
def start_dnp3_clients():
    if product != fleet_product:
        return

    print("Starting DNP3 clients.")
    if disable_comms:
        print("DNP3 client disabled for testing.")
    elif os.path.isdir(machine_config_dir_symlink + "/dnp3_client"):
        Popen([bin_dir + '/FlexGenMCP', machine_config_dir_symlink + '/mcp/mcp_dnp3_client.json'])
        time.sleep(sleeptime)
    else:
        print("No DNP3 client configuration file found.")


# start dnp3 server if applicable
def start_dnp3_server():
    if product != fleet_product:
        return

    print("Starting DNP3 server.")
    if disable_comms:
        print("DNP3 server disabled for testing.")
    elif os.path.isdir(machine_config_dir_symlink + "/dnp3_server"):
        Popen(['sudo', bin_dir + '/FlexGenMCP', machine_config_dir_symlink + '/mcp/mcp_dnp3_server.json'])
    else:
        print("No DNP3 server configuration file found.")


# define function for starting processes specific to the Fleet Manager product
def start_fleet_manager():
    if product != fleet_product:
        return

    # launch site controller
    num_sites = len(site_to_machine_to_config_dir) - 1  # subtract 1 entry used by fleet manager
    print("Starting {n:d} Site Controller container(s).".format(n=num_sites))
    launch_sites(num_sites, config_usecase_dir, hybridos_image_tag, hybridos_image_suffix)
    print("Waiting 15 seconds to give Site Controller container(s) time to boot.")
    time.sleep(sleeptime * 15)
    print("Done starting Site Controller container(s).")

    # TODO: rename fleet manager logging to "fleet_manager" for consistency
    prep_for_logging("/var/log/flexgen/fleet_man", "fleet_man.log")
    # start fleet_manager executable
    config_dir_mcp = machine_config_dir_symlink + "/mcp/mcp_fleet_manager.json"
    if os.path.isfile(config_dir_mcp):
        if reset_setpoints == True:
            print("Resetting fleet_manager configurations")
            load_dbi('fleet_manager', machine_config_dir_symlink)

        print("starting fleet_manager")
        Popen(['sudo', bin_dir + '/FlexGenMCP', config_dir_mcp])
    else:
        error_exit("could not find mcp_fleet_manager.json")


# start modbus clients
def start_modbus_clients():
    if product != site_product:
        return

    print("Starting Modbus clients.")
    if disable_comms:
        print("Modbus clients disabled for testing.")
    elif os.path.isdir(machine_config_dir_symlink + "/modbus_client"):
        if systemd:
            # Launch modbus with systemd with dedicated instance names
            clients = os.listdir(machine_config_dir_symlink + "/modbus_client")
            for client in clients:
                cmd = "sudo systemctl start modbus_client@" + client
                run(cmd, shell=True)  # running a shell command.
        else:
            Popen([bin_dir + '/FlexGenMCP', machine_config_dir_symlink + '/mcp/mcp_modbus_client.json'])
        time.sleep(sleeptime)
    else:
        print("No Modbus client configuration file found.")


# start modbus server if applicable
def start_modbus_server():
    if product != site_product:
        return

    print("Starting Modbus server.")
    if disable_comms:
        print("Modbus server disabled for testing.")
    elif os.path.isdir(machine_config_dir_symlink + "/modbus_server"):
        if systemd:
            # Launch modbus servers with systemd with dedicated instance name
            servers = os.listdir(machine_config_dir_symlink + "/modbus_server")
            for server in servers:
                cmd = "sudo systemctl start modbus_server@" + server
                run(cmd, shell=True)  # running a shell command.
        else:
            Popen(['sudo', bin_dir + '/FlexGenMCP', machine_config_dir_symlink + '/mcp/mcp_modbus_server.json'])
    else:
        print("No Modbus server configuration file found.")


# define function for starting processes specific to Site Controller product
def start_site_controller():
    if product != site_product:
        return

    # start PSM container
    print("starting PSM Docker container")
    run(['sudo', 'docker', 'network', 'create', '--subnet=172.3.27.0/24', 'psm_network'])
    launch_container_command = [
        "sudo", "docker", "run", "--name", "psm", "-dit", "-v", site_to_machine_to_config_dir[site_to_run]['psm'] + ":/home/config", "--net",
        "psm_network", "--ip", "172.3.27.2", "flexgen/psm" + hybridos_image_suffix + ":" + hybridos_image_tag]
    print(' '.join(launch_container_command))
    run(launch_container_command)

    # bootstrap's run.sh sleeps for a total of 10 seconds before launching all modbus/dnp3 servers, but disconnection messages are still seen when sleep time is 10.
    # using 15 seconds gives enough buffer for the servers to be started and spun up based on guess-and-check adjustments to startup time value
    component_server_startup_time = sleeptime * 15
    print(f"Waiting {component_server_startup_time} seconds to give PSM container's servers time to boot.")
    time.sleep(sleeptime * component_server_startup_time)

    start_fps_service(site_product, machine_config_dir_symlink)


# capture the product configured to be run and launch the relevant core processes
def read_product():
    global product
    product_txt = machine_config_dir_symlink + "/product.txt"
    if (os.path.isfile(product_txt)):
        product_txt_file = open(product_txt, "r")
        product = product_txt_file.read()
        product_txt_file.close()
    if product != site_product and product != fleet_product:
        error_exit("invalid product option")


# powercloud services
def start_ftd():
    if launch_powercloud == True:
        start_fps_service("ftd", machine_config_dir_symlink, True, {"ftd.json": "configuration"})


def start_cloud_sync():
    if launch_powercloud == True:
        start_fps_service("cloud_sync", machine_config_dir_symlink, True, {"cloud_sync.json": "configuration"})


def start_dts():
    if launch_powercloud == True:
        start_fps_service("dts", machine_config_dir_symlink, True, {"dts.json": "configuration"})


# LEVEL 4: start cops
def start_cops():
    start_fps_service("cops", machine_config_dir_symlink, root=True)


# LEVEL 5: start web_server, web_ui
def start_web_ui():
    if os.path.isdir(machine_config_dir_symlink + "/web_ui"):
        # The web UI dashboard config is UI editable, meaning it is only overwritten if the --init flag is sent
        if reset_setpoints == True:
            print("loading web_ui config into dbi")
            load_dbi('ui_config', machine_config_dir_symlink, file_document_renames={'custom_pages.json': 'assets', 'layout_configs.json': 'layout'})

        print("starting web_server")
        Popen(['sudo', bin_dir + '/FlexGenMCP', machine_config_dir_symlink + '/mcp/mcp_web_server.json'])

# Pull all current service files from hybridos/<product>/<product>.service
# and place these files at /usr/lib/systemd/system


def update_service_files():
    # Always reset mongod permissions
    # Popen(["sudo", "systemctl", "stop", "mongod"])
    # cmd = "sudo chown mongod:mongod /home/mongodb/* -R"
    # run(cmd, shell=True)  # running a shell command.
    # time.sleep(sleeptime)

    # Update service files if necessary
    if update_services:
        print("Updating .service files...")
        hybridos_dir = "/home/hybridos/git/hybridos"
        target_dir = "/usr/lib/systemd/system"

        # Iterate through hybridos and generate folder of files to copy
        for root, dirs, files in os.walk(hybridos_dir):
            for dir in dirs:
                # Check if our directory is a listed hybridos service
                if dir in hybridos_services:
                    folder = os.path.join(root, dir)
                    files = os.listdir(folder)
                    if files:
                        service_files = [file for file in files if file.endswith('.service')]

                        # If .service files are found copy it
                        if service_files:
                            for service_file in service_files:
                                file_to_copy = os.path.join(folder, service_file)
                                cmd = "sudo cp " + file_to_copy + " " + target_dir
                                run(cmd, shell=True)  # running a shell command.
                                print(f"File '{service_file}' updated.")

        # Reload systemctl daemon with updated files
        cmd = "sudo systemctl daemon-reload"
        run(cmd, shell=True)  # running a shell command.
        time.sleep(sleeptime)


def run_hybridos():
    read_product()

    start_influx()
    start_mongo()
    start_fims()
    start_dbi()
    start_events()
    start_metrics()
    start_go_metrics()
    start_fleet_manager()
    start_scheduler()
    start_dnp3_clients()
    start_dnp3_server()
    start_site_controller()
    start_modbus_clients()
    start_modbus_server()
    start_ftd()
    start_cloud_sync()
    start_dts()
    start_cops()
    start_web_ui()

    time.sleep(sleeptime)
    print("Finished launching all processes")


def is_docker_login() -> bool:
    """
        Checks to see if user is logged into docker
        Returns - True if user is logged in | False if user is not logged in
    """
    # get docker username
    docker_command_output = run("docker info | grep Username", stdout=PIPE, shell=True, universal_newlines=True)
    docker_username = docker_command_output.stdout.strip()
    # if not logged in then docker username will be null
    if len(docker_username) > 0:
        return True
    else:
        return False


if __name__ == "__main__":
    if not is_docker_login():
        error_exit("Error: not logged into Docker!")

    parse_args()
    update_service_files()
    scan_config_directory()
    stop_hybridos()
    stop_systemd_hybridos()
    check_environment()
    run_hybridos()
