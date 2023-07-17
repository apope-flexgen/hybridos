from fims import send_set, send_get, send_post, send_del
# need to import set_up since it is used as a test fixture. Pylance seemingly unable to detect that it is being used as a fixture
from fixtures import set_up
from sample_cfgs import site_scheduler_cfg, fleet_scheduler_cfg
from sample_events import event_0, event_1
from sample_modes import modes_cfg
import subprocess

# highest-level GET on /scheduler


def test_get_scheduler():
    response = send_get('/scheduler')
    assert response['configuration'] == site_scheduler_cfg
    assert response['timezones'] == { 'durham': 'America/New_York' }
    assert 'last_schedule_modification' in response
    assert response['modes'] == modes_cfg
    event_0['id'] = response['events']['durham'][0]['id']
    event_1['id'] = response['events']['durham'][1]['id']
    event_0['repeat']['id'] = response['events']['durham'][0]['repeat']['id']
    event_1['repeat']['id'] = response['events']['durham'][1]['repeat']['id']
    assert response['events'] == {'durham': [event_0, event_1]}

# high-level configuration


def test_set_fleet_scheduler_configuration():
    assert fleet_scheduler_cfg == send_set('/scheduler/configuration', fleet_scheduler_cfg)
    assert fleet_scheduler_cfg == send_get('/scheduler/configuration')


def test_set_site_scheduler_configuration():
    assert site_scheduler_cfg == send_set('/scheduler/configuration', site_scheduler_cfg)
    assert site_scheduler_cfg == send_get('/scheduler/configuration')

# local schedule configuration


def test_set_local_schedule_cfg():
    cfg = {
        'id': 'raleigh',
        'name': 'Raleigh',
        'clothed_setpoints': True,
        'setpoint_enforcement': {
            'enabled': True,
            'frequency_seconds': 10
        }
    }
    assert cfg == send_set('/scheduler/configuration/local_schedule', cfg)
    assert cfg == send_get('/scheduler/configuration/local_schedule')


def test_set_local_schedule_id():
    id = 'city_of_bulls'
    assert id == send_set('/scheduler/configuration/local_schedule/id', id)
    assert id == send_get('/scheduler/configuration/local_schedule/id')


def test_set_local_schedule_name():
    name = 'Bull City'
    assert name == send_set('/scheduler/configuration/local_schedule/name', name)
    assert name == send_get('/scheduler/configuration/local_schedule/name')


def test_set_local_schedule_clothed_setpoints():
    clothed_setpoints = True
    assert clothed_setpoints == send_set('/scheduler/configuration/local_schedule/clothed_setpoints', clothed_setpoints)
    assert clothed_setpoints == send_get('/scheduler/configuration/local_schedule/clothed_setpoints')


def test_set_local_schedule_setpoint_enforcement():
    setpoint_enforcement = {
        'enabled': True,
        'frequency_seconds': 360000
    }
    assert setpoint_enforcement == send_set('/scheduler/configuration/local_schedule/setpoint_enforcement', setpoint_enforcement)
    assert setpoint_enforcement == send_get('/scheduler/configuration/local_schedule/setpoint_enforcement')


def test_set_local_schedule_setpoint_enforcement_enabled():
    enabled = True
    assert enabled == send_set('/scheduler/configuration/local_schedule/setpoint_enforcement/enabled', enabled)
    assert enabled == send_get('/scheduler/configuration/local_schedule/setpoint_enforcement/enabled')


def test_set_local_schedule_setpoint_enforcement_frequency_seconds():
    frequency_seconds = 60
    assert frequency_seconds == send_set('/scheduler/configuration/local_schedule/setpoint_enforcement/frequency_seconds', frequency_seconds)
    assert frequency_seconds == send_get('/scheduler/configuration/local_schedule/setpoint_enforcement/frequency_seconds')


def test_set_local_schedule_pub():
    clothed_setpoints = True
    
    # Begin listening for the first pub message on /scheduler/configuration URI
    fims_listen_1 = subprocess.Popen(['fims_listen', '-m', 'pub', '-u', '/scheduler/configuration', '-n', '1'],
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    
    # Send set message to /scheduler/configuration/local_schedule/clothed_setpoints to generate pub message
    assert clothed_setpoints == send_set('/scheduler/configuration/local_schedule/clothed_setpoints', clothed_setpoints)
    
    # Wait for fims_listen command to terminate | If timeout occurs then the publish did not happen
    fims_listen_1.wait(1)

# WebSocket configuration


def test_set_web_sockets():
    web_sockets = {
        'server': {
            'enabled': True,
            'port': 10100
        }
    }
    assert web_sockets == send_set('/scheduler/configuration/web_sockets', web_sockets)
    assert web_sockets == send_get('/scheduler/configuration/web_sockets')


def test_get_web_socket_server_of_fleet_scheduler():
    assert fleet_scheduler_cfg == send_set('/scheduler/configuration', fleet_scheduler_cfg)
    assert { 'error': 'Invalid URI' } == send_get('/scheduler/configuration/web_sockets/server')


def test_get_web_socket_clients_of_site_scheduler():
    assert { 'error': 'Invalid URI' } == send_get('/scheduler/configuration/web_sockets/clients')


def test_set_web_socket_server():
    server = {
        'enabled': True,
        'port': 666
    }
    assert server == send_set('/scheduler/configuration/web_sockets/server', server)
    assert server == send_get('/scheduler/configuration/web_sockets/server')


def test_set_web_socket_server_enabled():
    enabled = True
    assert enabled == send_set('/scheduler/configuration/web_sockets/server/enabled', enabled)
    assert enabled == send_get('/scheduler/configuration/web_sockets/server/enabled')


def test_set_web_socket_server_port():
    port = 555123
    assert port == send_set('/scheduler/configuration/web_sockets/server/port', port)
    assert port == send_get('/scheduler/configuration/web_sockets/server/port')


def test_set_web_socket_clients():
    assert fleet_scheduler_cfg == send_set('/scheduler/configuration', fleet_scheduler_cfg)
    clients = [
        {
            'name': 'Raleigh',
            'ip': '172.16.1.80',
            'port': 9000
        },
        {
            'name': 'Durham/Chapel Hill',
            'ip': '86.75.30.9',
            'port': 9000
        }
    ]
    result = send_set('/scheduler/configuration/web_sockets/clients', clients)
    # id field should be automatically generated by back end.
    # slashes and spaces get replaced with underscores, and all letters are made lower-case
    clients[0]['id'] = 'raleigh'
    clients[1]['id'] = 'durham_chapel_hill'
    assert clients == result
    assert clients == send_get('/scheduler/configuration/web_sockets/clients')


def test_set_web_socket_individual_client():
    assert fleet_scheduler_cfg == send_set('/scheduler/configuration', fleet_scheduler_cfg)
    client = {
        'id': 'chapel_hill',
        'name': 'Chapel Hill',
        'ip': '192.1.1.80',
        'port': 9001
    }
    result = send_set('/scheduler/configuration/web_sockets/clients/durham', client)
    # SET to a client by its ID should not be allowed to change the ID
    client['id'] = 'durham'
    assert client == result
    assert client == send_get('/scheduler/configuration/web_sockets/clients/durham')


def test_set_web_socket_client_name():
    assert fleet_scheduler_cfg == send_set('/scheduler/configuration', fleet_scheduler_cfg)
    name = 'Bull Durham'
    assert name == send_set('/scheduler/configuration/web_sockets/clients/durham/name', name)
    assert name == send_get('/scheduler/configuration/web_sockets/clients/durham/name')


def test_set_web_socket_client_ip():
    assert fleet_scheduler_cfg == send_set('/scheduler/configuration', fleet_scheduler_cfg)
    ip = '80.80.80.80'
    assert ip == send_set('/scheduler/configuration/web_sockets/clients/durham/ip', ip)
    assert ip == send_get('/scheduler/configuration/web_sockets/clients/durham/ip')


def test_set_web_socket_clients_port():
    assert fleet_scheduler_cfg == send_set('/scheduler/configuration', fleet_scheduler_cfg)
    port = 10000
    assert port == send_set('/scheduler/configuration/web_sockets/clients/durham/port', port)
    assert port == send_get('/scheduler/configuration/web_sockets/clients/durham/port')


def test_post_web_socket_client():
    assert fleet_scheduler_cfg == send_set('/scheduler/configuration', fleet_scheduler_cfg)
    client = {
        'name': 'Chapel Hill',
        'ip': '192.1.1.80',
        'port': 9000
    }
    expected_clients = send_get('/scheduler/configuration/web_sockets/clients')
    result = send_post('/scheduler/configuration/web_sockets/clients', client)
    # id field should be automatically generated by back end.
    # slashes and spaces get replaced with underscores, and all letters are made lower-case
    client['id'] = 'chapel_hill'
    expected_clients.append(client)
    assert expected_clients == result
    assert expected_clients == send_get('/scheduler/configuration/web_sockets/clients')


def test_del_web_socket_clients():
    assert fleet_scheduler_cfg == send_set('/scheduler/configuration', fleet_scheduler_cfg)
    assert [] == send_del('/scheduler/configuration/web_sockets/clients')
    assert [] == send_get('/scheduler/configuration/web_sockets/clients')


def test_del_web_socket_individual_client():
    assert fleet_scheduler_cfg == send_set('/scheduler/configuration', fleet_scheduler_cfg)
    assert [] == send_del('/scheduler/configuration/web_sockets/clients/durham')
    assert [] == send_get('/scheduler/configuration/web_sockets/clients')

def test_set_web_socket_server_pub():
    server_enable = True
    
    # Begin listening for the first pub message on /scheduler/configuration URI
    fims_listen_1 = subprocess.Popen(['fims_listen', '-m', 'pub', '-u', '/scheduler/configuration', '-n', '1'],
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    
    # Send set message to /scheduler/configuration/web_sockets/server/enabled to generate pub message
    assert server_enable == send_set('/scheduler/configuration/web_sockets/server/enabled', server_enable)
    
    # Wait for fims_listen command to terminate | If timeout occurs then the publish did not happen
    fims_listen_1.wait(1)

def test_set_web_socket_clients_pub():
    ip_address = "172.16.1.80"
    # Begin listening for the first two pub messages on /scheduler/configuration URI 
    # First one is publishing change to fleet_manager | Second publish message is actual test
    fims_listen_1 = subprocess.Popen(['fims_listen', '-m', 'pub', '-u', '/scheduler/configuration', '-n', '2'],
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    
    # Send set message to /scheduler/configuration/web_sockets/clients to generate pub message
    # First set message is to trick scheduler into thinking it is a fleet_manager and not a site_manager
    assert fleet_scheduler_cfg == send_set('/scheduler/configuration', fleet_scheduler_cfg)
    assert ip_address == send_set('/scheduler/configuration/web_sockets/clients/durham/ip', ip_address)
    
    # Wait for fims_listen command to terminate | If timeout occurs then the publish did not happen
    fims_listen_1.wait(1)

# SCADA configuration


def test_set_scada_cfg():
    cfg = {
        'stage_size': 1,
        'max_num_events': 100,
        'num_floats': 2,
        'num_ints': 0,
        'num_bools': 1,
        'num_strings': 0
    }
    assert cfg == send_set('/scheduler/configuration/scada', cfg)
    assert cfg == send_get('/scheduler/configuration/scada')


def test_set_scada_stage_size():
    stage_size = 25
    assert stage_size == send_set('/scheduler/configuration/scada/stage_size', stage_size)
    assert stage_size == send_get('/scheduler/configuration/scada/stage_size')


def test_set_scada_max_num_events():
    max_num_events = 42
    assert max_num_events == send_set('/scheduler/configuration/scada/max_num_events', max_num_events)
    assert max_num_events == send_get('/scheduler/configuration/scada/max_num_events')


def test_set_scada_num_floats():
    num_floats = 69
    assert num_floats == send_set('/scheduler/configuration/scada/num_floats', num_floats)
    assert num_floats == send_get('/scheduler/configuration/scada/num_floats')


def test_set_scada_num_ints():
    num_ints = 420
    assert num_ints == send_set('/scheduler/configuration/scada/num_ints', num_ints)
    assert num_ints == send_get('/scheduler/configuration/scada/num_ints')


def test_set_scada_num_bools():
    num_bools = 100
    assert num_bools == send_set('/scheduler/configuration/scada/num_bools', num_bools)
    assert num_bools == send_get('/scheduler/configuration/scada/num_bools')


def test_set_scada_num_strings():
    num_strings = 1
    assert num_strings == send_set('/scheduler/configuration/scada/num_strings', num_strings)
    assert num_strings == send_get('/scheduler/configuration/scada/num_strings')

def test_set_scada_pub():
    max_num_events = 100
    
    # Begin listening for the first pub message on /scheduler/configuration URI
    fims_listen_1 = subprocess.Popen(['fims_listen', '-m', 'pub', '-u', '/scheduler/configuration', '-n', '1'],
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    
    # Send set message to /scheduler/configuration/scada/max_num_events to generate pub message
    assert max_num_events == send_set('/scheduler/configuration/scada/max_num_events', max_num_events)
    
    # Wait for fims_listen command to terminate | If timeout occurs then the publish did not happen
    fims_listen_1.wait(1)

def test_set_setpoint_enforcement_pub():
    frequency_seconds = 20
    
    # Begin listening for the first pub message on /scheduler/configuration URI
    fims_listen_1 = subprocess.Popen(['fims_listen', '-m', 'pub', '-u', '/scheduler/configuration', '-n', '1'],
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    
    # Send set message to /scheduler/configuration/local_schedule/setpoint_enforcement/frequency_seconds to generate pub message
    assert frequency_seconds == send_set('/scheduler/configuration/local_schedule/setpoint_enforcement/frequency_seconds', frequency_seconds)
    
    # Wait for fims_listen command to terminate | If timeout occurs then the publish did not happen
    fims_listen_1.wait(1)
