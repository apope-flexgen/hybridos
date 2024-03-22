export interface Details {
  message: string,
  timestamp: string,
}

export interface ActiveAlertObject {
  id: string,
  status: string,
  severity: number, // (0-4), representing (info, status, alarm, fault)
  organization?: string,
  site?: string,
  resolved?: boolean,
  resolution_time?: string,
  resolution_message?: string,
  trigger_time: string,
  details: Details[],
  deadline: number,
  title: string,
}

export interface ActiveAlertRow {
  id: string,
  status: JSX.Element,
  severity: JSX.Element,
  organization?: string,
  site?: string,
  alert: string,
  timestamp: string,
  deadline: JSX.Element,
  resolve: JSX.Element,
  expandRowContent: JSX.Element,
  rowHoverColor: string,
}
