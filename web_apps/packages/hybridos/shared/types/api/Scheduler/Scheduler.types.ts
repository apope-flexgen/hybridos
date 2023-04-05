export interface schedulerEventDataType {
    id: string
    duration: number
    mode: string
    start_time: string
    variables: {
        [key: string]: number
    }
    repeat?: any
}
