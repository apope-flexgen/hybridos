import { computeClothedValue } from '../../../utils/utils'
import { DisplayGroupDTO } from 'shared/types/dtos/configurablePages.dto'
import { clothedBodyFromFims, ControlTypes } from '../configurablePages.types'

export const parseClothedData = (
    rawData: clothedBodyFromFims,
    includeStatic: boolean,
    enableAssetPageControls: boolean,
): DisplayGroupDTO => {
    const displayGroupDTO: DisplayGroupDTO = {
        status: {},
        control: {},
        fault: [],
        alarm: [],
    }

    Object.keys(rawData).forEach((componentID) => {
        // not interested in any fields that are not for the UI - all UI
        // fields are objects with a field ui_type != 'none'
        if (
            typeof rawData[componentID] !== 'object' ||
            !rawData[componentID].hasOwnProperty('ui_type') ||
            rawData[componentID].ui_type === 'none'
        ) {
            return
        }

        if (typeof rawData[componentID].value === 'number') {
            const { value: trueValue, targetUnit } = computeClothedValue(
                Number(rawData[componentID].value),
                Number(rawData[componentID].scaler),
                rawData[componentID].unit
            )
            rawData[componentID].value = trueValue
            rawData[componentID].unit = targetUnit
        }

        switch (rawData[componentID].ui_type) {
            case 'status':
                displayGroupDTO.status[componentID] = {
                    state: { value: rawData[componentID].value },
                }
                break
            case 'control':
                displayGroupDTO.control[componentID] = {
                    state: {
                        value: rawData[componentID].value,
                        enabled: setControlEnabled(
                            enableAssetPageControls,
                            rawData[componentID].enabled
                        ),
                    },
                }
                break
            case 'fault':
                if ('options' in rawData[componentID]) {
                    rawData[componentID].options.forEach((option) => {
                        displayGroupDTO.fault.push(option.name)
                    })
                }
                break
            case 'alarm':
                if ('options' in rawData[componentID]) {
                    rawData[componentID].options.forEach((option) => {
                        displayGroupDTO.alarm.push(option.name)
                    })
                }
                break
            default:
                break
        }

        // only interested in static data if includeStatic is true
        if (!includeStatic) {
            return displayGroupDTO
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
                }
                break
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
                }
                if (rawData[componentID].name === ControlTypes.MaintenaceMode) {
                    displayGroupDTO.control[componentID].static.controlType = 'maint_mode_slider'
                }
                break
            default:
                break
        }
    })

    return displayGroupDTO
}

const setControlEnabled = (enableAssetPageControls: boolean, currentValue: boolean) => {
    const enableControl = enableAssetPageControls ? currentValue : false
    return enableControl
}
