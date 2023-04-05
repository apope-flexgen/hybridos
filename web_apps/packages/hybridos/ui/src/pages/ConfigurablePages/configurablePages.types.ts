import { ValueType } from 'shared/types/dtos/configurablePages.dto';

export type AlertState = {
  [displayGroupID: string]: {
    alarmInformation: string[]
    faultInformation: string[]
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
