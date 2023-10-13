/* eslint-disable */
// TODO: fix lint
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
} from './componentFactory.types';

// FIXME: debounce button inputs (should this be an option in our button component?)
const generateControlHandlerObject = (
  uri: string,
  componentName: string,
  scalar: number | null,
): ControlHandlerObject => {
  if (controlsSets.onClick.includes(componentName)) {
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
    return {
      onCheck: (value: PossibleConfirmValues) => {
        const realTimeService = RealTimeService.Instance;
        let trueValue = value;
        if (typeof value !== 'boolean') {
          const maybeNumber = Number(value);
          if (!Number.isNaN(maybeNumber) && scalar !== null) {
            trueValue = maybeNumber * scalar;
          }
        }
        realTimeService.send('fimsNoReply', {
          method: 'set',
          uri,
          replyto: 'web_ui',
          body: trueValue,
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
): any => {
  if (control) {
    return assetState[displayGroupID].control[componentID];
  }

  return assetState[displayGroupID].status[componentID];
};

const memoIsValid = (
  memoized: MemoizedComponentObject,
  stateInfo: StatusComponentStateInfo | ControlComponentStateInfo | undefined,
): boolean => {
  const { prevState } = memoized;
  if (stateInfo === undefined) {
    return true;
  }

  if (JSON.stringify(prevState) === JSON.stringify(stateInfo)) {
    return true;
  }

  return false;
};

// TODO: this could probably be done better
const organizeProps = (
  props: any,
  component: string,
  stateInfo: StatusComponentStateInfo | ControlComponentStateInfo | undefined,
): any => {
  if (stateInfo === undefined) {
    return props;
  }

  const [value, enabled] =
    typeof stateInfo === 'object' ? [stateInfo.value, stateInfo.enabled] : [stateInfo, true];

  props.disabled = !enabled;

  if (component === 'TextField') {
    props.disabled = false;
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
    props.endTextAdornment = props.unit === '' ? ' ' : props.unit;
    props.showStepper = false;
    props.size = 'small';
    props.validationRegEx = 'floats',
    props.bold = enabled;
    props.helperTextSize = 'small';
    props.color = enabled ? 'secondary' : 'primary';
    props.disabled = false;
  } else if (component === 'Switch') {
    props.labelPlacement = 'right'
    props.value = value;
    props.autoLayout = true;
  } else if (component === 'MaintModeSlider') {
    props.autoLayout = true;
    props.value = value;

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

  return props;
};

export const generateReactComponentFunction: (
  componentMetadata: SingleReactComponentMetadata,
  displayGroupID: string,
  componentID: string,
  uri?: string,
) => ConfigurableComponentFunction = (componentMetadata, displayGroupID, componentID, uri) => {
  const { component, props } = componentMetadata;
  const Component = storybookComponents[component];

  // we can pass scalar as a prop for relevant controls
  let scalar: number | null = null;
  if (props && 'scalar' in props) {
    scalar = props.scalar;
    delete props.scalar;
  }

  const controlHandlerObject = uri ? generateControlHandlerObject(uri, component, scalar) : {};

  let memoized: MemoizedComponentObject | undefined = undefined;

  if (component === 'MaintModeSlider') {
    props.product = props.extraProps?.product ?? '';
  }

  return (assetState: ConfigurablePageStateStructure): JSX.Element => {
    const stateInfo = getSpecificStateInfo(assetState, displayGroupID, componentID, !!uri);

    if (memoized !== undefined && memoIsValid(memoized, stateInfo)) {
      return memoized.element;
    }

    organizeProps(props, component, stateInfo);

    const element = (
      <Component key={`${displayGroupID}/${componentID}`} {...props} {...controlHandlerObject} />
    );

    memoized = { prevState: stateInfo, element };

    return element;
  };
};
