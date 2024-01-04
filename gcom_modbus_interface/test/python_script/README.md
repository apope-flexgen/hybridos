# Test Framework
## Setup
1. This program is run from Windows, so you will need to do this in your local command prompt.
    ```
    python -m pip install -r requirements.txt
    ```
2. Make sure you have a Docker container setup for both your server and your client. These do not have to be running when you call the python script.
  - Add the following to `~/.bashrc` for both the client container and the server container. (This starts fims_server when you first start a bash script but not if it's already running on the container.):
    ```
    if [[ ! $(ps ax | grep -v grep | grep fims_server) ]] ; then
    fims_server > /dev/null 2>&1 &
    fi
    ```
  - I recommend adding the following to the respective `~/.bashrc` file so that your containers are easy to identify:
    ```
    PROMPT_COMMAND='echo -en "\033]0;Client\a"'    # for your client
    PROMPT_COMMAND='echo -en "\033]0;Server\a"'    # for your server
    ```
3. Set up a config file for both your client and your server.
  - Make sure the client has "client" in the name of the file and the server has "server" in the name of the file.
  - Save these files in `<python_script_dir>/configs`
  - Make sure the "id" fields and offsets match for the client and the server side (though the uris can have "client" and "server" in them).
  - Make sure the IP addresses match your server and client, assuming you start your server before your client.

4. Update `test_utils.py` to make sure everything is named correctly (just the first few lines).

## Running the script
As long as the LOCAL_PYTHON_SCRIPT_DIR is the full path to the `run_test.py` script directory, then all you need to do is run the python script from anywhere in Windows.
```
python <path_to_python_script_folder>/run_test.py
```


## NOTE: Right now, the script cannot produce expected outputs for modbus registers.
