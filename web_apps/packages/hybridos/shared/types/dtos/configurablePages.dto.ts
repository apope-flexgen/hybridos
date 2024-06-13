export type ConfigurablePageDTO = {
  hasStatic: boolean;
  tree?: TreeConfig;
  hasBatchControls?: boolean;
  hasMaintenanceActions?: boolean;
  assetKey?: string;
  displayGroups: {
    [displayGroupID: string]: DisplayGroupDTO;
  };
};

export class TreeConfig implements TreeNode {
  id?: string
  asset_type?: string
  children?: TreeConfig[]
}

export interface TreeNode {
  id?: string;
  children?: TreeConfig[];
}

export type DisplayGroupDTO = {
  displayName?: string;
  tabKey?: string;
  treeId?: string;
  uri?: string;
  batteryViewStatus?: {
    [componentID: string]: StatusComponentDTO;
  };
  status?: {
    [componentID: string]: StatusComponentDTO;
  };
  alarmStatus?: {
    [componentID: string]: StatusComponentDTO;
  };
  faultStatus?: {
    [componentID: string]: StatusComponentDTO;
  };
  control?: {
    [componentID: string]: ControlComponentDTO;
  };
  batchControl?: {
    [componentID: string]: ControlComponentDTO;
  };
  maintenanceActions?: {
    [componentID: string]: MaintenanceActionComponentDTO;
  };
  fault?: string[];
  alarm?: string[];
};

export type MaintenanceActionStep = {
  step_name: string;
  estimated_duration: number;
};

export type MaintenanceActionPath = {
  path_name: string;
  estimated_duration: number;
  steps: MaintenanceActionStep[];
};

export type MaintenanceActionComponentDTO = {
  static?: {
    path_index: number;
    step_index: number;
    status: string;
    paths: MaintenanceActionPath[];
  };
  state?: {
    name?: string;
    path_name?: string;
    step_name?: string;
    path_index?: number;
    step_index?: number;
    seconds_left_in_step?: number;
    seconds_left_in_action?: number;
    status?: string;
  };
};

export type StatusComponentDTO = {
  static?: {
    label?: string;
    unit?: string;
    variant?: DataPointVariantType;
    type?: string;
  };
  state?: {
    value?: ValueType;
  };
};

export type ControlComponentDTO = {
  static?: {
    label: string;
    unit?: string;
    scalar?: number;
    controlType: ControlType;
    extraProps?: {
      [key: string]: any;
    };
  };
  state?: {
    value?: ValueType;
    enabled?: boolean;
    extraProps?: {
      [key: string]: any;
    };
  };
};

export type ControlType =
  | 'enum_button'
  | 'button'
  | 'number'
  | 'enum_slider'
  | 'switch'
  | 'maint_mode_slider'
  | 'maint_action_control';
export type ValueType = string | number | boolean;

export type DataPointVariantType = 'vertical' | 'horizontal' | 'dynamic';

export enum DashboardLayout {
  TABLE = 'table',
  CARD = 'card',
  DIAGRAM = 'diagram',
}
