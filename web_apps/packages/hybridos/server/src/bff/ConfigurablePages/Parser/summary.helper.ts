import { ControlType, DisplayGroupDTO, ValueType } from 'shared/types/dtos/configurablePages.dto';
import { computeNakedValue } from 'src/utils/utils';
import { SiteConfiguration } from 'src/webuiconfig/webUIConfig.interface';
import {
  controlComponentStateInfo,
  controlObjectForNakedBody,
  ControlTypes,
  individualClothedBody,
  metadataFromDBI,
  nakedBodyFromFims,
  statusComponentStateInfo,
} from '../configurablePages.types';

export const parseSummaryData = (
  dbiMetadata: metadataFromDBI,
  rawData: nakedBodyFromFims,
  includeStatic: boolean,
  enableAssetPageControls: boolean,
  siteConfiguration: SiteConfiguration,
): DisplayGroupDTO => {
  const displayGroupDTO: DisplayGroupDTO = {
    status: {},
    control: {},
  };

  dbiMetadata.summary.forEach((status) => {
    const componentID = status.uri.slice(1);
    if (rawData[componentID] === undefined) {
      return;
    }

    const rawValue = rawData[componentID];

    const computeValue: ValueType =
      rawValue !== null && typeof rawValue === 'object' ? rawValue.value : rawValue;

    const scalar =
      status.scalar && (typeof status.scalar === 'number' || !isNaN(Number(status.scalar)))
        ? Number(status.scalar)
        : 1;

    const precision =
      status.precision &&
      (typeof status.precision === 'number' || !Number.isNaN(Number(status.precision)))
        ? Number(status.precision)
        : 2;

    const { value: actualValue } =
      typeof computeValue === 'number'
        ? computeNakedValue(computeValue, scalar, status.units ?? '', precision)
        : { value: computeValue };

    displayGroupDTO.status[componentID] = {
      state: { value: actualValue },
    };

    if (!includeStatic) return;

    displayGroupDTO.status[componentID].static = {
      label: status.name,
      unit: status.units,
    };
  });

  dbiMetadata.summaryControls.forEach((control) => {
    const componentID = control.uri.slice(1);
    if (!rawData[componentID]) {
      return;
    }

    const rawValue = rawData[componentID] as ValueType;
    displayGroupDTO.control[componentID] = {
      state: {
        enabled: setControlEnabled(
          enableAssetPageControls,
          (rawData[componentID] as controlObjectForNakedBody).enabled,
        ),
      },
    };

    if (rawData[componentID].hasOwnProperty('value')) {
      const rawValue = rawData[componentID];

      const computeValue: ValueType =
        rawValue !== null && typeof rawValue === 'object' ? rawValue.value : rawValue;

      const scalar =
        control.scalar && (typeof control.scalar === 'number' || !isNaN(Number(control.scalar)))
          ? Number(control.scalar)
          : 1;

      const precision =
        control.precision &&
        (typeof control.precision === 'number' || !Number.isNaN(Number(control.precision)))
          ? Number(control.precision)
          : 2;

      const { value: trueValue } =
        typeof computeValue === 'number'
          ? computeNakedValue(computeValue, scalar, control.units ?? '', precision)
          : { value: computeValue };
      displayGroupDTO.control[componentID].state.value = trueValue;
    }

    if (!includeStatic) {
      return;
    }

    displayGroupDTO.control[componentID].static = {
      label: control.name,
      unit: control.units,
      scalar: Number(control.scalar),
      controlType:
        control.name === ControlTypes.MaintenaceMode
          ? 'maint_mode_slider'
          : (control.inputType as ControlType),
    };

    if (control.inputType === 'enum') {
      const optionsArray = (rawData[componentID] as controlObjectForNakedBody).options.map(
        (option) => {
          return {
            name: option.name,
            value: option.return_value.toString(),
          };
        },
      );

      displayGroupDTO.control[componentID].static = {
        ...displayGroupDTO.control[componentID].static,
        extraProps: { options: optionsArray },
      };
    }

    if (control.name === ControlTypes.MaintenaceMode) {
      displayGroupDTO.control[componentID].static.controlType = 'maint_mode_slider';

      displayGroupDTO.control[componentID].static.extraProps = {
        ...displayGroupDTO.control[componentID].static.extraProps,
        product: siteConfiguration.product,
      };
    }
  });

  dbiMetadata.info.alarmFields.forEach((alarm) => {
    if (!rawData[alarm] || rawData[alarm] === '0' || !isIndividualClothedBody(rawData[alarm]))
      return;

    const alarmInfo = rawData[alarm] as individualClothedBody;
    if ('options' in alarmInfo) {
      if (!displayGroupDTO.alarm) {
        displayGroupDTO.alarm = [];
      }
      alarmInfo.options.forEach((option) => {
        displayGroupDTO.alarm.push(option.name);
      });
    } else if ('value' in alarmInfo && Number(alarmInfo.value) > 0) {
      displayGroupDTO.alarm.push(alarmInfo.name);
    }
  });

  dbiMetadata.info.faultFields.forEach((fault) => {
    if (!rawData[fault] || rawData[fault] === '0' || !isIndividualClothedBody(rawData[fault]))
      return;

    const faultInfo = rawData[fault] as individualClothedBody;
    if ('options' in faultInfo) {
      if (!displayGroupDTO.fault) {
        displayGroupDTO.fault = [];
      }
      faultInfo.options.forEach((option) => {
        displayGroupDTO.fault.push(option.name);
      });
    } else if ('value' in faultInfo && Number(faultInfo.value) > 0) {
      displayGroupDTO.alarm.push(faultInfo.name);
    }
  });

  return displayGroupDTO;
};

const setControlEnabled = (enableAssetPageControls: boolean, currentValue: boolean) => {
  const enableControl = enableAssetPageControls ? currentValue : false;
  return enableControl;
};

const isIndividualClothedBody = (
  data: statusComponentStateInfo | controlComponentStateInfo | individualClothedBody,
): data is individualClothedBody => {
  return data.hasOwnProperty('options');
};
