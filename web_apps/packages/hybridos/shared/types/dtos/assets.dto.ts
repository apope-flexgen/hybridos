export class Alarm {
  alarmFields?: []
  faultFields?: []
}

export class Control {
  inputType: string
  name: string
  scalar: string
  units: string
  uri: string
}

export class Info {
  alarmFields?: string[]
  assetKey?: string
  tabKey?: string
  baseURI?: string
  customRegex?: string
  extension?: string
  range?: string[]
  faultFields?: string[]
  hasBatchControls?: boolean
  hasMaintenanceActions?: boolean
  hasSummary?: boolean
  icon?: string
  itemName?: string
  name?: string
  numberOfItems?: string
  sourceURI?: string
}

export class Status {
  name?: string
  scalar?: string
  units?: string
  uri?: string
}

export class Summary {
  name?: string
  scalar?: string
  units?: string
  uri?: string
}

export class SummaryControl {
  inputType: string
  name: string
  scalar: string
  units: string
  uri: string
}

export class Asset {
  alarms: Alarm
  batchControls?: Control[]
  controls: Control[]
  info: Info
  statuses: Status[]
  summary: Summary[]
  summaryControls: SummaryControl[]
}

export class AddAssetRequest {
  data: Asset[]
}
