export class Item {
  name: string
  uri: string
}

export class Info {
  baseURI?: string
  batteryView?: boolean
  batteryViewSourceURI?: null | string
  batteryViewURI?: string
  isTemplate?: boolean
  items?: Item[]
  name?: string
  sourceURIs?: string[]
  alarmFields?: string[]
  faultFields?: string[]
}

export class Status {
  name: string
  scalar: string
  units: string
  uri: string
}

export class Dashboard {
  info: Info
  status: Status[]
}

export class AddDashboardRequest {
  data: Dashboard[]
}
