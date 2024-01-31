// TODO: fix lint
/* eslint-disable react/jsx-no-useless-fragment */
/* eslint-disable max-lines */
/* eslint-disable max-len */
/* eslint-disable no-param-reassign */
import {
  ConfigurablePageDTO,
  ControlComponentDTO,
  MaintenanceActionComponentDTO,
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
  MaintModeState,
} from './configurablePages.types';

const getStatusStateForDisplayGroup: (
  status: ConfigurablePageDTO['displayGroups'][string]['status'],
  batteryViewStatus: ConfigurablePageDTO['displayGroups'][string]['batteryViewStatus'],
) => DisplayGroupStateStructure['status'] = (status, batteryViewStatus) => {
  if (status === undefined) return {};
  // eslint-disable-next-line max-len
  const combinedStatuses = batteryViewStatus === undefined ? status : { ...status, ...batteryViewStatus };

  return Object.entries(combinedStatuses).reduce(
    (statusState: DisplayGroupStateStructure['status'], [componentID, component]) => {
      const { state } = component;
      if (state === undefined || state.value === undefined) return statusState;

      statusState[componentID] = `${state.value}`;
      return statusState;
    },
    {},
  );
};

const getMaintenanceActionStateForDisplayGroup: (
  maintenanceAction: ConfigurablePageDTO['displayGroups'][string]['maintenanceActions'],
) => DisplayGroupStateStructure['maintenanceActions'] = (maintenanceAction) => {
  if (maintenanceAction === undefined) return {};
  return Object.entries(maintenanceAction).reduce(
    (maintenanceActionsState: DisplayGroupStateStructure['maintenanceActions'], [componentID, component]) => {
      const { state } = component;
      if (state === undefined) return maintenanceActionsState;

      maintenanceActionsState[componentID] = state;
      return maintenanceActionsState;
    },
    {},
  );
};

const getControlStateForDisplayGroup: (
  control: ConfigurablePageDTO['displayGroups'][string]['control'],
) => DisplayGroupStateStructure['control'] = (control) => {
  if (control === undefined) return {};

  return Object.entries(control).reduce(
    (controlState: DisplayGroupStateStructure['control'], [componentID, component]) => {
      const { state } = component;
      if (state === undefined || state.enabled === undefined) return controlState;

      controlState[componentID] = {
        value: state.value !== undefined ? state.value : '',
        enabled: state.enabled,
        extraProps: state.extraProps,
      };
      return controlState;
    },
    {},
  );
};

export const getUpdatedStates: (
  data: ConfigurablePageDTO['displayGroups'],
) => [ConfigurablePageStateStructure, AlertState, MaintModeState] = (data) => {
  const updatedComponentState: ConfigurablePageStateStructure = {};
  const updatedAlertState: AlertState = {};
  const updatedMaintModeState: MaintModeState = {};

  Object.entries(data).forEach(([displayGroupID, displayGroup]) => {
    updatedComponentState[displayGroupID] = {
      status: getStatusStateForDisplayGroup(displayGroup.status, displayGroup.batteryViewStatus),
      control: getControlStateForDisplayGroup(displayGroup.control),
      maintenanceActions: getMaintenanceActionStateForDisplayGroup(displayGroup.maintenanceActions)
    };

    updatedAlertState[displayGroupID] = {
      alarmInformation: displayGroup.alarm || [],
      faultInformation: displayGroup.fault || [],
    };

    updatedMaintModeState[displayGroupID] = {
      value: Boolean(displayGroup?.control?.maint_mode?.state?.value) || false,
    };
  });

  return [updatedComponentState, updatedAlertState, updatedMaintModeState];
};

const generateDataPointComponentFunction: (
  displayGroupID: string,
  componentID: string,
  staticData: StatusComponentDTO['static'],
) => ConfigurableComponentFunction = (displayGroupID, componentID, staticData) => {
  if (staticData === undefined) return () => <></>;

  return generateReactComponentFunction(
    {
      component: 'DataPoint',
      props: {
        label: staticData.label,
        unit: staticData.unit,
        divider: true,
        variant: staticData.variant,
      },
    },
    displayGroupID,
    componentID,
  );
};

const generateBatteryViewReactComponentFunction: (
  displayGroupID: string,
  componentID: string,
  staticData: StatusComponentDTO['static'],
) => ConfigurableComponentFunction = (displayGroupID, componentID, staticData) => {
  if (staticData === undefined) return () => <></>;
  return generateReactComponentFunction(
    {
      component: 'Progress',
      props: {
        label: staticData.label,
        showPercentage: true,
        orientation: 'horizontal',
      },
    },
    displayGroupID,
    componentID,
  );
};

const getStatusComponentFunctions: (
  status: ConfigurablePageDTO['displayGroups'][string]['status'],
  displayGroupID: string,
  batteryViewStatus: ConfigurablePageDTO['displayGroups'][string]['batteryViewStatus'],
) => ConfigurableComponentFunction[] = (status, displayGroupID, batteryViewStatus) => {
  if (status === undefined) return [];

  let batteryViewComponents: ConfigurableComponentFunction[] = [];

  if (batteryViewStatus !== undefined) {
    batteryViewComponents = Object.entries(batteryViewStatus).reduce(
      (batteryViewStatusFunctions: ConfigurableComponentFunction[], [componentID, component]) => {
        const { static: staticData } = component;
        if (staticData === undefined) return batteryViewStatusFunctions;

        batteryViewStatusFunctions.push(
          generateBatteryViewReactComponentFunction(displayGroupID, componentID, staticData),
        );

        return batteryViewStatusFunctions;
      },
      [],
    );
  }
  return Object.entries(status)
    .reduce((statusFunctions: ConfigurableComponentFunction[], [componentID, component]) => {
      const { static: staticData } = component;
      if (staticData === undefined) return statusFunctions;

      statusFunctions.push(
        generateDataPointComponentFunction(displayGroupID, componentID, staticData),
      );

      return statusFunctions;
    }, [])
    .concat(batteryViewComponents);
};

const generateProgressStepperComponent: (
  displayGroupID: string,
  componentID: string,
  staticData: MaintenanceActionComponentDTO['static'],
) => ConfigurableComponentFunction = (displayGroupID, componentID, staticData) => {
  if (staticData === undefined) return () => <></>;
  return generateReactComponentFunction(
    {
      component: 'MaintActionProgress',
      props: {
        label: componentID,
        stepIndex: staticData.step_index,
        pathIndex: staticData.path_index,
        paths: staticData.paths,
        status: staticData.status.toLowerCase(),
        steps: staticData.paths[staticData.path_index].steps,
        inactive: staticData.status.toLowerCase() === 'inactive',
        stopActionURI: `${displayGroupID}/actions/${componentID}/stop`,
        clearActionURI: `${displayGroupID}/actions/${componentID}/clear`,
      },
    },
    displayGroupID,
    componentID,
    '',
    true,
  );
};

const getMaintenanceActionComponentFunctions: (
  maintenanceAction: ConfigurablePageDTO['displayGroups'][string]['maintenanceActions'],
  displayGroupID: string,
) => ConfigurableComponentFunction[] = (maintenanceAction, displayGroupID) => {
  if (maintenanceAction === undefined) return [];
  return Object.entries(maintenanceAction)
    .reduce((maintenanceActionFunctions: ConfigurableComponentFunction[], [componentID, component]) => {
      const { static: staticData } = component;

      if (staticData === undefined) return maintenanceActionFunctions;
      maintenanceActionFunctions.push(
        generateProgressStepperComponent(displayGroupID, componentID, staticData),
      );

      return maintenanceActionFunctions;
    }, [])
};

const generateControlComponentFunction: (
  displayGroupID: string,
  componentID: string,
  staticData: ControlComponentDTO['static'],
) => ConfigurableComponentFunction = (displayGroupID, componentID, staticData) => {
  if (staticData === undefined) return () => <></>;

  // FIXME: may want to do this on the backend, or just better on the frontend
  const controlTypeString = staticData.controlType in ControlTypeMap ? staticData.controlType : 'TextField';

  let ReactComponent = ControlTypeMap[controlTypeString];
  const label = staticData.label.toLowerCase();
  if (label.includes('start site')) ReactComponent = 'ConfirmCancelButton';

  return generateReactComponentFunction(
    {
      component: ReactComponent,
      props: {
        label: staticData.label,
        unit: staticData.unit,
        scalar: staticData.scalar,
        extraProps: staticData.extraProps || undefined,
      },
    },
    displayGroupID,
    componentID,
    `${displayGroupID}/${componentID}`,
  );
};

const getControlComponentFunctions: (
  control: ConfigurablePageDTO['displayGroups'][string]['control'],
  displayGroupID: string,
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
      statusFunctions: getStatusComponentFunctions(
        displayGroup.status,
        displayGroupID,
        displayGroup.batteryViewStatus,
      ),
      controlFunctions: getControlComponentFunctions(displayGroup.control, displayGroupID),
      maintenanceActionsFunctions: getMaintenanceActionComponentFunctions(displayGroup.maintenanceActions, displayGroupID)
    };
  });

  return updatedComponentFunctions;
};
