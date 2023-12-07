import os
import pyautogui
import pygetwindow as gw
import time
from screeninfo import get_monitors
from test_utils import *


def setup_client_and_server_windows():
    # Define the dimensions of the external monitor
    # Identify the external monitor (you may need to customize this)

    for m in get_monitors():
        external_monitor_width = m.width
        external_monitor_height = m.height
        break

    # Get external monitor dimensions
    new_width = external_monitor_width // 2
    new_height = external_monitor_height // 3

    # Define the command to run in each command prompt window
    commands = [
        f"docker exec -it {CLIENT_CONTAINER} bash",
        f"docker exec -it {SERVER_CONTAINER} bash", 
        f"docker exec -it {CLIENT_CONTAINER} bash",  
        f"docker exec -it {SERVER_CONTAINER} bash",
        f"docker exec -it {CLIENT_CONTAINER} bash", 
        f"docker exec -it {SERVER_CONTAINER} bash", 
    ]

    # Open 6 command prompt windows and run commands
    for q in range(6):
        # Open a new command prompt window
        os.system('start cmd.exe')

        # Wait briefly for the window to open
        time.sleep(0.5)
        if q == 0:
            pyautogui.write(f'docker start {SERVER_CONTAINER}\n')
            time.sleep(3)
            pyautogui.write(f'docker start {CLIENT_CONTAINER}\n')
            time.sleep(3)
        # Run the specified command in the command prompt window
        pyautogui.write(commands[q] + '\n')

        time.sleep(0.5)
        pyautogui.write(f"cd {DOCKER_PYTHON_SCRIPT_DIR}\n")

        if q == 0 or q == 1:
            pyautogui.write(f"pkill {INTERFACE}_client\n")
            pyautogui.write(f"pkill {INTERFACE}_server\n")
            pyautogui.write("pkill fims_listen\n")

    # Give some time for the windows to settle
    time.sleep(2)

    server_cmd_prompts = gw.getWindowsWithTitle(f"{SERVER_CONTAINER_TITLE}")
    for q, cmd_prompt in enumerate(server_cmd_prompts):
        cmd_prompt.moveTo(0, 0)  
        cmd_prompt.resizeTo(new_width, new_height)
        cmd_prompt.move(external_monitor_width//2, new_height*q) # Move to the right side of the screen

    client_cmd_prompts = gw.getWindowsWithTitle(f"{CLIENT_CONTAINER_TITLE}")
    for q, cmd_prompt in enumerate(client_cmd_prompts):
        cmd_prompt.moveTo(0,0)  # Move to the left side of the screen
        cmd_prompt.resizeTo(new_width, new_height)
        cmd_prompt.move(0, new_height*q)
    
    client_commands = [
        f"{INTERFACE}_client {DOCKER_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/filename.json",
        f"fims_listen > {DOCKER_PYTHON_SCRIPT_DIR}/{TEST_LOG_DIR}/filename.log",
        f"sh {DOCKER_PYTHON_SCRIPT_DIR}/{TEST_SCRIPT_DIR}/filename.sh"
    ]

    server_commands = [
        f"{INTERFACE}_server {DOCKER_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}/filename.json",
        f"fims_listen > {DOCKER_PYTHON_SCRIPT_DIR}/{TEST_LOG_DIR}/filename.log",
        f"sh {DOCKER_PYTHON_SCRIPT_DIR}/{TEST_SCRIPT_DIR}/filename.sh"
    ]

    commands_both = ["pkill fims_listen", f"pkill {INTERFACE}_client", f"pkill {INTERFACE}_server", "pkill -SIGKILL bash"]
    server_files = []
    client_files = []

    # Loop through the command prompt windows and run additional commands
    for file in os.listdir(f"{LOCAL_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}"):
        filename = file.replace('.json', '')
        if 'server' in filename:
            server_files.append(filename)
        elif 'client' in filename:
            client_files.append(filename)

    server_files.sort()
    client_files.sort()

    for file_num in range(len(server_files)):
        for q, cmd_prompts in enumerate(list(zip(client_cmd_prompts[0:3],server_cmd_prompts[0:3]))):
            client_cmd_prompt = cmd_prompts[0]
            server_cmd_prompt = cmd_prompts[1]

            server_cmd_prompt.activate()
            command = server_commands[q].replace('filename', server_files[file_num])
            pyautogui.write(command + '\n')
            if q == 2:
                with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_SCRIPT_DIR}/{server_files[file_num]}.sh", 'r') as file:
                    first_line = file.readline()
                    first_line = first_line.replace("#","")
                    try:
                        sleep_time = round(float(first_line))
                    except:
                        sleep_time = 30
                    sleep_time += 20
                print(f"Sleeping for {sleep_time} seconds")
                time.sleep(sleep_time)
            
            client_cmd_prompt.activate()
            command = client_commands[q].replace('filename', client_files[file_num])
            pyautogui.write(command + '\n')

            if q == 2:
                with open(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_SCRIPT_DIR}/{client_files[file_num]}.sh", 'r') as file:
                    first_line = file.readline()
                    first_line = first_line.replace("#","")
                    try:
                        sleep_time = round(float(first_line))
                    except:
                        sleep_time = 30
                sleep_time += 20
                print(f"Sleeping for {sleep_time} seconds")
                time.sleep(sleep_time)
                
                client_cmd_prompt.activate() # in case they clicked out while waiting for this to finish
                for command in commands_both:
                    pyautogui.write(command + '\n')
                server_cmd_prompt.activate()
                for command in commands_both:
                    pyautogui.write(command + '\n')

            time.sleep(1)
    
    for cmd_prompt in server_cmd_prompts[0:3]:
        cmd_prompt.activate()
        cmd_prompt.close()
    for cmd_prompt in client_cmd_prompts[0:3]:
        cmd_prompt.activate()
        cmd_prompt.close()