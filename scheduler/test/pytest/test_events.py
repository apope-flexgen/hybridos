from fims import send_set, send_get, send_post, send_del
# need to import set_up since it is used as a test fixture. Pylance seemingly unable to detect that it is being used as a fixture
from fixtures import set_up
from sample_events import event_0, event_1, event_2
from sample_cfgs import fleet_scheduler_cfg
from time import sleep

# schedule map


def test_set_events():
    assert [] == send_del('/scheduler/events/durham')
    result = send_set('/scheduler/events', {'durham': [event_0, event_1]})
    event_0['id'] = result['durham'][0]['id']
    event_1['id'] = result['durham'][1]['id']
    event_0['repeat']['id'] = result['durham'][0]['repeat']['id']
    event_1['repeat']['id'] = result['durham'][1]['repeat']['id']
    assert result == {'durham': [event_0, event_1]}
    assert {'durham': [event_0, event_1]} == send_get('/scheduler/events')

# schedule


def test_del_events_schedule():
    assert [] == send_del('/scheduler/events/durham')
    assert [] == send_get('/scheduler/events/durham')


def test_post_events_schedule():
    result = send_post('/scheduler/events/durham', dict(sorted(event_2.items())))
    event_2['id'] = result[2]['id']
    event_2['repeat']['id'] = result[2]['repeat']['id']
    assert result == [event_0, event_1, event_2]
    assert [event_0, event_1, event_2] == send_get('/scheduler/events/durham')


def test_disallow_post_events_schedule_without_time_zone():
    # configure as Fleet Manager and add new client with bogus IP so we know it will not connect
    assert fleet_scheduler_cfg == send_set('/scheduler/configuration', fleet_scheduler_cfg)
    expected_clients = send_get('/scheduler/configuration/web_sockets/clients')
    client = { 'name': 'Chapel Hill', 'ip': '1.1.1.1', 'port': 9000 }
    result = send_post('/scheduler/configuration/web_sockets/clients', client)
    client['id'] = 'chapel_hill'
    expected_clients.append(client)
    assert expected_clients == result
    # test that schedule without initial connection cannot handle POST
    assert { 'error': 'Invalid Data' } == send_post('/scheduler/events/chapel_hill', event_2)


def test_disallow_post_overlapping_event():
    overlapping_event = {
        'start_time': '2025-03-13T08:00:00-04:00',  # Wednesday
        'duration': 90, 'mode': 'target_soc', 'variables': { 'soc_target': 50 },
    }
    assert { 'error': 'Overlapping Events' } == send_post('/scheduler/events/durham', overlapping_event)


def test_disallow_post_past_event():
    past_event = {
        'start_time': '2020-03-13T08:00:00-04:00',
        'duration': 90, 'mode': 'target_soc', 'variables': { 'soc_target': 50 },
    }
    assert { 'error': 'Event Starts In Past' } == send_post('/scheduler/events/durham', past_event)


def test_get_nonexistent_schedule():
    assert { 'error': 'Resource Not Found' } == send_get('/scheduler/events/charlotte')


def test_first_event_exception():
    # Tests adding a new event that has the same start time as an already-existing event,
    # but the already existing event has an exception so the new event should be accepted.
    shared_start_time = event_1['start_time']
    saved_event_1_repeat_exceptions = event_1['repeat']['exceptions']
    event_1['repeat']['exceptions'] = [shared_start_time]
    assert [shared_start_time] == send_set(f'/scheduler/events/durham/{event_1["id"]}/repeat/exceptions', [shared_start_time])
    new_event = {
        'start_time': shared_start_time,
        'duration': 90, 'mode': 'target_soc',
        'repeat': {'cycle': 'day', 'day_mask': 127, 'end_count': 1, 'end_time': '2000-01-01T00:00:00Z', 'exceptions': [], 'frequency': 1},
        'variables': {'soc_target': 50}
    }
    result = send_post('/scheduler/events/durham', new_event)
    new_event['id'] = result[1]['id']
    new_event['repeat']['id'] = result[1]['repeat']['id']
    assert [event_0, new_event, event_1] == result
    event_1['repeat']['exceptions'] = saved_event_1_repeat_exceptions


def test_set_events_schedule():
    assert [] == send_del('/scheduler/events/durham')
    result = send_set('/scheduler/events/durham', [event_0, event_1])
    event_0['id'] = result[0]['id']
    event_1['id'] = result[1]['id']
    event_0['repeat']['id'] = result[0]['repeat']['id']
    event_1['repeat']['id'] = result[1]['repeat']['id']
    assert result == [event_0, event_1]
    assert [event_0, event_1] == send_get('/scheduler/events/durham')

def test_disallow_set_events_schedule_without_time_zone():
    # configure as Fleet Manager and add new client with bogus IP so we know it will not connect
    assert fleet_scheduler_cfg == send_set('/scheduler/configuration', fleet_scheduler_cfg)
    expected_clients = send_get('/scheduler/configuration/web_sockets/clients')
    client = { 'name': 'Chapel Hill', 'ip': '1.1.1.1', 'port': 9000 }
    result = send_post('/scheduler/configuration/web_sockets/clients', client)
    client['id'] = 'chapel_hill'
    expected_clients.append(client)
    assert expected_clients == result
    # test that schedule without initial connection cannot handle SET
    assert { 'error': 'Invalid Data' } == send_set('/scheduler/events/chapel_hill', [event_0, event_1])
    

# individual event


def test_del_individual_event():
    assert [event_1] == send_del(f'/scheduler/events/durham/{event_0["id"]}')
    assert event_1 == send_get('/scheduler/events/durham/{}'.format(event_1['id']))


def test_set_individual_event():
    assert [event_1] == send_del(f'/scheduler/events/durham/{event_0["id"]}')
    result = send_set(f'/scheduler/events/durham/{event_1["id"]}', event_0)
    event_0['id'] = event_1['id']
    # this assertion also verifies that a SET to a specific event, ex: event 123, that has
    # a DIFFERENT ID in the body, ex: 321, than in the URI will keep the ID as 123. clients
    # cannot change event IDs. only back end can generate IDs
    assert result == event_0
    assert event_0 == send_get('/scheduler/events/durham/{}'.format(event_0['id']))


def test_set_individual_event_without_id():
    """
        This test is specifically to make sure that if a SET to a specific event does not include the ID in the body,
        the back end will preserve the existing ID while also applying the new event settings.
    """
    assert [event_1] == send_del(f'/scheduler/events/durham/{event_0["id"]}')
    event_0.pop('id')
    result = send_set(f'/scheduler/events/durham/{event_1["id"]}', event_0)
    event_0['id'] = event_1['id']
    assert result == event_0
    assert event_0 == send_get('/scheduler/events/durham/{}'.format(event_0['id']))


def test_disallow_set_individual_event_to_past():
    past_event = {
        'start_time': '2020-03-13T08:00:00-04:00',
        'duration': 90, 'mode': 'target_soc', 'variables': { 'soc_target': 50 },
    }
    assert { 'error': 'Invalid Data' } == send_set(f'/scheduler/events/durham/{event_0["id"]}', past_event)


# individual event fields


def test_get_event_mode():
    # only GET that is not also tested in a SET test
    assert event_1['mode'] == send_get(f'/scheduler/events/durham/{event_1["id"]}/mode')


def test_set_event_start_time():
    # this also tests that the exception's time is updated
    new_start_time = '2025-04-12T08:00:00-04:00'  # Friday
    new_exception = '2025-04-16T08:00:00-04:00'
    assert new_start_time == send_set(f'/scheduler/events/durham/{event_1["id"]}/start_time', new_start_time)
    assert new_start_time == send_get(f'/scheduler/events/durham/{event_1["id"]}/start_time')
    assert new_exception == send_get(f'/scheduler/events/durham/{event_1["id"]}/repeat/exceptions')[0]


def test_disallow_set_event_start_time_to_past():
    past_start_time = '2020-03-13T08:00:00-04:00'
    assert { 'error': 'Invalid Data' } == send_set(f'/scheduler/events/durham/{event_0["id"]}/start_time', past_start_time)


def test_set_event_duration():
    new_duration = 60
    assert new_duration == send_set(f'/scheduler/events/durham/{event_1["id"]}/duration', new_duration)
    assert new_duration == send_get(f'/scheduler/events/durham/{event_1["id"]}/duration')


def test_set_event_variables_map():
    new_variables_map = {'soc_target': 10}
    assert new_variables_map == send_set('/scheduler/events/durham/{}/variables'.format(event_0['id']), new_variables_map)
    assert new_variables_map == send_get('/scheduler/events/durham/{}/variables'.format(event_0['id']))


def test_set_event_variable_value():
    new_variable_value = -5
    assert new_variable_value == send_set('/scheduler/events/durham/{}/variables/soc_target'.format(event_0['id']), new_variable_value)
    assert new_variable_value == send_get('/scheduler/events/durham/{}/variables/soc_target'.format(event_0['id']))


def test_set_event_repeat_object():
    new_repeat_object = {
        'cycle': 'week',
        'day_mask': 65,  # Sundays & Saturdays
        'end_time': '2025-04-27T09:00:00-04:00',  # Saturday
        'exceptions': ['2025-04-20T09:00:00-04:00'],
        'frequency': 1,
        'id': event_1['repeat']['id']
    }
    result = send_set(f'/scheduler/events/durham/{event_1["id"]}/repeat', new_repeat_object)
    # sneak this in after sending SET to ensure Scheduler is resetting end_count to 0 when given repeat object has valid end_time
    new_repeat_object['end_count'] = 0
    assert result == new_repeat_object
    assert new_repeat_object == send_get(f'/scheduler/events/durham/{event_1["id"]}/repeat')

# repeat object fields


def test_set_event_repeat_cycle():
    new_repeat_cycle = 'week'
    assert new_repeat_cycle == send_set(f'/scheduler/events/durham/{event_1["id"]}/repeat/cycle', new_repeat_cycle)
    assert new_repeat_cycle == send_get(f'/scheduler/events/durham/{event_1["id"]}/repeat/cycle')


def test_set_event_repeat_day_mask():
    new_repeat_day_mask = 3
    assert new_repeat_day_mask == send_set(f'/scheduler/events/durham/{event_1["id"]}/repeat/day_mask', new_repeat_day_mask)
    assert new_repeat_day_mask == send_get(f'/scheduler/events/durham/{event_1["id"]}/repeat/day_mask')


def test_set_event_repeat_end_time():
    new_repeat_end_time = '2025-05-04T09:00:00-04:00'  # Saturday
    assert new_repeat_end_time == send_set(f'/scheduler/events/durham/{event_1["id"]}/repeat/end_time', new_repeat_end_time)
    assert new_repeat_end_time == send_get(f'/scheduler/events/durham/{event_1["id"]}/repeat/end_time')
    assert 0 == send_get(f'/scheduler/events/durham/{event_1["id"]}/repeat/end_count')


def test_set_event_repeat_end_count():
    new_repeat_end_count = 25
    assert new_repeat_end_count == send_set(f'/scheduler/events/durham/{event_1["id"]}/repeat/end_count', new_repeat_end_count)
    assert new_repeat_end_count == send_get(f'/scheduler/events/durham/{event_1["id"]}/repeat/end_count')
    assert '2000-01-01T00:00:00Z' == send_get(f'/scheduler/events/durham/{event_1["id"]}/repeat/end_time')


def test_set_event_repeat_exceptions():
    new_repeat_exceptions = ['2025-04-20T09:00:00-04:00', '2025-04-27T09:00:00-04:00']
    assert new_repeat_exceptions == send_set(f'/scheduler/events/durham/{event_1["id"]}/repeat/exceptions', new_repeat_exceptions)
    assert new_repeat_exceptions == send_get(f'/scheduler/events/durham/{event_1["id"]}/repeat/exceptions')


def test_set_event_repeat_frequency():
    new_repeat_frequency = 2
    assert new_repeat_frequency == send_set(f'/scheduler/events/durham/{event_1["id"]}/repeat/frequency', new_repeat_frequency)
    assert new_repeat_frequency == send_get(f'/scheduler/events/durham/{event_1["id"]}/repeat/frequency')

# exceptions array POSTs/DELs


def test_post_event_repeat_exception():
    new_exception = '2025-05-05T09:00:00-04:00'
    current_exceptions = send_get(f'/scheduler/events/durham/{event_1["id"]}/repeat/exceptions')
    current_exceptions.append(new_exception)
    assert current_exceptions == send_post(f'/scheduler/events/durham/{event_1["id"]}/repeat/exceptions', new_exception)
    assert current_exceptions == send_get(f'/scheduler/events/durham/{event_1["id"]}/repeat/exceptions')


def test_delete_event_repeat_specific_exception():
    exception_to_delete = '2025-04-16T09:00:00-04:00'
    # set up this test by verifying the targeted exception is indeed in the existing exceptions array
    current_exceptions = send_get(f'/scheduler/events/durham/{event_1["id"]}/repeat/exceptions')
    index_to_delete = current_exceptions.index(exception_to_delete)
    current_exceptions.pop(index_to_delete)
    # execute test
    result = send_del(f'/scheduler/events/durham/{event_1["id"]}/repeat/exceptions/{index_to_delete}')
    assert result == current_exceptions


def test_delete_event_repeat_all_exceptions():
    assert [] == send_del(f'/scheduler/events/durham/{event_1["id"]}/repeat/exceptions')
    assert [] == send_get(f'/scheduler/events/durham/{event_1["id"]}/repeat/exceptions')


def test_update_current_event():
    """ creates an event at current time + 1 min 
    waits for the event to begin. Then tries to update
    fields that should be updateable """

    assert [] == send_del('/scheduler/events/durham')
    from datetime import datetime, timezone, timedelta

    def create_event_starting_in_one_minute():
        """ YOU SHOULD PROBABLY SYNC YOUR CLOCK """
        # get current time
        # Get the current time in UTC
        current_time_utc = datetime.now(timezone.utc)
        # Convert the current time to Eastern Standard Time (EST) by adding a timedelta of -5 hours
        current_time_est = current_time_utc + timedelta(hours=-5)
        # Format the time in ISO 8601 format with timezone information
        current_time_est_isoformat = current_time_est.strftime('%Y-%m-%dT%H:%M:%S-05:00')
        event = {
            'start_time': current_time_est_isoformat,
            'duration': 90,
            'mode': 'target_soc',
            'variables': {
                'soc_target': 50
                },
            'repeat': {
                'cycle': 'day',
                'day_mask': 127,
                'end_count': 1,
                'end_time': '2000-01-01T00:00:00Z',
                'exceptions': [],
                'frequency': 1
                }
            }
        return event

    event = create_event_starting_in_one_minute()
    result = send_set('/scheduler/events', {'durham': [event]})
    assert result['durham'][0]['variables'] == event['variables']
    assert result['durham'][0]['duration'] == event['duration']
    sleep(90)
    event_get = send_get('/scheduler/events/durham')
    event_id = str(event_get[0]['id'])
    set_string = '/scheduler/events/durham/' + event_id + '/variables/soc_target'
    _ = send_set(set_string, 100)
    event_get = send_get('/scheduler/events/durham/' + event_id)
    assert event_get['variables']['soc_target'] == 100
    _ = send_set('/scheduler/events/durham/' + event_id + '/variables/soc_target', 10)
    event_get = send_get('/scheduler/events/durham/' + event_id)
    assert event_get['variables']['soc_target'] == 10

    _ = send_set('/scheduler/events/durham/' + event_id + '/duration', 100)
    event_get = send_get('/scheduler/events/durham/' + event_id)
    assert event_get['duration'] == 100
    _ = send_set('/scheduler/events/durham/' + event_id + '/duration', 10)
    event_get = send_get('/scheduler/events/durham/' + event_id)
    assert event_get['duration'] == 10
