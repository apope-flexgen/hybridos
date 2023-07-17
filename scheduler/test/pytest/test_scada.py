from fims import send_set, send_get
# need to import set_up since it is used as a test fixture. Pylance seemingly unable to detect that it is being used as a fixture
from fixtures import set_up

def stage_event(stage_index: int, year: int, month: int, day: int, hour: int, minute: int, duration: int, repeat_daily: int, site: int, mode: int, variables: dict) -> None:
    assert year == send_set(f'/scheduler/scada/write/event_{stage_index}/year', year)
    assert year == send_get(f'/scheduler/scada/write/event_{stage_index}/year')

    assert month == send_set(f'/scheduler/scada/write/event_{stage_index}/month', month)
    assert month == send_get(f'/scheduler/scada/write/event_{stage_index}/month')

    assert day == send_set(f'/scheduler/scada/write/event_{stage_index}/day', day)
    assert day == send_get(f'/scheduler/scada/write/event_{stage_index}/day')

    assert hour == send_set(f'/scheduler/scada/write/event_{stage_index}/hour', hour)
    assert hour == send_get(f'/scheduler/scada/write/event_{stage_index}/hour')

    assert minute == send_set(f'/scheduler/scada/write/event_{stage_index}/minute', minute)
    assert minute == send_get(f'/scheduler/scada/write/event_{stage_index}/minute')

    assert duration == send_set(f'/scheduler/scada/write/event_{stage_index}/duration', duration)
    assert duration == send_get(f'/scheduler/scada/write/event_{stage_index}/duration')

    assert repeat_daily == send_set(f'/scheduler/scada/write/event_{stage_index}/repeat_daily', repeat_daily)
    assert repeat_daily == send_get(f'/scheduler/scada/write/event_{stage_index}/repeat_daily')

    assert site == send_set(f'/scheduler/scada/write/event_{stage_index}/site', site)
    assert site == send_get(f'/scheduler/scada/write/event_{stage_index}/site')

    assert mode == send_set(f'/scheduler/scada/write/event_{stage_index}/mode', mode)
    assert mode == send_get(f'/scheduler/scada/write/event_{stage_index}/mode')

    for var_id in variables:
        assert variables[var_id] == send_set(f'/scheduler/scada/write/event_{stage_index}/{var_id}', variables[var_id])
        assert variables[var_id] == send_get(f'/scheduler/scada/write/event_{stage_index}/{var_id}')

def assert_event_read(event_index: int, year: int, month: int, day: int, hour: int, minute: int, duration: int, repeat_daily: int, site: int, mode: int, variables: dict) -> None:
    assert year == send_get(f'/scheduler/scada/read/event_{event_index}/year')
    assert month == send_get(f'/scheduler/scada/read/event_{event_index}/month')
    assert day == send_get(f'/scheduler/scada/read/event_{event_index}/day')
    assert hour == send_get(f'/scheduler/scada/read/event_{event_index}/hour')
    assert minute == send_get(f'/scheduler/scada/read/event_{event_index}/minute')
    assert duration == send_get(f'/scheduler/scada/read/event_{event_index}/duration')
    assert repeat_daily == send_get(f'/scheduler/scada/read/event_{event_index}/repeat_daily')
    assert site == send_get(f'/scheduler/scada/read/event_{event_index}/site')
    assert mode == send_get(f'/scheduler/scada/read/event_{event_index}/mode')
    for var_id in variables:
        assert variables[var_id] == send_get(f'/scheduler/scada/read/event_{event_index}/{var_id}')

def test_scada():
    """
        Stages a daily-repeating event and tests that the Overwrite command works.
        Stages a one-off event and tests that the Append command works.
    """
    # mode=0 translates to Absolute ESS mode which has one float and one bool
    stage_event(stage_index=0, year=2024, month=3, day=20, hour=12, minute=0, duration=60, repeat_daily=1, site=0, mode=0, variables={'float_1': 1000, 'bool_1': 0})
    # overwrite with Command 2 to replace existing schedule
    assert "Success" == send_set('/scheduler/scada/write/command', 2)
    assert_event_read(event_index=0, year=2024, month=3, day=20, hour=12, minute=0, duration=60, repeat_daily=1, site=0, mode=0, variables={'float_1': 1000, 'bool_1': 0})

    # now append with Command 1 to add to existing schedule
    stage_event(stage_index=1, year=2024, month=3, day=21, hour=20, minute=0, duration=60, repeat_daily=0, site=0, mode=0, variables={'float_1': 1000, 'bool_1': 1})
    assert "Success" == send_set('/scheduler/scada/write/command', 1)
    assert_event_read(event_index=0, year=2024, month=3, day=20, hour=12, minute=0, duration=60, repeat_daily=1, site=0, mode=0, variables={'float_1': 1000, 'bool_1': 0})
    assert_event_read(event_index=1, year=2024, month=3, day=21, hour=20, minute=0, duration=60, repeat_daily=0, site=0, mode=0, variables={'float_1': 1000, 'bool_1': 1})

def test_scada_boolean_representation():
    """
        SCADA system users like when all data is represented by integers, so boolean values are represented by 1s and 0s.
        In SETs, any non-zero values are interpreted as true and are standardized to 1. This pytest ensures the 1s and 0s
        representation is properly followed, including the standardization of non-zero values to 1.
    """

    # make sure all non-zero numbers are standardized to 1 for repeat_daily
    for integer in [-2, -1, 1, 2]:
        assert 1 == send_set(f'/scheduler/scada/write/event_{0}/repeat_daily', integer)
        assert 1 == send_get(f'/scheduler/scada/write/event_{0}/repeat_daily')
    
    # make sure zero is NOT standardized to 1 for repeat_daily
    assert 0 == send_set(f'/scheduler/scada/write/event_{0}/repeat_daily', 0)
    assert 0 == send_get(f'/scheduler/scada/write/event_{0}/repeat_daily')

    # make sure all non-zero numbers are standardized to 1 for bool_1
    for integer in [-2, -1, 1, 2]:
        assert 1 == send_set(f'/scheduler/scada/write/event_{0}/bool_1', integer)
        assert 1 == send_get(f'/scheduler/scada/write/event_{0}/bool_1')
    
    # make sure zero is NOT standardized to 1 for bool_1
    assert 0 == send_set(f'/scheduler/scada/write/event_{0}/bool_1', 0)
    assert 0 == send_get(f'/scheduler/scada/write/event_{0}/bool_1')
