export type ConfigurablePageDTO = {
  hasStatic: boolean;
  hasAllControls?: boolean;
  displayGroups: {
    [displayGroupID: string]: DisplayGroupDTO;
  };
};

export type DisplayGroupDTO = {
  displayName?: string;
  batteryViewStatus?: {
    [componentID: string]: StatusComponentDTO;
  };
  status?: {
    [componentID: string]: StatusComponentDTO;
  };
  control?: {
    [componentID: string]: ControlComponentDTO;
  };
  fault?: string[];
  alarm?: string[];
};

export type StatusComponentDTO = {
  static?: {
    label?: string;
    unit?: string;
    variant?: DataPointVariantType;
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
  | 'maint_mode_slider';
export type ValueType = string | number | boolean;

export type DataPointVariantType = 'vertical' | 'horizontal' | 'dynamic';

export enum DashboardLayout {
  TABLE = 'table',
  CARD = 'card',
}
