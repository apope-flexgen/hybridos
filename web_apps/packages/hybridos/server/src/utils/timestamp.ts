import dayjs from 'dayjs'

export const getformattedTimestamp = (time: string, format: string = 'YYYY-MM-DD HH:mm:ss') => {
    return dayjs(time, format)
}
