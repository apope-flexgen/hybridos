// TODO: fix lint
/* eslint-disable react/jsx-no-useless-fragment */
/* eslint-disable max-lines */
/* eslint-disable no-param-reassign */
import {
  ConfigurablePageDTO,
  ControlComponentDTO,
  StatusComponentDTO,
} from 'shared/types/dtos/configurablePages.dto';
import { generateReactComponentFunction } from './ComponentFactory';
import { ControlTypeMap } from './configurablePages.constants';
import {
  AlertState,
  ConfigurableComponentFunction,
  ConfigurablePageStateStructure,
  DisplayGroupFunctions,
  DisplayGroupStateStructure,
} from './configurablePages.types';

const getStatusStateForDisplayGroup: (
  status: ConfigurablePageDTO['displayGroups'][string]['status']
) => DisplayGroupStateStructure['status'] = (status) => {
  if (status === undefined) return {};

  return Object.entries(status).reduce(
    (statusState: DisplayGroupStateStructure['status'], [componentID, component]) => {
      const { state } = component;
      if (state === undefined || state.value === undefined) return statusState;

      statusState[componentID] = `${state.value}`;
      return statusState;
    },
    {},
  );
};

const getControlStateForDisplayGroup: (
  control: ConfigurablePageDTO['displayGroups'][string]['control']
) => DisplayGroupStateStructure['control'] = (control) => {
  if (control === undefined) return {};

  return Object.entries(control).reduce(
    (controlState: DisplayGroupStateStructure['control'], [componentID, component]) => {
      const { state } = component;
      if (state === undefined || state.enabled === undefined) return controlState;

      controlState[componentID] = {
        value: state.value !== undefined ? state.value : '',
        enabled: state.enabled,
      };
      return controlState;
    },
    {},
  );
};

export const getUpdatedStates: (
  data: ConfigurablePageDTO['displayGroups']
) => [ConfigurablePageStateStructure, AlertState] = (data) => {
  const updatedComponentState: ConfigurablePageStateStructure = {};
  const updatedAlertState: AlertState = {};

  Object.entries(data).forEach(([displayGroupID, displayGroup]) => {
    updatedComponentState[displayGroupID] = {
      status: getStatusStateForDisplayGroup(displayGroup.status),
      control: getControlStateForDisplayGroup(displayGroup.control),
    };

    updatedAlertState[displayGroupID] = {
      alarmInformation: displayGroup.alarm || [],
      faultInformation: displayGroup.fault || [],
    };
  });

  return [updatedComponentState, updatedAlertState];
};

const generateDataPointComponentFunction: (
  displayGroupID: string,
  componentID: string,
  staticData: StatusComponentDTO['static']
) => ConfigurableComponentFunction = (displayGroupID, componentID, staticData) => {
  if (staticData === undefined) return () => <></>;

  return generateReactComponentFunction(
    {
      component: 'DataPoint',
      props: {
        label: staticData.label,
        unit: staticData.unit,
      },
    },
    displayGroupID,
    componentID,
  );
};

const getStatusComponentFunctions: (
  status: ConfigurablePageDTO['displayGroups'][string]['status'],
  displayGroupID: string
) => ConfigurableComponentFunction[] = (status, displayGroupID) => {
  if (status === undefined) return [];

  return Object.entries(status).reduce(
    (statusFunctions: ConfigurableComponentFunction[], [componentID, component]) => {
      const { static: staticData } = component;
      if (staticData === undefined) return statusFunctions;

      statusFunctions.push(
        generateDataPointComponentFunction(displayGroupID, componentID, staticData),
      );
      return statusFunctions;
    },
    [],
  );
};

const generateControlComponentFunction: (
  displayGroupID: string,
  componentID: string,
  staticData: ControlComponentDTO['static']
) => ConfigurableComponentFunction = (displayGroupID, componentID, staticData) => {
  if (staticData === undefined) return () => <></>;

  // FIXME: may want to do this on the backend, or just better on the frontend
  const controlTypeString = staticData.controlType in ControlTypeMap ? staticData.controlType : 'TextField';

  const ReactComponent = ControlTypeMap[controlTypeString];

  return generateReactComponentFunction(
    {
      component: ReactComponent,
      props: {
        label: staticData.label,
        unit: staticData.unit,
        scalar: staticData.scalar,
      },
    },
    displayGroupID,
    componentID,
    `${displayGroupID}/${componentID}`,
  );
};

const getControlComponentFunctions: (
  control: ConfigurablePageDTO['displayGroups'][string]['control'],
  displayGroupID: string
) => ConfigurableComponentFunction[] = (control, displayGroupID) => {
  if (control === undefined) return [];

  return Object.entries(control).reduce(
    (controlFunctions: ConfigurableComponentFunction[], [componentID, component]) => {
      const { static: staticData } = component;
      if (staticData === undefined) return controlFunctions;

      controlFunctions.push(
        generateControlComponentFunction(displayGroupID, componentID, staticData),
      );
      return controlFunctions;
    },
    [],
  );
};

export const getUpdatedComponentFunctions: (data: ConfigurablePageDTO['displayGroups']) => {
  [displayGroupID: string]: DisplayGroupFunctions;
} = (data) => {
  const updatedComponentFunctions: { [displayGroupID: string]: DisplayGroupFunctions } = {};

  Object.entries(data).forEach(([displayGroupID, displayGroup]) => {
    updatedComponentFunctions[displayGroupID] = {
      displayName: displayGroup.displayName || displayGroupID,
      statusFunctions: getStatusComponentFunctions(displayGroup.status, displayGroupID),
      controlFunctions: getControlComponentFunctions(displayGroup.control, displayGroupID),
    };
  });

  return updatedComponentFunctions;
};
