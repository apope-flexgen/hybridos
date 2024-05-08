/* eslint-disable */
// TODO: fix lint
import { useContext } from 'react';
import {
  ConfigurableComponentFunction,
  ConfigurablePageStateStructure,
} from 'src/pages/ConfigurablePages/configurablePages.types';
import RealTimeService from 'src/services/RealTimeService/realtime.service';
import storybookComponents, { controlsSets } from './storybookHash';
import {
  SingleReactComponentMetadata,
  ControlHandlerObject,
  PossibleConfirmValues,
  StatusComponentStateInfo,
  ControlComponentStateInfo,
  MemoizedComponentObject,
  MaintenanceActionComponentStateInfo,
  AlarmFaultStatusComponentStateInfo,
} from './componentFactory.types';
import { useAppContext } from 'src/App/App';
import { Roles } from 'shared/types/api/Users/Users.types';

// FIXME: debounce button inputs (should this be an option in our button component?)
const generateControlHandlerObject = (
  uri: string,
  componentName: string,
  scalar: number | null,
  batchControl?: boolean,
  controlRecipients?: string[],
  batchControlExtension?: string,
): ControlHandlerObject => {
  if (controlsSets.onClickHandlers.includes(componentName)) {
    if (batchControl) {
      const trueOnClick = () => {
        const controlURIs =
          controlRecipients?.map((batchURI: string) => batchURI + `/${batchControlExtension}`) ||
          [];

        controlURIs.forEach((uri) => {
          const realTimeService = RealTimeService.Instance;
          realTimeService.send('fimsNoReply', {
            method: 'set',
            uri,
            replyto: 'web_ui',
            body: true,
            username: 'web_ui',
          });
        });
      };
      const falseOnClick = () => {
        const controlURIs =
          controlRecipients?.map((batchURI: string) => batchURI + `/${batchControlExtension}`) ||
          [];

        controlURIs.forEach((uri) => {
          const realTimeService = RealTimeService.Instance;
          realTimeService.send('fimsNoReply', {
            method: 'set',
            uri,
            replyto: 'web_ui',
            body: false,
            username: 'web_ui',
          });
        });
      };
      return {
        onClickHandlers: {
          true: trueOnClick,
          false: falseOnClick,
        },
      };
    }
  }

  if (controlsSets.onClick.includes(componentName)) {
    if (batchControl) {
      return {
        onClick: () => {
          const controlURIs =
            controlRecipients?.map((batchURI: string) => batchURI + `/${batchControlExtension}`) ||
            [];
          controlURIs.forEach((uri) => {
            const realTimeService = RealTimeService.Instance;
            realTimeService.send('fimsNoReply', {
              method: 'set',
              uri,
              replyto: 'web_ui',
              body: true,
              username: 'web_ui',
            });
          });
        },
      };
    }
    return {
      onClick: () => {
        const realTimeService = RealTimeService.Instance;
        realTimeService.send('fimsNoReply', {
          method: 'set',
          uri,
          replyto: 'web_ui',
          body: true,
          username: 'web_ui',
        });
      },
    };
  }

  if (controlsSets.withConfirm.includes(componentName)) {
    if (batchControl) {
      const controlURIs =
        controlRecipients?.map((batchURI: string) => batchURI + `/${batchControlExtension}`) || [];
      return {
        onCheck: (value: PossibleConfirmValues) => {
          controlURIs.forEach((uri) => {
            const realTimeService = RealTimeService.Instance;
            let realValue = value;
            if (typeof value !== 'boolean') {
              const maybeNumber = Number(value);
              if (!Number.isNaN(maybeNumber) && scalar !== null) {
                realValue = maybeNumber * scalar;
              }
            }
            realTimeService.send('fimsNoReply', {
              method: 'set',
              uri,
              replyto: 'web_ui',
              body: realValue,
              username: 'web_ui',
            });
          });
        },
        onX: (value: PossibleConfirmValues) => {},
        ...(componentName === 'MaintModeSlider'
          ? controlURIs.forEach((uri) => ({
              onLock: (value: boolean) => {
                const realTimeService = RealTimeService.Instance;
                realTimeService.send('lockMode', {
                  value: value,
                  uri: uri.replace('/maint_mode', ''),
                });
              },
              logging: (reason: string, comment: string) => {
                const realTimeService = RealTimeService.Instance;
                const data = {
                  modified_field: 'maintenance_mode',
                  modified_value: true,
                  extraFields: {
                    reason,
                    comment,
                  },
                };
                realTimeService.send('audit-logging', data);
              },
            }))
          : {}),
      };
    }

    return {
      onCheck: (value: PossibleConfirmValues) => {
        const realTimeService = RealTimeService.Instance;
        let realValue = value;
        if (typeof value !== 'boolean') {
          const maybeNumber = Number(value);
          if (!Number.isNaN(maybeNumber) && scalar !== null) {
            realValue = maybeNumber * scalar;
          }
        }
        realTimeService.send('fimsNoReply', {
          method: 'set',
          uri,
          replyto: 'web_ui',
          body: realValue,
          username: 'web_ui',
        });
      },
      onX: (value: PossibleConfirmValues) => {},
      ...(componentName === 'MaintModeSlider'
        ? {
            onLock: (value: boolean) => {
              const realTimeService = RealTimeService.Instance;
              realTimeService.send('lockMode', {
                value: value,
                uri: uri.replace('/maint_mode', ''),
              });
            },
            logging: (reason: string, comment: string) => {
              const realTimeService = RealTimeService.Instance;
              const data = {
                modified_field: 'maintenance_mode',
                modified_value: true,
                extraFields: {
                  reason,
                  comment,
                },
              };
              realTimeService.send('audit-logging', data);
            },
          }
        : {}),
    };
  }

  return {};
};

// FIXME: any
const getSpecificStateInfo = (
  assetState: ConfigurablePageStateStructure,
  displayGroupID: string,
  componentID: string,
  control: boolean,
  maintenanceAction?: boolean,
  alarmIDs?: string[],
  faultIDs?: string[],
): any => {
  if (assetState && displayGroupID in assetState) {
    if (control) {
      return assetState[displayGroupID].control[componentID];
    }
    if (maintenanceAction) {
      return assetState[displayGroupID].maintenanceActions[componentID];
    }
    if (componentID === 'AlarmFaultStatus') {
      return {
        alarmStatus: alarmIDs?.some((id) => assetState[displayGroupID].status[id] === 'true'),
        faultStatus: faultIDs?.some((id) => assetState[displayGroupID].status[id] === 'true'),
      };
    }

    return assetState[displayGroupID].status[componentID];
  }
};

const memoIsValid = (
  memoized: MemoizedComponentObject,
  stateInfo: StatusComponentStateInfo | ControlComponentStateInfo | undefined,
  controlRecipients?: string[],
): boolean => {
  const { prevState, prevControlRecipients } = memoized;
  if (stateInfo === undefined && controlRecipients === undefined) {
    return true;
  }

  if (
    JSON.stringify(prevState) === JSON.stringify(stateInfo) &&
    JSON.stringify(controlRecipients) === JSON.stringify(prevControlRecipients)
  ) {
    return true;
  }

  return false;
};

// TODO: this could probably be done better
const organizeProps = (
  props: any,
  component: string,
  stateInfo:
    | StatusComponentStateInfo
    | ControlComponentStateInfo
    | MaintenanceActionComponentStateInfo
    | AlarmFaultStatusComponentStateInfo
    | undefined,
  batchControl?: boolean,
  controlRecipients?: string[],
): any => {
  const { currentUser } = useAppContext();

  if (stateInfo === undefined && !batchControl) {
    props.disabled = currentUser?.role === Roles.Observer;
    return props;
  }
  // if this is state info regarding maintenance actions
  else if (typeof stateInfo === 'object' && 'step_name' in stateInfo) {
    props.stepIndex = stateInfo.step_index;
    props.pathIndex = stateInfo.path_index;
    props.status = stateInfo.status.toLowerCase();
    props.inactive = stateInfo.status.toLowerCase() === 'inactive';
    return props;
  }

  const [value, enabled] =
    typeof stateInfo === 'object' && 'value' in stateInfo
      ? [stateInfo.value, stateInfo.enabled]
      : [stateInfo, true];

  props.disabled = !enabled || currentUser?.role === Roles.Observer;

  if (component === 'MaintActionControl') {
    if (typeof stateInfo === 'object' && 'extraProps' in stateInfo && stateInfo.extraProps?.options)
      props.options = stateInfo.extraProps.options;
    if (batchControl) props.controlURI = controlRecipients;
  }

  if (
    component === 'AlarmFaultContainer' &&
    typeof stateInfo === 'object' &&
    'alarmStatus' in stateInfo &&
    'faultStatus' in stateInfo
  ) {
    props.showAlarm = stateInfo.alarmStatus;
    props.showFault = stateInfo.faultStatus;
  }

  if (component === 'TextField') {
    props.disabled = currentUser?.role === Roles.Observer;
    props.bold = enabled;
    props.value = value;
    props.adornment = 'end';
    props.type = 'text';
    props.TextAdornment = props.unit === '' ? ' ' : props.unit;
    props.helperTextSize = 'small';
    props.size = 'small';
    props.color = enabled ? 'secondary' : 'primary';
  } else if (component === 'NumericInput') {
    props.value = value;
    props.placeholder = 0.0;
    props.endTextAdornment = props.unit === '' ? ' ' : props.unit;
    props.showStepper = false;
    props.size = 'small';
    (props.validationRegEx = 'floats'), (props.bold = enabled);
    props.helperTextSize = 'small';
    props.color = enabled ? 'secondary' : 'primary';
    props.disabled = currentUser?.role === Roles.Observer;
  } else if (component === 'Switch') {
    props.labelPlacement = 'right';
    props.color = 'primary';
    props.value = value;
    props.autoLayout = true;
  } else if (component === 'MaintModeSlider') {
    props.autoLayout = true;
    props.value = value;
    props.color = 'primary';
    const { extraProps } = stateInfo as ControlComponentStateInfo;
    const { lockMode } = extraProps || {};
    props.disabledLock = !(lockMode?.lockModeButtonEnabled ?? true);
    props.lockUsername = lockMode?.username;
    props.isLocked = lockMode?.isLocked;
  } else {
    if (typeof value === 'number') {
      props.value = value.toFixed(2);
    } else {
      props.value = String(value);
    }
  }

  if (component === 'TrueFalseButtonSet' || component === 'TrueFalseMaintModeButtonSet') {
    props.isMaintMode = component === 'TrueFalseMaintModeButtonSet';
  }

  if (component === 'MuiButton') {
    props.size = 'large';
    props.color = props.label.toLowerCase() === 'clear faults' ? 'error' : 'primary';
  }

  if (
    component === 'MuiButton' ||
    component === 'DataPoint' ||
    component === 'TextField' ||
    component === 'NumericInput'
  ) {
    props.fullWidth = true;
  }

  if (component === 'DataPoint') {
    props.variant = props.variant === 'dynamic' ? undefined : props.variant;
    props.allowWrap = true;
  }

  if (component === 'Select') {
    props.value = String(value).charAt(0);
    props.menuItems = props.extraProps.options;
    props.fullWidth = true;
  }

  if (batchControl) {
    props.disabled = currentUser?.role === Roles.Observer || controlRecipients?.length === 0;
  }

  return props;
};

export const generateReactComponentFunction: (
  componentMetadata: SingleReactComponentMetadata,
  displayGroupID: string,
  componentID: string,
  uri?: string,
  maintenanceAction?: boolean,
  batchControl?: boolean,
  alarmIDs?: string[],
  faultIDs?: string[],
) => ConfigurableComponentFunction = (
  componentMetadata,
  displayGroupID,
  componentID,
  uri,
  maintenanceAction,
  batchControl,
  alarmIDs,
  faultIDs,
) => {
  const { component, props } = componentMetadata;
  const Component = storybookComponents[component];

  // we can pass scalar as a prop for relevant controls
  let scalar: number | null = null;
  if (props && 'scalar' in props) {
    scalar = props.scalar;
    delete props.scalar;
  }

  let memoized: MemoizedComponentObject | undefined = undefined;

  if (component === 'MaintModeSlider') {
    props.product = props.extraProps?.product ?? '';
  }

  if (component === 'MaintActionControl') {
    props.controlURI = uri;
    props.options = props.extraProps.options ?? [];
  }

  return (
    assetState: ConfigurablePageStateStructure,
    controlRecipients?: string[],
  ): JSX.Element => {
    const stateInfo = getSpecificStateInfo(
      assetState,
      displayGroupID,
      componentID,
      !!uri,
      maintenanceAction,
      alarmIDs,
      faultIDs,
    );

    if (memoized !== undefined && memoIsValid(memoized, stateInfo, controlRecipients)) {
      return memoized.element;
    }

    organizeProps(props, component, stateInfo, batchControl, controlRecipients);

    const controlHandlerObject = uri
      ? generateControlHandlerObject(
          uri,
          component,
          scalar,
          batchControl,
          controlRecipients,
          componentID,
        )
      : {};

    const element = (
      <Component key={`${displayGroupID}/${componentID}`} {...props} {...controlHandlerObject} />
    );

    memoized = {
      prevState: stateInfo,
      prevControlRecipients: controlRecipients,
      element,
    };

    return element;
  };
};
