/* ----- Clothed Body Types ----- */

export interface summaryDataFromFims {
    [key: string]: clothedBodyFromFims
}

export type clothedBodyFromFims = {
    name: string
    [key: string]: individualClothedBody | string | any
}

export type individualClothedBody = {
    name: string
    unit: string
    scaler: number
    type: string
    ui_type: string
    enabled: boolean
    value: any // should this be generic instead of any?
    options?: [{ name: string; return_value: any }]
}

/* ----- Naked Body Types ----- */

export interface metadataFromDBI {
    alarms: {
        alarmFields: any[]
        faultFields: any[]
    }
    allControls: controlDataFromNakedBody[]
    controls: controlDataFromNakedBody[]
    info: {
        [key: string]: any
        alarmFields: string[]
        faultFields: string[]
    }
    statuses: statusDataFromNakedBody[]
    summary: any[]
    summaryControls: any[]
}

export interface nakedBodyFromFims {
    [key: string]: statusComponentStateInfo | controlObjectForNakedBody | individualClothedBody
}

export interface statusDataFromNakedBody {
    name: string
    scalar: string
    units: string
    uri: string
}

export interface controlDataFromNakedBody {
    inputType: string
    name: string
    scalar: string
    units: string
    uri: string
}

export interface controlObjectForNakedBody {
    value?: boolean | number
    enabled: boolean
    options: {
        name: string
        return_value: any
    }[]
}

export type statusComponentStateInfo = string | number | boolean

export interface controlComponentStateInfo {
    value?: string | number | boolean
    enabled: boolean
}

export enum ControlTypes {
    MaintenaceMode = 'Maintenance Mode',
}
