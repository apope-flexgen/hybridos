export type SiteStatusConfig = {
    siteStatusLabel?: string,
    dataSources: SiteStatusDataField[]
}

export type SiteStatusDataFieldConfig = {
    uri: string
    field: string
    dataType: string
    label: string
}

export type SiteStatusDataField = SiteStatusDataFieldConfig & {
    index: number
}

export type SiteStatusDataPointInfo = {
    label: string
    value: string
    unit: string
    index: number
}

export type SiteStatusDataPointsDTO = {
    [uri: string]: SiteStatusDataPointInfo
}

export type SiteStatusResponse = {
    data: {
        siteStatusLabel?: string
        activeFaults?: number
        activeAlarms?: number
        siteState?: string
        dataPoints?: SiteStatusDataPointsDTO
    }
}
