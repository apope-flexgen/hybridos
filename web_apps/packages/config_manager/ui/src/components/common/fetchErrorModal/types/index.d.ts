export interface RootStates {
    state: {
        queries: object
        status: string
    }
}

export interface QueryData {
    endpointName?: string
    error?: ErrorStatus
}

export interface ErrorStatus {
    status?: number
    data?: {
        errorMessage?: string
    }
}
