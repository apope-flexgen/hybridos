import { Configuration } from 'shared/types/dtos/scheduler.dto';

const schedulerConfigurationData: Configuration = {
  scheduler_type: 'SC',
  local_schedule: {
    id: 'durham',
    name: 'Durham',
    setpoint_enforcement: {
      enabled: true,
      frequency_seconds: 5,
    },
  },
  web_sockets: {
    server: {
      enabled: true,
      port: 9000,
    },
  },
  scada: {
    stage_size: 5,
    max_num_events: 20,
    num_floats: 1,
    num_ints: 0,
    num_bools: 1,
    num_strings: 0,
  },
};

export default schedulerConfigurationData;
