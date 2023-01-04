import pytest
from pytest import ExitCode

import sys
import time
import os 
import subprocess

sys.path.append(".")
from pyfims_rewrite import pyfims

# set up site controller
@pytest.fixture (scope = "session")
def site_cntlr(): 
    sleeptime = 1
    bin_dir = "/usr/local/bin"
    config_dir = "/usr/local/etc/config"

    if bin_dir in os.getenv('PATH') == False:
        print("/usr/local/bin not found in PATH environmental variable. Please add it for convenience.")
        sys.exit()

    subprocess.run('/home/vagrant/git/scripts/hybridos_stop.sh')
    time.sleep(sleeptime)

    if not os.path.isdir(config_dir):
        print("config directory: ", config_dir)
        print("config directory not found.")
        sys.exit()

    print("##### LAUNCH SITE CONTROLLER #####")
    print("config directory: " + os.readlink(config_dir))
    print("starting system...")

    # LEVEL 1: start influxdb, mongodb, fims server
    subprocess.Popen(['sudo', 'mongod', '--config', '/etc/mongod.conf'])
    print("starting fims")
    subprocess.Popen([bin_dir + '/fims_server'])
    time.sleep(sleeptime)

    # LEVEL 2: start metrics, events, dbi, scheduler
    if os.path.isdir(config_dir + "/metrics"):
        print("starting metrics")
        subprocess.Popen([bin_dir + '/FlexGenMCP', config_dir + '/mcp/mcp_metrics.json'])

    if os.path.isdir(config_dir + "/events"):
        print("starting events")
        subprocess.Popen([bin_dir + '/FlexGenMCP', config_dir + '/mcp/mcp_events.json'])

    if os.path.isdir(config_dir + "/dbi"):
        print("starting dbi")
        subprocess.Popen([bin_dir + '/FlexGenMCP', config_dir + '/mcp/mcp_dbi.json'])

    time.sleep(sleeptime)

    if os.path.isfile(config_dir + "/mcp/mcp_scheduler.json"):
        if subprocess.Popen(['pgrep', 'dbi']) == 0:
            print("dbi is not running, cannot reset scheduler configurations")
            sys.exit()
        print("Resetting scheduler configurations")
        subprocess.Popen(['fims_send', '-m', 'set', '-u', '/dbi/scheduler/configuration', '-f', config_dir + '/scheduler/scheduler.json'])
        subprocess.Popen(['fims_send', '-m', 'set', '-u', '/dbi/scheduler/modes', '-f', config_dir + '/scheduler/modes.json'])
        subprocess.Popen(['fims_send', '-m', 'set', '-u', '/dbi/scheduler/events', '-f', config_dir + '/scheduler/events.json'])
        time.sleep(sleeptime)
        print("starting scheduler")
        subprocess.Popen([bin_dir + '/FlexGenMCP', config_dir + '/mcp/mcp_scheduler.json'])

    # LEVEL 3: start core product processes 
    # define function for generating one port argument (of the form "-p <port number>:<port number>") per Modbus component.
    # used to link Modbus communications from Site Controller to TWINS Docker container
    def generate_port_arguments(num_components):
        if (num_components == 0):
            return

        # start with port 10000 and increment by 1 for each port
        current_port = 10000
        port_args = " ".join([ "{port:d}:{port:d}".format(port=i) for i in range(current_port, current_port+num_components)])
        return port_args

    # define function for reading a repo's tag from repo.txt
    def read_repo_tag(repo_name):
        repo_txt = config_dir + "/repo.txt"
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
            print ("<<<<< DID NOT FIND " + repo_name + " LINE IN REPO.TXT >>>>>")
            return [1, None]
        # extract tag from repo line
        line_frags = repo_line.split("|")
        if len(line_frags) != 2:
            print("<<<<< FOUND " + repo_name + "LINE IN REPO.TXT BUT AFTER SPLITTING ON | FOUND ", line_frags, " FRAGS NOT 2")
            return [1, None]
        repo_tag = line_frags[1]
        return [0, repo_tag]

    # define function for starting processes specific to Site Controller product
    def start_site_controller_processes():
        # get TWINS image tag from repo.txt
        twins_pm_repo_tag = read_repo_tag("twins_pm")
        if twins_pm_repo_tag == [1, None]:
            return 1
        twins_pm_image_tag = twins_pm_repo_tag[1][1:] # popping off 'v' from beginning of tag name

        # will need to open one port per modbus component
        num_components_file = open(config_dir + "/twins/components.txt", "r")
        read_num_components = num_components_file.read()
        num_components_file.close()
        num_components = len(read_num_components.split())
        port_args = generate_port_arguments(num_components)

        # start TWINS container
        print("flushing Docker system")
        subprocess.Popen(['sudo', 'docker', 'system', 'prune', '--force', '--filter', 'label=twins', '--filter', 'label=twins_network'])
        print("starting TWINS Docker container")
        subprocess.Popen(['sudo', 'docker', 'network', 'create', '--subnet=172.3.27.0/24', 'twins_network'])
        port_args_split = port_args.split()
        launch_container_command = ["sudo", "docker", "run", "--name", "twins", "-dit", "-v", config_dir + "/twins/:/home/config", "--net", "twins_network", "--ip", "172.3.27.2"]
        i = 0
        for i in range(num_components - 1):
            launch_container_command.append("-p")
            launch_container_command.append(port_args_split[i])
            i += 1
        launch_container_command.append("flexgen/twins_pm:" + twins_pm_image_tag)
        subprocess.Popen(launch_container_command)
        time.sleep(sleeptime)

        #load Site Controller configs
        print("loading site_controller configs into dbi")
        subprocess.Popen(['fims_send', '-m', 'set', '-u', '/dbi/site_controller/assets', '-f', config_dir + '/site_controller/assets.json'])
        subprocess.Popen(['fims_send', '-m', 'set', '-u', '/dbi/site_controller/sequences', '-f', config_dir + '/site_controller/sequences.json'])
        subprocess.Popen(['fims_send', '-m', 'set', '-u', '/dbi/site_controller/variables', '-f', config_dir + '/site_controller/variables.json'])
        # Site Controller setpoints are runtime editable, for testing purposes, going to overwrite on every set up as if --init was present
        print("Resetting site controller runtime settings")
        subprocess.Popen(['fims_send', '-m', 'del', '-u', '/dbi/site_controller/setpoints'])
        time.sleep(sleeptime)

        # start modbus clients
        print("starting modbus clients")
        if os.path.isdir(config_dir + "/modbus_client"):
            subprocess.Popen([bin_dir + '/FlexGenMCP', config_dir + '/mcp/mcp_modbus_client.json'])
        else:
            print("Modbus client disabled for testing.")
            time.sleep(sleeptime)

        # start site_controller executable
        if os.path.isdir(config_dir + "/site_controller"):
            print("starting site_controller")
            # will enable memory checking via valgrind for memeory leaks 
            subprocess.Popen([bin_dir + '/FlexGenMCP', config_dir + '/mcp/mcp_site_controller_memcheck.json'])

        # start modbus server if applicable
        if os.path.isdir(config_dir + "/modbus_server"):
            print("starting modbus server")
            subprocess.Popen(['sudo', bin_dir + '/FlexGenMCP', config_dir + '/mcp/mcp_modbus_server.json'])
        else:
            print("Modbus server disabled for testing.")
            time.sleep(sleeptime)

        return 0

    # launch Site Controller core processes
    if start_site_controller_processes() == 1:
        print("Failed to start Site Controller core processes")
        sys.exit() 

    # LEVEL 4: start cops
    if os.path.isdir(config_dir + "/cops"):
        print("loading cops config into dbi")
        subprocess.Popen(['fims_send', '-m', 'set', '-u', '/dbi/cops/configuration', '-f', config_dir + '/cops/cops.json'])
        time.sleep(sleeptime)
        print("starting cops")
        subprocess.Popen(['sudo', bin_dir + '/FlexGenMCP', config_dir + '/mcp/mcp_cops.json'])

    #LEVEL 5: start web_server, web_ui
    if os.path.isdir(config_dir + "/web_ui"):
        # The web UI dashboard config is UI editable, for testing purposes, going to overwite as if --init flag is present 
        print("loading web_ui config into dbi")
        subprocess.Popen(['fims_send', '-m', 'set', '-u', '/dbi/ui_config/dashboard', '-f', config_dir + '/ui_config/dashboard.json'])
        subprocess.Popen(['fims_send', '-m', 'set', '-u', '/dbi/ui_config/assets', '-f', config_dir + 'ui_config/CA_configs.json'])
        subprocess.Popen(['fims_send', '-m', 'set', '-u', '/dbi/ui_config/layout', '-f', config_dir + '/ui_config/layout_configs.json'])
        print("starting web_server")
        subprocess.Popen(['sudo', bin_dir + '/web_server', bin_dir + '/web_ui/', config_dir + '/web_ui', config_dir + '/web_server'])

# set up pyfims
@pytest.fixture (scope = "session")
def pyfims_instance(): 
    # connect to fims server
    p = pyfims()
    x = p.Connect("pytest_connection")

    # if unable to connect to fims server, cannot continue with tests 
    if x == None or x[1] != 0:
        discontinue_tests(ExitCode.INTERRUPTED) 

    return p

# set up input commands 
@pytest.fixture 
def input_values():
    input = {
        "uri_1": "/features/active_power/absolute_ess_direction_flag",        
        "uri_2": "/features/active_power/absolute_ess_kW_cmd",
        "absolute_ess_direction_flag": True,     # true indicates charging, false indicates discharging
        "absolute_ess_kW_cmd": 0                 # kw command 
    }

    return input

# if unable to continue running tests for some reason, exit      
def discontinue_tests(exit_code):
    sys.exit(exit_code)

# test absolute ess power feature 
def test_absolute_ess(site_cntlr, pyfims_instance, input_values): 
    # subscribe to uri(s) 
    x = pyfims_instance.Subscribe("/pytest_connection") 
    assert x[1] == 0  

    # set absolute_ess_direction_flag
    x = pyfims_instance.Send("set", "/features/active_power/absolute_ess_direction_flag", None, True)
    assert x[1] == 0

    # get absolute_ess_direction_flag 
    x = pyfims_instance.Send("get", "/features/active_power/absolute_ess_direction_flag", "/pytest_connection")
    # for debugging:  
    assert x == None

    x = pyfims_instance.Receive(10) 
    # for debugging: 
    assert x == None 

    pyfims_instance.Close() 
