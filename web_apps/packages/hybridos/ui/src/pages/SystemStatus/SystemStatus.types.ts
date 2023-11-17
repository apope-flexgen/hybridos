export interface SystemStatusObject {
  dependencies?: string[],
  serviceName?: string,
  serviceStatus?: string,
  connectionStatus?: string,
  cpuUsage?: number,
  memoryUsage?: number,
  uptime?: number | string,
  lastRestart?: string,
  actions?: string[],
}

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
  setDisplayData: React.Dispatch<React.SetStateAction<SystemStatusObject[]>>
}

export interface SystemStatusTableProps {
  systemStatusData: SystemStatusObject[],
}
