export class EventsRequestParams {}

export class EventsObject {
  [siteId: string]: SchedulerEvent[];
}

export class Connected {
  [siteName: string]: boolean;
}

export class SchedulerEvent {
  id?: string
  duration: number
  mode: string
  start_time: string
  variables?: {
    [key: string]: any;
  }
  repeat?: Repeat
}

export class schedulerEventForAPI {
  id?: string
  duration: number
  mode: string
  start_time: string
  variables?: {
    [key: string]: number | boolean | string;
  }
  repeat?: RepeatForAPI
}

export class Repeat {
  id?: string
  start: string
  cycle?: 'day' | 'week'
  frequency?: number
  day_mask?: number | undefined
  end_time?: string
  end_count?: number
  exceptions?: string[]
}

export class RepeatForAPI {
  id?: string
  cycle?: 'day' | 'week'
  frequency?: number
  day_mask?: number | undefined
  end_time?: string
  end_count?: number
  exceptions?: string[]
}

export class EventVariables {
  id: string
  name: string
  type: string
  unit: string
  uri: string
  value: number | boolean
}

export class Mode {
  id?: string
  name: string
  color_code: string
  icon: string
  variables: EventVariables[]
  constants: EventVariables[]
}

export class Configuration {
  scheduler_type: string
  // required for SC, optional for FM
  local_schedule?: {
    id: string;
    name: string;
    clothed_setpoints?: boolean;
    setpoint_enforcement: {
      enabled: boolean;
      frequency_seconds: number;
    };
  }
  web_sockets?: {
    clients?: {
      id: string;
      name: string;
      ip: string;
      port: number | string;
    }[];
    server?: {
      enabled: boolean;
      port: number | string;
    };
  }
  scada?: {
    append_can_edit: boolean;
    stage_size: number;
    max_num_events: number;
    num_floats: number;
    num_ints: number;
    num_bools: number;
    num_strings: number;
  }
}

export class DeleteModeID {
  id: string
}

/** This is the format the API accepts. */
export class ApiMode {
  [id: string]: ModeBody;
}

export class ModeBody {
  name: string
  color_code: string
  icon: string
  variables: {
    id: string;
    name: string;
    type: string;
    unit: string;
    uri: string;
    value: number | boolean;
    batch_value?: string[];
    batch_prefix?: string;
    batch_range?: string[];
  }[]
  constants: {
    id: string;
    name: string;
    type: string;
    unit: string;
    uri: string;
    value: number | boolean;
  }[]
}

/** Mode format used in backend. */
export class ModeDTO {
  id: string
  mode: {
    name: string;
    color_code: string;
    icon: string;
    variables: {
      id: string;
      name: string;
      type: string;
      unit: string;
      uri: string;
      value: number | boolean;
    }[];
    constants: {
      id: string;
      name: string;
      type: string;
      unit: string;
      uri: string;
      value: number | boolean;
    }[];
  }
}

export class Timezone {
  [key: string]: string;
}

export class DeleteEventRequestParams {
  public id: string[]
}
