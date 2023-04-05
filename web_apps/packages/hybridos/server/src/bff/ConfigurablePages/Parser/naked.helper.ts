import { computeNakedValue } from 'src/utils/utils'
import {
    ControlComponentDTO,
    ControlType,
    DisplayGroupDTO,
    StatusComponentDTO,
    ValueType,
} from 'shared/types/dtos/configurablePages.dto'

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
} from '../configurablePages.types'

// TODO: finalize this method
export const parseNakedData = (
    rawData: nakedBodyFromFims,
    metaData: metadataFromDBI,
    includeStatic: boolean,
    enableAssetPageControls: boolean,
): DisplayGroupDTO => {
    const displayGroupDTO: DisplayGroupDTO = {
        status: parseNakedBodyStatus(rawData, metaData, includeStatic),
        control: parseNakedBodyControl(rawData, metaData, includeStatic, enableAssetPageControls),
        fault: parseNakedBodyFault(rawData, metaData),
        alarm: parseNakedBodyAlarm(rawData, metaData),
    }

    return displayGroupDTO
}

const parseNakedBodyStatus = (
    rawData: nakedBodyFromFims,
    metaData: metadataFromDBI,
    includeStatic: boolean
): { [componentID: string]: StatusComponentDTO } => {
    const aggregatedDTOs: { [componentID: string]: StatusComponentDTO } = {}

    metaData.statuses.forEach((status: statusDataFromNakedBody) => {
        const componentID = status.uri.slice(1)

        if (!rawData[componentID]) {
            return
        }

        const rawValue = rawData[componentID] as ValueType // actual value from naked body data

        const actualValue =
            typeof rawValue === 'number'
                ? computeNakedValue(rawValue, Number(status.scalar))
                : rawValue

        aggregatedDTOs[componentID] = {
            state: {
                value: actualValue,
            },
        }

        if (!includeStatic) {
            return
        }

        aggregatedDTOs[componentID].static = {
            label: status.name,
            unit: status.units,
        }
    })

    return aggregatedDTOs
}

const parseNakedBodyControl = (
    rawData: nakedBodyFromFims,
    metaData: metadataFromDBI,
    includeStatic: boolean,
    enableAssetPageControls: boolean,
): { [componentID: string]: ControlComponentDTO } => {
    const aggregatedDTOs: { [componentID: string]: ControlComponentDTO } = {}

    metaData.controls.forEach((control: controlDataFromNakedBody) => {
        const componentID = control.uri.slice(1)

        if (!rawData[componentID]) {
            return
        }

        aggregatedDTOs[componentID] = {
            state: {
                enabled: setControlEnabled(
                    enableAssetPageControls,
                    (rawData[componentID] as controlObjectForNakedBody).enabled
                ),
            },
        }

        if (rawData[componentID].hasOwnProperty('value')) {
            const rawValue = (rawData[componentID] as controlObjectForNakedBody).value
            const trueValue =
                typeof rawValue === 'number'
                    ? computeNakedValue(rawValue, Number(control.scalar))
                    : rawValue
            aggregatedDTOs[componentID].state.value = trueValue
        }

        if (!includeStatic) {
            return
        }

        aggregatedDTOs[componentID].static = {
            label: control.name,
            unit: control.units,
            scalar: Number(control.scalar),
            controlType:
                control.name === ControlTypes.MaintenaceMode
                    ? 'maint_mode_slider'
                    : (control.inputType as ControlType),
        }
    })

    return aggregatedDTOs
}

const parseNakedBodyAlarm = (rawData: nakedBodyFromFims, metaData: metadataFromDBI): string[] => {
    const stateData: string[] = []

    const fieldsToCheck = metaData.info['alarmFields']

    fieldsToCheck.forEach((field: string) => {
        if (!rawData[field] || rawData[field] === '0' || !isIndividualClothedBody(rawData[field]))
            return

        const alarmInfo = rawData[field] as individualClothedBody
        alarmInfo.options.forEach((option) => {
            stateData.push(option.name)
        })
    })

    return stateData
}

const parseNakedBodyFault = (rawData: nakedBodyFromFims, metaData: metadataFromDBI): string[] => {
    const stateData: string[] = []

    const fieldsToCheck = metaData.info['faultFields']

    fieldsToCheck.forEach((field: string) => {
        if (!rawData[field] || rawData[field] === '0' || !isIndividualClothedBody(rawData[field]))
            return

        const faultInfo = rawData[field] as individualClothedBody
        faultInfo.options.forEach((option) => {
            stateData.push(option.name)
        })
    })

    return stateData
}

const isIndividualClothedBody = (
    data: statusComponentStateInfo | controlComponentStateInfo | individualClothedBody
): data is individualClothedBody => {
    return data.hasOwnProperty('options')
}

const setControlEnabled = (enableAssetPageControls: boolean, currentValue: boolean) => {
    const enableControl = enableAssetPageControls ? currentValue : false
    return enableControl
}
