import os
import json

import sys

if sys.version_info[0] == 2:
    print("You are running Python 2")
    my_input = raw_input
elif sys.version_info[0] == 3:
    print("You are running Python 3")
    my_input = input
else:
    print("Unknown Python version")
    import json
import os

CONFIG_FILE = 'get_logs_prefs.json'

def load_preferences():
    if os.path.exists(CONFIG_FILE):
        with open(CONFIG_FILE, 'r') as f:
            return json.load(f)
    return {}

def save_preferences(prefs):
    with open(CONFIG_FILE, 'w') as f:
        json.dump(prefs, f, indent=4)
    
# def get_input_with_default(prefs, key, prompt):
#     """Get user input with a default value from the preferences."""
#     default_value = prefs.get(key, '')
#     user_input = input(f"{prompt} [default: {default_value}]: ").strip() or default_value
#     prefs[key] = user_input
#     return user_input

def get_input_with_default(prefs, key, prompt, default_from_args=''):
    """Get user input with a default value from the preferences or arguments."""
    default_value = prefs.get(key, default_from_args)
    user_input = my_input(f"{prompt} [default: {default_value}]: ").strip() or default_value
    prefs[key] = user_input
    return user_input

def get_logs(directory="/var/log/flexgen/gcom_iothreads/"):
    logs = []
    files = [f for f in os.listdir(directory) if f.endswith('.log')]
    print(" found files")
    print(files)

    for file in files:
        with open(os.path.join(directory, file), 'r') as f:
            for line in f:
                try:
                    data = json.loads(line)
                    if data not in logs:
                        logs.append(data)
                except json.JSONDecodeError:
                    pass

    return sorted(logs, key=lambda x: (x["date"], x["time"]))



def display_logs(logs):
    prefs = load_preferences()


    # Get the start date and search string with defaults
    start_date = get_input_with_default(prefs, 'start_date', 'Enter the start date', 'any')
    start_time = get_input_with_default(prefs, 'start_time', 'Enter the start time', '')
    search_string = get_input_with_default(prefs, 'search_string', 'Enter the search string', '')
    num_lines = get_input_with_default(prefs, 'num_lines', 'Enter the number of lines', 'all')

    if start_date.lower() != "any":
        # If only date is provided without time
        if not start_time:
            start_time = "00:00:00"
        
        # Filter logs based on provided date and time
        logs = [log for log in logs if log["date"] > start_date or (log["date"] == start_date and log["time"] >= start_time)]



    if search_string:
        logs = [log for log in logs if search_string in json.dumps(log)]


    if num_lines.lower() != "all":
        try:
            num = int(num_lines)
            logs = logs[:num]
        except ValueError:
            print("Invalid number of lines, displaying all...")

    save_preferences(prefs)

    for log in logs:
        print(json.dumps(log))

if __name__ == "__main__":
    logs = get_logs()
    display_logs(logs)
