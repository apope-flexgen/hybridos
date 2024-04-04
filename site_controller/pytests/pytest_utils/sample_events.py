balance_event = {
    'start_time': '2024-03-13T09:00:00-04:00',  # Wednesday
    'duration': 90,
    'mode': 'battery_balancing',
    'variables': {
        'start': {
            'batch_value': [1],
            'value': True
        },
        'maint_mode': {
            'batch_value': [1],
            'value': True
        }
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

