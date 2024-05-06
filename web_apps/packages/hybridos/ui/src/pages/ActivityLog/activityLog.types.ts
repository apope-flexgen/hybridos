/* eslint-disable max-lines */

export interface Organization {
  id?: string,
  name: string,
}
export interface Details {
  message: string,
  timestamp: string,
}

export enum LogicalOperators {
  Equal = '==',
  NotEqual = '!=',
  LessThan = '<',
  LessThanOrEqual = '<=',
  GreaterThan = '>',
  GreaterThanOrEqual = '>=',
  And = '&&',
  Or = '||',
}

export type LogicalOperator = '==' | '!=' | '<' | '>=' | '>' | '<=';

export interface AlertFilters {
  resolvedFilter?: boolean,
  severityFilter?: number,
  statusFilter?: string,
  orgFilter?: string,
  limit?: number,
  page?: number,
  order?: number,
  orderBy?: string | number,
}
export interface AlertInstance {
  details: Details[],
  site?: string,
  resolved: boolean
}

export interface ActiveAlertObject {
  id: string,
  status: string,
  severity: number, // (0-4), representing (info, status, alarm, fault)
  organization?: string,
  site?: string,
  resolved?: boolean,
  title: string,
  resolution_time?: string,
  resolution_message?: string,
  trigger_time: string,
  details: Details[],
  deadline: number,
}

export interface Comparator {
  type: 'alias' | 'literal',
  value: string | number | boolean,
}

export interface Duration {
  value: string | number,
  unit: 'minute' | 'hour' | 'second',
}

export interface Expression {
  index: number,
  connectionOperator: 'and' | 'or' | null,
  comparator1: Comparator,
  conditional: LogicalOperator,
  comparator2: Comparator,
  duration?: Duration,
  message: string,
}

export interface Alias {
  id: string,
  alias: string,
  uri: string,
  type: 'number' | 'boolean' | 'string',
}

export interface Template {
  id: string,
  type: 'list' | 'sequential' | 'range',
  list?: string[],
  from?: number,
  to?: number,
  minWidth?: number,
  separateAlerts: boolean,
  token: string
}

export interface AlertConfigurationObject {
  id?: string,
  enabled: boolean,
  title: string,
  severity: number | string,
  organization: string,
  last_trigger_time?: string,
  deadline: { value: number | string, unit: 'minute' | 'hour' },
  aliases: Alias[],
  templates?: Template[],
  conditions: Expression[],
}

export interface AlertManagementRow {
  id: string,
  deliver: JSX.Element | string,
  name: string,
  organization?: JSX.Element | string,
  severity: JSX.Element | string,
  last_triggered: string,
  actions: JSX.Element | string,
  deadline: number | string,
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

export interface ResolvedAlertObject {
  id: string,
  severity: number,
  organization?: string,
  site?: string,
  resolved: boolean,
  resolution_time: string,
  resolution_message: string,
  trigger_time: string,
  details: Details[],
  title: string,
}

export interface ResolvedAlertRow {
  id: string,
  severity: JSX.Element,
  organization?: string,
  site?: string,
  alert: string,
  triggerTime: string,
  resolutionTime: string,
  notes: JSX.Element,
  expandRowContent: JSX.Element,
  rowHoverColor: string,
}
