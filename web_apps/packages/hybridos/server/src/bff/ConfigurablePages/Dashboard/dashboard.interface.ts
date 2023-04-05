export type summaryInfoFromStream = {
    [key: string]: number | string | boolean
    name?: string
}

export type ConfigBody = {
    _doc: string
    _id: string
    _version: string
    data: SingleCardData[]
}

export type SingleCardInfo = {
    baseURI: string
    batteryView: boolean
    batteryViewSourceURI: string | null
    batteryViewURI: string
    isTemplate: boolean
    items: { name: string; uri: string }[]
    name: string
    sourceURIs: string[]
}

export type SingleStatus = {
    name: string
    scalar: string | null
    sourceURI: string
    units: string
    uri: string
}

export type SingleCardData = {
    info: SingleCardInfo
    status: SingleStatus[]
}
