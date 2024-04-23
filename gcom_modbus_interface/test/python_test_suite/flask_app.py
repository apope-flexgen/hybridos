'''
A very basic Flask application that runs commands from within a
docker container. Required because `docker exec` is too slow.
'''
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
    Run a command using the /bin/bash of a Docker container.
    '''
    command = request.args.get("command")
    if "+" in command:
        print(command)
    subprocess.run(command, shell=True, check=False)
    return "Ran all commands"

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=443)
