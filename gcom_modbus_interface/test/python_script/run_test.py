from compare_messages import *
from gen_test_script import *
from test_python_output import *
from test_utils import *
from setup_client_and_server import *

if __name__ == '__main__':
    for folder in dirs:
        if not os.path.exists(f"{LOCAL_PYTHON_SCRIPT_DIR}/{folder}"):
            os.mkdir(f"{LOCAL_PYTHON_SCRIPT_DIR}/{folder}")
    for file in os.listdir(f"{LOCAL_PYTHON_SCRIPT_DIR}/{CONFIGS_DIR}"):
        if file.lower().endswith('.json'):
            gen_commands(file)
    
    setup_client_and_server_windows()
    
    test_output()
    