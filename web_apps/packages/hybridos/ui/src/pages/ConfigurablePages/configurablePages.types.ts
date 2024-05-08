import { ValueType } from 'shared/types/dtos/configurablePages.dto';

export type AlertState = {
  [displayGroupID: string]: {
    alarmInformation: string[];
    faultInformation: string[];
  };
};

export type MaintModeState = {
  [displayGroupID: string]: {
    value: boolean;
  };
};

export type DisplayGroupStateStructure = {
  status: {
    [componentID: string]: string;
  };
  control: {
    [componentID: string]: {
      value: ValueType;
      enabled?: boolean;
      extraProps?: {
        [key: string]: any;
      };
    };
  };
  maintenanceActions: {
    [componentId: string]: any;
  };
};

export type ConfigurablePageStateStructure = {
  [displayGroupID: string]: DisplayGroupStateStructure;
};

export type ConfigurableComponentFunction = (
  assetState: ConfigurablePageStateStructure,
) => JSX.Element;

export type ConfigurableBatchControlComponentFunction = (
  assetState: ConfigurablePageStateStructure,
  controlRecipients: string[],
) => JSX.Element;

export type DisplayGroupFunctions = {
  displayName: string;
  tabKey: string | undefined;
  statusFunctions: ConfigurableComponentFunction[];
  alarmFaultStatusFunction: ConfigurableComponentFunction;
  controlFunctions: ConfigurableComponentFunction[];
  batchControlFunctions?: ConfigurableBatchControlComponentFunction[];
  maintenanceActionsFunctions: ConfigurableComponentFunction[];
};
