from fims import send_set
import pytest
from sample_cfgs import site_scheduler_cfg
from sample_modes import modes_cfg
from sample_events import event_0, event_1


def init_cfg():
    assert site_scheduler_cfg == send_set('/scheduler/configuration', site_scheduler_cfg)


def init_modes():
    assert modes_cfg == send_set('/scheduler/modes', modes_cfg)


def init_events():
    result = send_set('/scheduler/events/durham', [event_0, event_1])
    event_0['id'] = result[0]['id']
    event_1['id'] = result[1]['id']
    event_0['repeat']['id'] = result[0]['repeat']['id']
    event_1['repeat']['id'] = result[1]['repeat']['id']
    assert result == [event_0, event_1]


@pytest.fixture(autouse=True)
def set_up():
    """
        Due to @pytest.fixture(autouse=True), this function will automatically run before every pytest.
        It resets Scheduler to be in the same state every time so each pytest does not have to worry
        about the side effects of other pytests.
    """
    init_cfg()
    init_modes()
    init_events()
