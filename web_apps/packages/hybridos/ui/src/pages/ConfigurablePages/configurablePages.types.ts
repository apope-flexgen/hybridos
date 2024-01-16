import { ValueType } from 'shared/types/dtos/configurablePages.dto';

export type AlertState = {
  [displayGroupID: string]: {
    alarmInformation: string[]
    faultInformation: string[]
  }
};

export type MaintModeState = {
  [displayGroupID: string]: {
    value: boolean
  }
};

export type MaintActionsControlState = {
  [displayGroupID: string]: {
    value: boolean
  }
};

export type DisplayGroupStateStructure = {
  status: {
    [componentID: string]: string
  }
  control: {
    [componentID: string]: {
      value: ValueType
      enabled?: boolean
      extraProps?: {
        [key: string]: any
      }
    }
  }
};

export type ConfigurablePageStateStructure = {
  [displayGroupID: string]: DisplayGroupStateStructure
};

export type ConfigurableComponentFunction = (
  assetState: ConfigurablePageStateStructure
) => JSX.Element;

export type DisplayGroupFunctions = {
  displayName: string
  statusFunctions: ConfigurableComponentFunction[]
  controlFunctions: ConfigurableComponentFunction[]
};
