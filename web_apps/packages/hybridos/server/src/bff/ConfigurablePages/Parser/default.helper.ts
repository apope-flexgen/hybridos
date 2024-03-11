import { computeClothedValue } from '../../../utils/utils';
import { DisplayGroupDTO } from 'shared/types/dtos/configurablePages.dto';
import { SiteConfiguration } from 'src/webuiconfig/webUIConfig.interface';
import {
  clothedBodyFromFims,
  ControlTypes,
  individualClothedBody,
} from '../configurablePages.types';

const skippedComponents: string[] = ['lock_mode'];

export const parseDefaultData = (
  rawData: clothedBodyFromFims,
  setLockMode: (state: boolean) => void,
  includeStatic: boolean,
  enableAssetPageControls: boolean,
  siteConfiguration: SiteConfiguration,
): DisplayGroupDTO => {
  const displayGroupDTO: DisplayGroupDTO = {
    status: {},
    control: {},
    fault: [],
    alarm: [],
  };

  const lockModeStatus: individualClothedBody = rawData['lock_mode'] as individualClothedBody;
  if (lockModeStatus?.value !== undefined) {
    setLockMode(!!lockModeStatus?.value);
  }

  Object.keys(rawData).forEach((componentID) => {
    if (skippedComponents.includes(componentID)) return;

    // not interested in any fields that are not for the UI - all UI
    // fields are objects with a field ui_type != 'none'
    if (
      typeof rawData[componentID] !== 'object' ||
      !rawData[componentID].hasOwnProperty('ui_type') ||
      rawData[componentID].ui_type === 'none'
    ) {
      return;
    }

    if (typeof rawData[componentID].value === 'number') {
      const {
        value: trueValue,
        targetUnit,
        scalar: newScalar,
      } = computeClothedValue(
        Number(rawData[componentID].value),
        Number(rawData[componentID].scaler),
        rawData[componentID].unit,
        siteConfiguration,
      );
      rawData[componentID].value = trueValue;
      rawData[componentID].unit = targetUnit;
      rawData[componentID].scaler = newScalar;
    }

    switch (rawData[componentID].ui_type) {
      case 'status':
        displayGroupDTO.status[componentID] = {
          state: { value: rawData[componentID].value },
        };
        break;
      case 'control':
        displayGroupDTO.control[componentID] = {
          state: {
            value: rawData[componentID].value,
            enabled: setControlEnabled(enableAssetPageControls, rawData[componentID].enabled),
          },
        };
        break;
      case 'fault':
        if ('options' in rawData[componentID]) {
          rawData[componentID].options.forEach((option) => {
            displayGroupDTO.fault.push(option.name);
          });
        }
        // display site faults as generic singular fault (if value is > 0, display fault, otherwise do not)
        else if ('value' in rawData[componentID] && Number(rawData[componentID].value) > 0) {
          displayGroupDTO.fault = [rawData[componentID].name];
        }
        break;
      case 'alarm':
        if ('options' in rawData[componentID]) {
          rawData[componentID].options.forEach((option) => {
            displayGroupDTO.alarm.push(option.name);
          });
        }
        // display site alarms as generic singular alarm (if value is > 0, display alarm, otherwise do not)
        else if ('value' in rawData[componentID] && Number(rawData[componentID].value) > 0) {
          displayGroupDTO.alarm = [rawData[componentID].name];
        }
        break;
      default:
        break;
    }

    // only interested in static data if includeStatic is true
    if (!includeStatic) {
      return displayGroupDTO;
    }

    switch (rawData[componentID].ui_type) {
      case 'status':
        displayGroupDTO.status[componentID] = {
          static: {
            label: rawData[componentID].name,
            unit: rawData[componentID].unit,
          },
          state: {
            ...displayGroupDTO.status[componentID]?.state,
          },
        };
        break;
      case 'control':
        displayGroupDTO.control[componentID] = {
          static: {
            label: rawData[componentID].name,
            unit: rawData[componentID].unit,
            scalar: rawData[componentID].scaler,
            controlType: rawData[componentID].type,
          },
          state: {
            ...displayGroupDTO.control[componentID]?.state,
          },
        };
        if (rawData[componentID].type === 'enum') {
          const optionsArray = rawData[componentID].options.map((option) => {
            return {
              name: option.name,
              value: option.return_value,
            };
          });
          displayGroupDTO.control[componentID].static = {
            ...displayGroupDTO.control[componentID].static,
            extraProps: { options: optionsArray },
          };
        }
        if (rawData[componentID].name === ControlTypes.MaintenaceMode) {
          displayGroupDTO.control[componentID].static.controlType = 'maint_mode_slider';

          displayGroupDTO.control[componentID].static = {
            ...displayGroupDTO.control[componentID].static,
            extraProps: {
              ...displayGroupDTO.control[componentID].static.extraProps,
              product: siteConfiguration.product,
            },
          };
        }
        break;
      default:
        break;
    }
  });

  return displayGroupDTO;
};

const setControlEnabled = (enableAssetPageControls: boolean, currentValue: boolean) => {
  const enableControl = enableAssetPageControls ? currentValue : false;
  return enableControl;
};
