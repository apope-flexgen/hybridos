event_0 = {
    'start_time': '2024-03-13T09:00:00-04:00',  # Wednesday
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

event_1 = {
    'start_time': '2024-04-13T09:00:00-04:00',  # Saturday
    'duration': 90,
    'mode': 'target_soc',
    'variables': {
        'soc_target': 50
    },
    'repeat': {
        'cycle': 'day',
        'day_mask': 127,
        'end_count': -1,
        'end_time': '2000-01-01T00:00:00Z',
        'exceptions': ['2024-04-16T09:00:00-04:00'],
        'frequency': 1
    }
}

event_2 = {
    'start_time': '2024-04-19T14:00:00-04:00',  # Friday
    'duration': 90,
    'mode': 'target_soc',
    'variables': {
        'soc_target': 50
    },
    'repeat': {
        'cycle': 'week',
        'day_mask': 3,  # Fridays and Saturdays
        'end_count': 0,
        'end_time': '2024-05-18T12:00:00-04:00',  # Saturday
        'exceptions': [],
        'frequency': 2  # every other week
    }
}
