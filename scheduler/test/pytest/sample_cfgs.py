local_schedule_nonenforcing_cfg = {
    'id': 'durham',
    'name': 'Durham',
    'clothed_setpoints': False,
    'setpoint_enforcement': {
        'enabled': False,
        'frequency_seconds': 5
    }
}

server_disabled_cfg = {
    'server': {
        'enabled': False,
        'port': 9000
    }
}

one_client_cfg = {
    'clients': [
        {
            'id': 'durham',
            'name': 'Durham',
            'ip': '172.16.1.80',
            'port': 9000
        }
    ]
}

site_scheduler_cfg = {
    'scheduler_type': 'SC',
    'local_schedule': local_schedule_nonenforcing_cfg,
    'web_sockets': server_disabled_cfg,
    'scada': {
        'stage_size': 2,
        'max_num_events': 10,
        'num_floats': 1,
        'num_ints': 0,
        'num_bools': 1,
        'num_strings': 0
    }
}

fleet_scheduler_cfg = {
    'scheduler_type': 'FM',
    'web_sockets': one_client_cfg,
    'scada': {
        'stage_size': 1,
        'max_num_events': 100,
        'num_floats': 1,
        'num_ints': 0,
        'num_bools': 1,
        'num_strings': 0
    }
}
