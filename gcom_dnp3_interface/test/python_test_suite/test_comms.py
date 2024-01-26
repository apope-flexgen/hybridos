from global_utils import *
import unittest
import argparse
import signal
from comms_testing import *
from custom_thread import CustomThread

quit_test = False
def quit_testing(sig, frame):
    global quit_test
    if sig == signal.SIGINT:
        quit_test = True

class CommsTest(unittest.TestCase):
    client_container = None
    server_containter = None
    fims_listen_client = None
    fims_listen_server = None

    @classmethod
    def setUpClass(cls):
        gen_directories()
        gen_test_cases(new_test_cases)
        signal.signal(signal.SIGINT, quit_testing)
    
    @classmethod
    def tearDownClass(cls):
        cls.quit = True
        if cls.client_container is not None and cls.server_container is not None:
            cls.fims_listen_client.join(subprocess.run, ["docker", "exec", CLIENT_CONTAINER, "sh", "-c", "fims_send -m set -u /some"])
            cls.fims_listen_server.join(subprocess.run, ["docker", "exec", SERVER_CONTAINER, "sh", "-c", "fims_send -m set -u /some"])
            kill_containers(cls.client_container, cls.server_container)
            write_output_files(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_OUTPUT_DIR}", True)

    def test_client_and_server(self):
        for (idx, ((client_file, server_file),(client_test_cases, server_test_cases))) in enumerate(zip(all_config_file_pairs, output_file_content)):
            client_file = client_file.replace(".json", "")
            server_file = server_file.replace(".json", "")
            self.__class__.client_container, self.__class__.server_container = start_containers(client_file, server_file)
            self.__class__.fims_listen_client = CustomThread(target=capture_fims_listen, args=("client", CLIENT_CONTAINER, client_file))
            self.__class__.fims_listen_server = CustomThread(target=capture_fims_listen, args=("server", SERVER_CONTAINER, server_file))
            self.__class__.fims_listen_client.start()
            self.__class__.fims_listen_server.start()

            for test_id, client_test_case in client_test_cases.items():
                with self.subTest():
                    result, str_output = run_client_test_case(test_id, client_test_case)
                    self.assertTrue(result, msg=str_output)
                    client_test_cases[test_id] = client_test_case
                    if result:
                        print(f"Test Case {test_id}: PASS")
                    else:
                        print()

            for test_id, server_test_case in server_test_cases.items():
                with self.subTest():
                    result, str_output = run_server_test_case(test_id, server_test_case)
                    self.assertTrue(result, msg=str_output)
                    server_test_cases[test_id] = server_test_case
                    if result:
                        print(f"Test Case {test_id}: PASS")
                    else:
                        print()
            
            output_file_content[idx] = (client_test_cases, server_test_cases)
            write_output_files(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_OUTPUT_DIR}", True)
            self.__class__.fims_listen_client.join(subprocess.run, ["docker", "exec", CLIENT_CONTAINER, "sh", "-c", "fims_send -m set -u /some"])
            self.__class__.fims_listen_server.join(subprocess.run, ["docker", "exec", SERVER_CONTAINER, "sh", "-c", "fims_send -m set -u /some"])
            kill_containers(self.__class__.client_container, self.__class__.server_container)
            self.__class__.client_container = None
            self.__class__.server_containter = None
            self.__class__.fims_listen_client = None
            self.__class__.fims_listen_server = None
            
        write_output_files(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_OUTPUT_DIR}", True)

if __name__ == '__main__':
    unittest.main()