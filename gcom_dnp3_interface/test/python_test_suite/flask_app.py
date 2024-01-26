from flask import Flask, request
import subprocess

app = Flask(__name__)


@app.route('/docker/run_command')
def run_command():
    command = request.args.get("command")
    if "+" in command:
        print(command)
    subprocess.run(command, shell=True)
    return "Ran all commands"

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=443)