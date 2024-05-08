export const statusColorMapping: {
  [key: string]:
  | 'error'
  | 'primary'
  | 'warning'
  | 'success'
  | 'inherit'
  | 'secondary'
  | 'disabled'
  | 'action'
  | 'info'
  | undefined;
} = {
  failed: 'error',
  initialized: 'primary',
  stopped: 'warning',
  active: 'success',
  reloading: 'primary',
  inactive: 'warning',
  activating: 'primary',
  deactivating: 'primary',
};

export const initialSystemStatusFilter = {
  serviceNames: [],
  serviceStatus: [],
  connectionStatus: [],
};

export const SYSTEM_STATUS_URL = '/system-status';
