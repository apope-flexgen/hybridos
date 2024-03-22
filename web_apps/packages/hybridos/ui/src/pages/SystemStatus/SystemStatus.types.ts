import { ConnectionStatus } from 'shared/types/dtos/systemStatus.dto';

export interface SystemStatusObject {
  dependencies?: string[],
  serviceName?: string,
  serviceStatus?: string,
  connectionStatus?: ConnectionStatus,
  cpuUsage?: number,
  memoryUsage?: number,
  uptime?: number | string,
  lastRestart?: string,
  actions?: ServiceActionObject[],
}

export interface ServiceActionObject {
  enabled: boolean,
  action: ServiceActionType,
}

export type ServiceActionType = 'start' | 'stop' | 'restart';

export interface SystemStatusRow {
  id: string,
  dependencies: JSX.Element | null,
  service_name: string,
  service_status: JSX.Element | string,
  connection_status: JSX.Element | string,
  cpu_usage: string,
  memory_usage: string,
  uptime: string | number,
  last_restart: string,
  actions: JSX.Element | any[],
}

export interface SystemStatusFilter {
  serviceNames: string[],
  serviceStatus: string[],
  connectionStatus: string[],
}

export interface SystemStatusHeaderProps {
  systemStatusData: SystemStatusObject[],
  summarySystemData: SystemStatusObject,
  setDisplayData: React.Dispatch<React.SetStateAction<SystemStatusObject[]>>
}

export interface SystemStatusTableProps {
  systemStatusData: SystemStatusObject[],
}

export interface FilterSearchBarProps {
  systemStatusData: SystemStatusObject[],
  setDisplayData: React.Dispatch<React.SetStateAction<SystemStatusObject[]>>
}
