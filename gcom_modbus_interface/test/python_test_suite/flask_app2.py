#
# Flask process running on the docker container
#
from flask import Flask, request
import subprocess, sys

try:
    from flask import Flask, request
except ImportError:
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'flask'])
    from flask import Flask, request

app = Flask(__name__)

@app.route('/docker/run_command')
def run_command():
    '''
    Run a command using the /bin/bash of a Docker container and return its output.
    '''
    command = request.args.get("command")
    # Important: Ensure you validate and sanitize the command here to prevent command injection
    
    if "+" in command:  # Example of a very basic form of validation
        return "Invalid command", 400

    try:
        # For Python 3.6 and earlier, use stdout and stderr PIPEs directly
        result = subprocess.run(command, shell=True, check=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        output = result.stdout if result.stdout else result.stderr
        return output
        # Capture the output of the command
        #result = subprocess.run(command, shell=True, check=True, capture_output=True, text=True)
        # Return the stdout if command executed, else return stderr
        #return result.stdout if result.stdout else "Command executed successfully, but no output was returned."
    except subprocess.CalledProcessError as e:
        # This captures errors from unsuccessfully executed commands
        return f"Command failed:\nSTDOUT: {e.stdout}\nSTDERR: {e.stderr}", 501
    except Exception as e:
        print(str(e))
        return f"Some other error occurred: {str(e)}", 502

if __name__ == '__main__':
    #app.run(host="0.0.0.0", port=443, ssl_context='adhoc')
    app.run(host="0.0.0.0", port=443)
