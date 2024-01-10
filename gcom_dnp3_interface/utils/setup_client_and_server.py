# This is a simple python script that will set up 6 cmd windows (3 clients and 3 servers) and arrange them nicely on the screen.
# To run:
# Create one container named client_container (vlink C:/<path_to_gcom_modbus_interface>:/home/docker/hybridos/gcom_modbus_interface)
# Create one container named server_container (vlink C:/<path_to_gcom_modbus_interface:/home/docker/hybridos/gcom_modbus_interface)
# Add the following to ~/.bashrc for client_container:  PROMPT_COMMAND='echo -en "\033]0;Client\a"'
# Add the following to ~/.bashrc for server_container:  PROMPT_COMMAND='echo -en "\033]0;Server\a"'
# Run from Windows (not from within a contianer)

import os
import pyautogui
import pygetwindow as gw
import time
from pynput.keyboard import Key, Controller
from screeninfo import get_monitors

keyboard = Controller()


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
    "docker exec -it client_container bash",
    "docker exec -it server_container bash", 
    "docker exec -it client_container bash",  
    "docker exec -it server_container bash",
    "docker exec -it client_container bash", 
    "docker exec -it server_container bash", 
]

# Open 6 command prompt windows and run commands
for i in range(6):
    # Open a new command prompt window
    os.system('start cmd.exe')

    # Wait briefly for the window to open
    time.sleep(0.5)
    if i == 0:
        pyautogui.write('docker start server_container\n')
        time.sleep(3)
        pyautogui.write('docker start client_container\n')
        time.sleep(3)
    # Run the specified command in the command prompt window
    pyautogui.write(commands[i] + '\n')

    time.sleep(0.5)
    pyautogui.write("cd /home/docker/hybridos/gcom_dnp3_interface\n")
    if i == 0:
        pyautogui.write("make install\n")

# Give some time for the windows to settle
time.sleep(2)

cmd_prompts = gw.getWindowsWithTitle("Server")
for i, cmd_prompt in enumerate(cmd_prompts):
    cmd_prompt.moveTo(0, 0)  
    cmd_prompt.resizeTo(new_width, new_height)
    cmd_prompt.move(external_monitor_width//2, new_height*i) # Move to the right side of the screen

cmd_prompts = gw.getWindowsWithTitle("Client")
for i, cmd_prompt in enumerate(cmd_prompts):
    cmd_prompt.moveTo(0,0)  # Move to the left side of the screen
    cmd_prompt.resizeTo(new_width, new_height)
    cmd_prompt.move(0, new_height*i) 