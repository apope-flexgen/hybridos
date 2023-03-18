from http import server
import json
import os
import time
from subprocess import Popen, PIPE
from pathlib import Path

def create_client_directories(parent_directory, num_clients):
    clients = []
    for i in range(num_clients):
        client_directory_name = 'client{client_index:d}_files'.format(client_index = i+1)
        client_directory_path = parent_directory / client_directory_name
        clients.append(client_directory_path)
        clients[i].mkdir()
        print('client {client_index:d} directory: '.format(client_index = i+1) + str(client_directory_path))
    return clients

def create_server_directories(parent_directory, num_servers):
    servers = []
    for i in range(num_servers):
        server_directory_name = 'server{serv_index:d}_files'.format(serv_index = i+1)
        server_directory_path = parent_directory / server_directory_name
        servers.append(server_directory_path)
        servers[i].mkdir()
        print('server {serv_index:d} directory: '.format(serv_index = i+1) + str(server_directory_path))
    return servers

def create_config_file(file_path, data):
    json_string = json.dumps(data)
    with open(file_path, 'w') as outfile:
        outfile.write(json_string)
    print('config: ' + json_string)

def start_cloud_sync(config_path):
    cloud_sync_binary = Path('/usr/local/bin/cloud_sync')
    assert cloud_sync_binary.is_file()
    Popen(['sudo', cloud_sync_binary, config_path])

def wait_for_transfer_to_servers(servers, client_dirs):
    for _ in range(10):
        time.sleep(1)
        all_transfers_done = True
        for serv_dir in servers:
            for client in client_dirs:
                for client_file in os.listdir(client):
                    if client_file not in os.listdir(serv_dir):
                        all_transfers_done = False
        if all_transfers_done:
            break

# most basic test of cloud_sync copying a txt file from the client to local servers.
# DO NOT delete test data from client after transfer
def test_local_copy_no_delete(tmp_path):
    # SETUP
    num_clients = 2
    client_dirs = create_client_directories(tmp_path, num_clients)
    server_dirs = create_server_directories(tmp_path, 2)
    config_path = tmp_path / 'config'
    os.system('sudo rm -rf /home/.cloud_sync') # delete old metadata file so test starts fresh

    # TEST INPUT
    data = {
        'clients': [
            {
                'name': 'test-client-1',
                'path': str(client_dirs[0]),
                'extension': '.txt',
                'operation': 'source'
            },
            {
                'name': 'test-client-2',
                'path': str(client_dirs[1]),
                'extension': '.txt',
                'operation': 'source'
            }
        ],
        'deleteFile': False,
        'servers': [
            {
                'name': 'test-server-1',
                'ip': '0.0.0.0',
                'path': str(server_dirs[0]),
                'timer_s': 2
            },
            {
                'name': 'test-server-2',
                'ip': '0.0.0.0',
                'path': str(server_dirs[1]),
                'timer_s': 2
            }
        ]
    }
    create_config_file(config_path, data)

    # RUN CLOUD_SYNC
    start_cloud_sync(config_path)
    time.sleep(1)
    for i in range(num_clients):
        open(client_dirs[i] / 'test_data{file_index:d}.txt'.format(file_index = i+1), 'x') # creates the new file test_data.txt in the client directory
    wait_for_transfer_to_servers(server_dirs, client_dirs)
    os.system('sudo pkill cloud_sync')

    # RESULTS
    # test_data files, which were in client directories, must now be in all server directories
    for serv_dir in server_dirs:
        # directory file lists are not sorted, so check that every expected file is present
        assert all(elem in os.listdir(serv_dir) for elem in [ 'test_data1.txt', 'test_data2.txt' ])
    # files should not be deleted from client directories since 'deleteFile' was set to False in config
    for i, client_dir in enumerate(client_dirs):
        assert os.listdir(client_dir) == [ 'test_data{file_index:d}.txt'.format(file_index = i+1) ]

# most basic test of cloud_sync copying a txt file from the client to local servers.
# DO delete test data from client after transfer
def test_local_copy_with_delete(tmp_path):
    # SETUP
    num_clients = 2
    client_dirs = create_client_directories(tmp_path, num_clients)
    server_dirs = create_server_directories(tmp_path, 2)
    config_path = tmp_path / 'config'
    os.system('sudo rm -rf /home/.cloud_sync') # delete old metadata file so test starts fresh

    # TEST INPUT
    data = {
        'clients': [
            {
                'name': 'test-client-1',
                'path': str(client_dirs[0]),
                'extension': '.txt',
                'operation': 'source'
            },
            {
                'name': 'test-client-2',
                'path': str(client_dirs[1]),
                'extension': '.txt',
                'operation': 'source'
            }
        ],
        'deleteFile': True,
        'servers': [
            {
                'name': 'test-server-1',
                'ip': '0.0.0.0',
                'path': str(server_dirs[0]),
                'timer_s': 2
            },
            {
                'name': 'test-server-2',
                'ip': '0.0.0.0',
                'path': str(server_dirs[1]),
                'timer_s': 2
            }
        ]
    }
    create_config_file(config_path, data)

    # RUN CLOUD_SYNC
    start_cloud_sync(config_path)
    time.sleep(1)
    for i in range(num_clients):
        open(client_dirs[i] / 'test_data{file_index:d}.txt'.format(file_index = i+1), 'x') # creates the new file test_data.txt in the client directory
    wait_for_transfer_to_servers(server_dirs, client_dirs)
    os.system('sudo pkill cloud_sync')

    # RESULTS
    # test_data files, which was in client directories, must now be in all server directories
    for serv_dir in server_dirs:
        # directory file lists are not sorted, so check that every expected file is present
        assert all(elem in os.listdir(serv_dir) for elem in  [ 'test_data1.txt', 'test_data2.txt' ])
    # test files should be deleted from client directories, since 'deleteFile' was set to True in config
    for client_dir in client_dirs:
        assert len(os.listdir(client_dir)) == 0

# make new file while CloudSync is running (in addition to file already existing at startup) and make sure it gets copied
def test_intermediate_insert(tmp_path):
    # SETUP
    client_dirs = create_client_directories(tmp_path, 2)
    server_dirs = create_server_directories(tmp_path, 2)
    config_path = tmp_path / 'config'
    os.system('sudo rm -rf /home/.cloud_sync') # delete old metadata file so test starts fresh

    # TEST INPUT
    data = {
        'clients': [
            {
                'name': 'test-client-1',
                'path': str(client_dirs[0]),
                'extension': '.txt',
                'operation': 'source'
            },
            {
                'name': 'test-client-2',
                'path': str(client_dirs[1]),
                'extension': '.txt',
                'operation': 'source'
            }
        ],
        'deleteFile': False,
        'servers': [
            {
                'name': 'test-server-1',
                'ip': '0.0.0.0',
                'path': str(server_dirs[0]),
                'timer_s': 2
            },
            {
                'name': 'test-server-2',
                'ip': '0.0.0.0',
                'path': str(server_dirs[1]),
                'timer_s': 2
            }
        ]
    }
    create_config_file(config_path, data)

    # RUN CLOUD_SYNC
    start_cloud_sync(config_path)
    time.sleep(1)
    open(client_dirs[0] / 'test_data1.txt', 'x')
    wait_for_transfer_to_servers(server_dirs, client_dirs[:1])
    open(client_dirs[1] / 'test_data2.txt', 'x')
    wait_for_transfer_to_servers(server_dirs, client_dirs[1:])
    os.system('sudo pkill cloud_sync')

    # RESULTS
    # test_data files, which were in client directories, must now be in all server directories
    for serv_dir in server_dirs:
        print("got " + str(os.listdir(serv_dir)))
        # directory file lists are not sorted, so check that every expected file is present
        assert all(elem in os.listdir(serv_dir) for elem in [ 'test_data1.txt', 'test_data2.txt' ])
    # file should not be deleted from client directories since 'deleteFile' was set to False in config
    for i, client_dir in enumerate(client_dirs):
        assert os.listdir(client_dir) == [ 'test_data{file_index:d}.txt'.format(file_index = i+1) ]

