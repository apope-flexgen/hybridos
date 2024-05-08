import { ValueType } from 'shared/types/dtos/configurablePages.dto';

export type SingleReactComponentMetadata = {
  component: string;
  props?: any;
};

export type StatusComponentStateInfo = ValueType;

export type ControlComponentStateInfo = {
  value: ValueType;
  enabled: boolean;
  extraProps?: {
    [key: string]: any;
  };
};

export type MaintenanceActionComponentStateInfo = {
  name: string;
  path_name: boolean;
  step_name: string;
  path_index: number;
  step_index: number;
  seconds_left_in_step: number;
  seconds_left_in_action: number;
  status: string;
};

export type AlarmFaultStatusComponentStateInfo = {
  alarmStatus: boolean;
  faultStatus: boolean;
};

export type MemoizedComponentObject = {
  prevState: any;
  prevControlRecipients?: string[];
  element: JSX.Element;
};

export type PossibleConfirmValues = ValueType;

export interface ControlHandlerObject {
  onChange?: (event?: React.ChangeEvent<HTMLInputElement>) => void;
  onClick?: (event?: React.MouseEvent<HTMLButtonElement>) => void;
  onCheck?: (value: PossibleConfirmValues) => void;
  onX?: (value: PossibleConfirmValues) => void;
  onClickHandlers?: {
    [key: string]: (event?: React.MouseEvent<HTMLButtonElement>) => void;
  };
}
