'''
The test_comms module will run everything that is needed to check the communications
interface of interest (specified by global_utils).
'''
import os
import subprocess
try:
    from user_global_utils import LOCAL_PYTHON_SCRIPT_DIR, NEW_TEST_CASES, \
    USE_LATEST_RELEASE, CLIENT_CONTAINER, SERVER_CONTAINER, TEST_OUTPUT_DIR
except ImportError:
    from global_utils import LOCAL_PYTHON_SCRIPT_DIR, NEW_TEST_CASES, \
    USE_LATEST_RELEASE, CLIENT_CONTAINER, SERVER_CONTAINER, TEST_OUTPUT_DIR
import unittest
import signal
from comms_testing import gen_directories, check_for_binaries, update_file_contents, \
containers_exist, build_containers, gen_test_cases, kill_containers, start_containers, \
run_client_test_case, run_server_test_case
from gen_test_script import write_output_files, ALL_CONFIG_FILE_PAIRS, OUTPUT_FILE_CONTENT
from custom_thread import CustomThread
from run_test_case import capture_fims_listen
from git_info import git_info

QUIT_TEST = False
def quit_testing(sig, frame):
    '''
    Signal handler for Ctrl + C
    '''
    del frame
    global QUIT_TEST
    if sig == signal.SIGINT:
        QUIT_TEST = True

class CommsTest(unittest.TestCase):
    '''
    The CommsTest class is a unittest TestCase class that sets up and takes
    down all servers/clients using the configs in the CONFIGS_DIR.
    '''
    client_container = None
    server_containter = None
    fims_listen_client = None
    fims_listen_server = None

    @classmethod
    def setUpClass(cls):
        '''
        Check for all dependencies, set up folders and files,
        set up containers, and generate test cases.
        '''
        os.chdir(LOCAL_PYTHON_SCRIPT_DIR)
        gen_directories()
        if not USE_LATEST_RELEASE:
            check_for_binaries()
        update_file_contents()
        if not containers_exist():
            build_containers()
        gen_test_cases(NEW_TEST_CASES)
        signal.signal(signal.SIGINT, quit_testing)

    @classmethod
    def tearDownClass(cls):
        '''
        Kill the docker containers and save all fims_listen output and program output
        to file
        '''
        print("Cleaning up test program...")
        if cls.client_container is not None and cls.server_container is not None:
            print("Killing fims_listen on client...")
            cls.fims_listen_client.join(1, subprocess.run,
            ["docker", "exec", CLIENT_CONTAINER, "sh", "-c", "fims_send -m set -u /some"])
            print("Killing fims_listen on server...")
            cls.fims_listen_server.join(1, subprocess.run,
            ["docker", "exec", SERVER_CONTAINER, "sh", "-c", "fims_send -m set -u /some"])
            kill_containers(cls.client_container, cls.server_container)
            print("Writing output files...")
            write_output_files(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_OUTPUT_DIR}", True)

    def test_client_and_server(self):
        '''
        Iterate through all test files in the CONFIG_DIR and run all associated test cases.
        '''
        for (idx, ((client_file, server_file),(client_test_cases, server_test_cases))) \
            in enumerate(zip(ALL_CONFIG_FILE_PAIRS, OUTPUT_FILE_CONTENT)):
            client_file = client_file.replace(".json", "")
            server_file = server_file.replace(".json", "")
            self.__class__.client_container, self.__class__.server_container = \
                start_containers(client_file, server_file)
            self.__class__.fims_listen_client = CustomThread(target=capture_fims_listen,
                                                args=("client", CLIENT_CONTAINER, client_file))
            self.__class__.fims_listen_server = CustomThread(target=capture_fims_listen,
                                                args=("server", SERVER_CONTAINER, server_file))
            self.__class__.fims_listen_client.start()
            self.__class__.fims_listen_server.start()

            client_test_cases['git_branch'] = git_info.branch
            client_test_cases['git_commit_hash'] = git_info.commit_hash
            client_test_cases['git_commit_author'] = git_info.author
            server_test_cases['git_branch'] = git_info.branch
            server_test_cases['git_commit_hash'] = git_info.commit_hash
            server_test_cases['git_commit_author'] = git_info.author

            for test_id, client_test_case in client_test_cases.items():
                if test_id in ['git_commit_hash', 'git_branch', 'git_commit_author']:
                    continue
                if QUIT_TEST:
                    break
                with self.subTest():
                    result, str_output = run_client_test_case(test_id, client_test_case)
                    self.assertTrue(result, msg=str_output)
                    client_test_cases[test_id] = client_test_case
                    if result:
                        print(f"Test Case {test_id}: PASS")
                    else:
                        print()

            for test_id, server_test_case in server_test_cases.items():
                if test_id in ['git_commit_hash', 'git_branch', 'git_commit_author']:
                    continue
                if QUIT_TEST:
                    break
                with self.subTest():
                    result, str_output = run_server_test_case(test_id, server_test_case)
                    self.assertTrue(result, msg=str_output)
                    server_test_cases[test_id] = server_test_case
                    if result:
                        print(f"Test Case {test_id}: PASS")
                    else:
                        print()

            print("Cleaning up test program...")
            OUTPUT_FILE_CONTENT[idx] = (client_test_cases, server_test_cases)
            print("Writing output files...")
            write_output_files(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_OUTPUT_DIR}", True)
            print("Killing fims_listen on client...")
            self.__class__.fims_listen_client.join(1, subprocess.run,
            ["docker", "exec", CLIENT_CONTAINER, "sh", "-c", "fims_send -m set -u /some"])
            print("Killing fims_listen on server...")
            self.__class__.fims_listen_server.join(1, subprocess.run,
            ["docker", "exec", SERVER_CONTAINER, "sh", "-c", "fims_send -m set -u /some"])
            kill_containers(self.__class__.client_container, self.__class__.server_container)
            self.__class__.client_container = None
            self.__class__.server_containter = None
            self.__class__.fims_listen_client = None
            self.__class__.fims_listen_server = None

        write_output_files(f"{LOCAL_PYTHON_SCRIPT_DIR}/{TEST_OUTPUT_DIR}", True)

if __name__ == '__main__':
    unittest.main(exit=True)
