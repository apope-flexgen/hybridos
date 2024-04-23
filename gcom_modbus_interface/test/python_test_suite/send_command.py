# send a command to the flask app  
import requests
import sys

PORT        = "4040"
RUN_COMMAND = "docker/run_command"

def send_command_to_flask_app(port, command):
    url = f"http://localhost:{port}/{RUN_COMMAND}"
    params = {'command': command}
    try:
        response = requests.get(url, params=params)
        response.raise_for_status()  # Raise an error for bad responses
        return response.text
    except requests.RequestException as e:
        return f"Request failed: {e}"

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python send_command.py port '<command>'")
        sys.exit(1)

    
    port = sys.argv[1]
    command = sys.argv[2]
    result = send_command_to_flask_app(port, command)
    print(result)
