# Test Framework
## Setup
1. This program is run from Windows, so you will need to do this in your local command prompt.
    ```
    python -m pip install -r requirements.txt
    ```
2. Edit the top part of `global_utils.py` to make sure that it matches your system.
  - If you would like to save your settings for later, you might want to rename this file `user_global_utils.py` (so that it doesn't get overwritten when you checkout a new branch).
3. Set up a config file for both your client and your server.
  - Make sure the client has "client" in the name of the file and the server has "server" in the name of the file.
  - Save these files in `<python_script_dir>/configs`

## Running the script
As long as the LOCAL_PYTHON_SCRIPT_DIR is the full path to the `run_test.py` script directory, then all you need to do is run the python script from anywhere in Windows.
```
python <path_to_python_script_folder>/test_comms.py
```

## Disclaimer
This is a side project for me and is made specifically for testing GCOM systems, so it might be organized differently than it might make sense to organize it. There are a lot of things that could be done better or more generically. (I could come up with a pretty long list myself...)