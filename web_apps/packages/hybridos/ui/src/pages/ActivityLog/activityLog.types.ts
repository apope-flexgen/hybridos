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
  Addition = '+',
  Subtraction = '-',
  Multiplication = '*',
  Division = '%',
  And = '&&',
  Or = '||',
}

export type LogicalOperator = '==' | '!=' | '<' | '>=' | '>' | '<=' | '+' | '-' | '*' | '%' | '&&' | '||';
export interface AlertFilters {
  resolvedFilter?: boolean,
  severityFilter?: number,
  statusFilter?: string,
  orgFilter?: string,
  sitesFilter?: string[],
  limit?: number,
  page?: number,
  order?: number,
  orderBy?: string | number,
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
  unit?: string,
}

export interface Duration {
  value: string | number,
  unit: 'minutes' | 'seconds' | 'hours',
}

export interface Expression {
  index: number,
  connectionOperator: 'and' | 'or' | null,
  comparator1: Comparator,
  conditional: LogicalOperator,
  comparator2: Comparator,
  duration?: Duration
}

export interface Alias {
  alias: string,
  uri: string,
  type: 'number' | 'boolean' | 'string',
  isTemplate?: boolean,
}

export interface Template {
  type: 'list' | 'sequential',
  list?: string[],
  from?: number,
  to?: number,
  token: string
}

export interface AlertConfigurationObject {
  id?: string,
  enabled: boolean,
  title: string,
  description: string,
  severity: number,
  organization: string,
  sites: string[],
  last_trigger_time?: string,
  deadline: number,
  aliases: Alias[],
  templates?: Template[],
  conditions: Expression[],
}

export interface AlertManagementRow {
  id: string,
  deliver: JSX.Element,
  name: string,
  organization: JSX.Element,
  sites: JSX.Element,
  severity: JSX.Element,
  last_triggered: string,
  deadline: string,
  actions: JSX.Element,
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
