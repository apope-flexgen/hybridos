export type ConfigurablePageDTO = {
    hasStatic: boolean
    displayGroups: {
        [displayGroupID: string]: DisplayGroupDTO
    }
}

export type DisplayGroupDTO = {
    displayName?: string
    status?: {
        [componentID: string]: StatusComponentDTO
    }
    control?: {
        [componentID: string]: ControlComponentDTO
    }
    fault?: string[]
    alarm?: string[]
}

export type StatusComponentDTO = {
    static?: {
        label?: string
        unit?: string
    }
    state?: {
        value?: ValueType
    }
}

export type ControlComponentDTO = {
    static?: {
        label: string
        unit?: string
        scalar?: number
        controlType: ControlType
    }
    state?: {
        value?: ValueType
        enabled?: boolean
    }
}

export type ControlType =
    | 'enum_button'
    | 'button'
    | 'number'
    | 'enum_slider'
    | 'switch'
    | 'maint_mode_slider'
export type ValueType = string | number | boolean
