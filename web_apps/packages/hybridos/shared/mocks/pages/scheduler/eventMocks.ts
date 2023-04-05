import { SchedulerEvent } from '../../../types/dtos/scheduler.dto'
import { v4 as uuid } from '../../../../ui/node_modules/uuid'

export const schedulerEventsData: SchedulerEvent[] = [
    {
        id: uuid(),
        duration: 90,
        mode: 'charging',
        start_time: '01-24-2023 11:30:00',
        variables: {
            charge_cmd: 3000,
            active_pwr: 2500,
        },
    },
    {
        id: uuid(),
        duration: 90,
        mode: 'discharging',
        start_time: '01-28-2023 16:00:00',
        variables: {},
    },
    {
        id: uuid(),
        duration: 90,
        mode: 'maintenance',
        start_time: '01-31-2023 18:30:00',
        variables: {
            reactive_pwr: 3000,
        },
        repeat: {
            start: '01-31-2023 18:30:00',
            id: uuid(),
            cycle: 'day',
            frequency: 4,
        },
    },
    {
        id: uuid(),
        duration: 90,
        mode: 'charging',
        start_time: '02-05-2023 05:30:00',
        variables: {
            charge_cmd: 3000,
            active_pwr: 1500,
        },
        repeat: {
            start: '02-05-2023 05:30:00',
            id: uuid(),
            cycle: 'day',
            frequency: 2,
            end_count: 9,
        },
    },
    {
        id: uuid(),
        duration: 90,
        mode: 'discharging',
        start_time: '02-06-2023 05:30:00',
        variables: {},
        repeat: {
            start: '02-06-2023 05:30:00',
            id: uuid(),
            cycle: 'week',
            frequency: 2,
            day_mask: 42,
            end_time: '03-10-2023 07:00:00',
        },
    },
]
