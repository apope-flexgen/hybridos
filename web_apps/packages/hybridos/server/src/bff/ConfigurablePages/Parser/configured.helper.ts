import { computeNakedValue } from 'src/utils/utils';
import {
  ControlComponentDTO,
  ControlType,
  DisplayGroupDTO,
  MaintenanceActionComponentDTO,
  StatusComponentDTO,
  ValueType,
} from 'shared/types/dtos/configurablePages.dto';

import {
  nakedBodyFromFims,
  metadataFromDBI,
  individualClothedBody,
  statusDataFromNakedBody,
  controlDataFromNakedBody,
  controlObjectForNakedBody,
  statusComponentStateInfo,
  controlComponentStateInfo,
  ControlTypes,
} from '../configurablePages.types';
import { SiteConfiguration } from 'src/webuiconfig/webUIConfig.interface';

// TODO: finalize this method
export const parseConfiguredData = (
  rawData: nakedBodyFromFims,
  setLockMode: (state: boolean) => void,
  metaData: metadataFromDBI,
  includeStatic: boolean,
  enableAssetPageControls: boolean,
  siteConfiguration: SiteConfiguration,
  maintenanceActionsStaticData?: any,
  hasBatchControls?: boolean,
): DisplayGroupDTO => {
  const displayGroupDTO: DisplayGroupDTO = {
    status: parseNakedBodyStatus(rawData, metaData, includeStatic),
    control: parseNakedBodyControl(
      rawData,
      setLockMode,
      metaData,
      includeStatic,
      enableAssetPageControls,
      siteConfiguration,
      false
    ),
    batchControl: hasBatchControls ? parseNakedBodyControl(
      rawData,
      setLockMode,
      metaData,
      includeStatic,
      enableAssetPageControls,
      siteConfiguration,
      true,
    ): {},
    fault: parseNakedBodyFault(rawData, metaData),
    alarm: parseNakedBodyAlarm(rawData, metaData),
    maintenanceActions: parseMaintenanceActionData(includeStatic, maintenanceActionsStaticData),
  };

  return displayGroupDTO;
};

const parseMaintenanceActionData = (
  includeStatic: boolean,
  maintenanceActionsStaticData: any,
) => {
  let maintenanceActionsMetadata = {}
  if (maintenanceActionsStaticData) {
      const dataType = includeStatic ? "static" : "state";
      Object.keys(maintenanceActionsStaticData).forEach((maintenanceAction) => {
        maintenanceActionsMetadata = { ...maintenanceActionsMetadata, [maintenanceAction]: { [dataType]: maintenanceActionsStaticData[maintenanceAction] } }
      });
  } 
  return maintenanceActionsMetadata;
}

const parseNakedBodyStatus = (
  rawData: nakedBodyFromFims,
  metaData: metadataFromDBI,
  includeStatic: boolean,
): { [componentID: string]: StatusComponentDTO } => {
  const aggregatedDTOs: { [componentID: string]: StatusComponentDTO } = {};

  metaData.statuses.forEach((status: statusDataFromNakedBody) => {
    const componentID = status.uri.slice(1);

    if (rawData[componentID] === undefined || rawData[componentID] === null) {
      return;
    }

    const rawValue = rawData[componentID]; // actual value from naked body data

    const computeValue: ValueType =
      rawValue !== null && typeof rawValue === 'object' ? rawValue.value : rawValue;

    const scalar =
      status.scalar && (typeof status.scalar === 'number' || !isNaN(Number(status.scalar)))
        ? Number(status.scalar)
        : 1;

    const { value: actualValue } =
      typeof computeValue === 'number'
        ? computeNakedValue(computeValue, scalar, status.units ?? '')
        : { value: computeValue };

    aggregatedDTOs[componentID] = {
      state: {
        value: actualValue,
      },
    };

    if (!includeStatic) {
      return;
    }

    aggregatedDTOs[componentID].static = {
      label: status.name,
      unit: status.units,
    };
  });

  return aggregatedDTOs;
};

const parseNakedBodyControl = (
  rawData: nakedBodyFromFims,
  setLockMode: (state: boolean) => void,
  metaData: metadataFromDBI,
  includeStatic: boolean,
  enableAssetPageControls: boolean,
  siteConfiguration: SiteConfiguration,
  isBatchControl: boolean,
): { [componentID: string]: ControlComponentDTO } => {
  const aggregatedDTOs: { [componentID: string]: ControlComponentDTO } = {};
  const controlsFromMetadata = isBatchControl && metaData.batchControls?.length > 0 ? metaData.batchControls :  metaData.controls;

  const lockModeStatus: controlObjectForNakedBody = rawData[
    'lock_mode'
  ] as controlObjectForNakedBody;
  setLockMode(!!lockModeStatus?.value);

  controlsFromMetadata.forEach((control: controlDataFromNakedBody) => {
    const componentID = control.uri.slice(1);

    if (rawData[componentID] === undefined || rawData[componentID] === null) {
      return;
    }

    aggregatedDTOs[componentID] = {
      state: {
        enabled: setControlEnabled(
          enableAssetPageControls,
          (rawData[componentID] as controlObjectForNakedBody).enabled === undefined ? true : (rawData[componentID] as controlObjectForNakedBody).enabled,
        ),
      },
    };

    let trueScalar = 1
    const rawValue = rawData[componentID];

    const computeValue: ValueType =
      (rawValue !== null && typeof rawValue === 'object' && rawData[componentID].hasOwnProperty('value')) ? rawValue.value : rawValue;

    const optionsArray: any[] = (rawValue !== null && typeof rawValue === 'object' && rawData[componentID].hasOwnProperty('options'))
      ? rawValue.options.map((option) => {
        return {
          name: option.name,
          value: option.return_value,
        };
      })
      : [];

    if (optionsArray.length > 0) {
      aggregatedDTOs[componentID].state.extraProps = {...aggregatedDTOs[componentID].state.extraProps, options: optionsArray};
    }

    const scalar =
      control.scalar && (typeof control.scalar === 'number' || !isNaN(Number(control.scalar)))
        ? Number(control.scalar)
        : 1;

    if (includeStatic) {
      // first run
      const { scalar: newScalar } =
        control.inputType === 'number'
          ? computeNakedValue(undefined, scalar, control.units ?? '')
          : { scalar: trueScalar };
      trueScalar = newScalar;
      aggregatedDTOs[componentID].state.value = computeValue;
    } else {
      const { value: trueValue, scalar: newScalar } =
        typeof computeValue === 'number'
          ? computeNakedValue(computeValue, scalar, control.units ?? '')
          : { value: computeValue, scalar: trueScalar };

      aggregatedDTOs[componentID].state.value = trueValue;
      trueScalar = newScalar;
    }

    if (!includeStatic) {
      return;
    }

    aggregatedDTOs[componentID].static = {
      label: control.name,
      unit: control.units ?? '',
      scalar: trueScalar,
      controlType: control.inputType as ControlType,
    };

    if (control.inputType === 'enum' || control.inputType === 'maint_action_control') {
      const optionsArray = (rawData[componentID] as controlObjectForNakedBody).options.map(
        (option) => {
          return {
            name: option.name,
            value: option.return_value.toString(),
          };
        },
      );

      aggregatedDTOs[componentID].static = {
        ...aggregatedDTOs[componentID].static,
        extraProps: { options: optionsArray },
      };
    }

    if (control.name === ControlTypes.MaintenaceMode) {
      aggregatedDTOs[componentID].static.controlType = 'maint_mode_slider';

      aggregatedDTOs[componentID].static.extraProps = {
        ...aggregatedDTOs[componentID].static.extraProps,
        product: siteConfiguration.product,
      };
    }
  });

  return aggregatedDTOs;
};

const parseNakedBodyAlarm = (rawData: nakedBodyFromFims, metaData: metadataFromDBI): string[] => {
  const stateData: string[] = [];

  const fieldsToCheck = metaData.info['alarmFields'];

  fieldsToCheck.forEach((field: string) => {
    if (rawData[field] === undefined || rawData[field] === '0')
      return;

    if (isIndividualClothedBody(rawData[field])) {
      const alarmInfo = rawData[field] as individualClothedBody;
      if ('options' in alarmInfo) {
        alarmInfo.options.forEach((option) => {
          stateData.push(option.name);
        });
      } else if ('value' in alarmInfo && Number(alarmInfo.value) > 0) {
        stateData.push(alarmInfo.name);
      }
    } else if (typeof rawData[field] === 'boolean') {
      const alarmPresent = rawData[field] as boolean;
      if (alarmPresent) {
        stateData.push('Alarm present');
      }
    }
  });

  return stateData;
};

const parseNakedBodyFault = (rawData: nakedBodyFromFims, metaData: metadataFromDBI): string[] => {
  const stateData: string[] = [];

  const fieldsToCheck = metaData.info['faultFields'];

  fieldsToCheck.forEach((field: string) => {
    if (rawData[field] === undefined || rawData[field] === '0')
      return;

    if (isIndividualClothedBody(rawData[field])) {
      const faultInfo = rawData[field] as individualClothedBody;
      if ('options' in faultInfo) {
        faultInfo.options.forEach((option) => {
          stateData.push(option.name);
        });
      } else if ('value' in faultInfo && Number(faultInfo.value) > 0) {
        stateData.push(faultInfo.name);
      }
    } else if (typeof rawData[field] === 'boolean') {
      const faultPresent = rawData[field] as boolean;
      if (faultPresent) {
        stateData.push('Fault present');
      }
    }
  });

  return stateData;
};

const isIndividualClothedBody = (
  data: statusComponentStateInfo | controlComponentStateInfo | individualClothedBody,
): data is individualClothedBody => {
  return data.hasOwnProperty('options');
};

const setControlEnabled = (enableAssetPageControls: boolean, currentValue: boolean) => {
  const enableControl = enableAssetPageControls ? currentValue : false;
  return enableControl;
};
