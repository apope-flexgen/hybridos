import { ValueType } from 'shared/types/dtos/configurablePages.dto';

export type SingleReactComponentMetadata = {
  component: string
  props?: any
};

export type StatusComponentStateInfo = ValueType;

export type ControlComponentStateInfo = {
  value: ValueType
  enabled: boolean
  extraProps?: {
    [key: string]: any
  }
};

export type MemoizedComponentObject = { prevState: any; element: JSX.Element };

export type PossibleConfirmValues = ValueType;

export interface ControlHandlerObject {
  onChange?: (event?: React.ChangeEvent<HTMLInputElement>) => void
  onClick?: (event?: React.MouseEvent<HTMLButtonElement>) => void
  onCheck?: (value: PossibleConfirmValues) => void
  onX?: (value: PossibleConfirmValues) => void
}
